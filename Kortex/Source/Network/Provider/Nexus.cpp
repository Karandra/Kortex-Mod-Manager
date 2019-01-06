#include "stdafx.h"
#include "Nexus.h"
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Application.hpp>
#include <Kortex/Events.hpp>
#include "UI/KMainWindow.h"
#include "Utility/String.h"
#include <KxFramework/KxWebSockets.h>
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxJSON.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxIndexedEnum.h>

namespace
{
	using namespace Kortex;
	using namespace Kortex::NetworkManager;
	using TJsonValue = typename nlohmann::json::value_type;

	class CategoryDef: public KxIndexedEnum::Definition<CategoryDef, IModFileInfo::CategoryID, wxString>
	{
		inline static const TItem ms_Index[] = 
		{
			{IModFileInfo::CategoryID::Main, wxS("MAIN")},
			{IModFileInfo::CategoryID::Optional, wxS("OPTIONAL")},
		};
	};

	wxString& ConvertChangeLog(wxString& changeLog)
	{
		changeLog.Replace(wxS("<br>"), wxS("\r\n"));
		changeLog.Replace(wxS("<br/>"), wxS("\r\n"));
		changeLog.Replace(wxS("<br />"), wxS("\r\n"));
		changeLog.Replace(wxS("</br>"), wxS("\r\n"));

		changeLog.Replace(wxS("\n\r\n"), wxS("\r\n"));
		KxString::Trim(changeLog, true, true);

		return changeLog;
	}
	wxString& ConvertDisplayName(wxString& name)
	{
		name.Replace(wxS("_"), wxS(" "));
		KxString::Trim(name, true, true);

		return name;
	}
	wxString& ConvertUnicodeEscapes(wxString& source)
	{
		// Find and replace all '\uABCD' 6-char hex patterns to corresponding Unicode codes.
		// This is almost the same as 'ModImporterMO::DecodeUTF8'. Need to generalize and merge these functions.
		constexpr size_t prefixLength = 2;
		constexpr size_t sequenceLength = 6;
		constexpr size_t valueLength = sequenceLength - prefixLength;

		for (size_t i = 0; i < source.Length(); i++)
		{
			size_t pos = source.find(wxS("\\u"), i);
			if (pos != wxString::npos)
			{
				unsigned long long value = 0;
				if (source.Mid(pos + prefixLength, valueLength).ToULongLong(&value, 16) && value != 0)
				{
					wxUniChar c(value);
					source.replace(pos, sequenceLength, c);
				}
			}
		}
		return source;
	}

	wxDateTime ReadDateTime(const TJsonValue& json)
	{
		wxDateTime date;
		date.ParseISOCombined(json.get<wxString>());
		return date.FromUTC(date.IsDST());
	}
	void ReadFileInfo(const TJsonValue& json, ReplyStructs::ModFileInfo& info)
	{
		info.ID = json["file_id"].get<ModID::TValue>();
		info.IsPrimary = json["is_primary"];
		info.Name = json["file_name"].get<wxString>();
		info.m_DisplayName = ConvertDisplayName(json["name"].get<wxString>());
		info.Version = json["version"].get<wxString>();
		info.m_ChangeLog = ConvertChangeLog(json["changelog_html"].get<wxString>());
		info.UploadDate = ReadDateTime(json["uploaded_time"]);

		// WTF?! Why file size is in kilobytes instead of bytes?
		// Ok, I convert it here, though final size may be a bit smaller.
		// At least download manager can request correct file size upon downloading.
		info.Size = json["size"].get<int64_t>() * 1024;

		// Values: 'MAIN', 'OPTIONAL', <TBD>.
		info.Category = CategoryDef::FromString(json["category_name"].get<wxString>(), IModFileInfo::CategoryID::Unknown);
	}
	void ReadGameInfo(const TJsonValue& json, Nexus::Internal::ReplyStructs::GameInfo& info)
	{
		info.ID = json["id"];
		info.Name = json["name"].get<wxString>();
		info.Genre = json["genre"].get<wxString>();
		info.ForumURL = json["forum_url"].get<wxString>();
		info.NexusURL = json["nexusmods_url"].get<wxString>();
		info.DomainName = json["domain_name"].get<wxString>();

		info.FilesCount = json["file_count"];
		info.DownloadsCount = json["downloads"];
		info.ModsCount = json["mods"];
		info.m_ApprovedDate = wxDateTime((time_t)json["approved_date"]);
	}

	void ReportRequestQuoteReached(const NexusProvider& nexus)
	{
		INotificationCenter::GetInstance()->NotifyFromManager<INetworkManager>(KTrf("Network.RequestQuotaReched", nexus.GetName()), KxICON_WARNING);
	}
	void ReportRequestError(const NexusProvider& nexus, const wxString& message)
	{
		INotificationCenter::GetInstance()->NotifyFromManager<INetworkManager>(message, KxICON_ERROR);
	}
}

namespace Kortex::NetworkManager
{
	void NexusProvider::OnAuthSuccess(wxWindow* window)
	{
		IEvent::CallAfter([this, window]()
		{
			INetworkProvider::OnAuthSuccess(window);
		});
	}
	void NexusProvider::OnAuthFail(wxWindow* window)
	{
		IEvent::CallAfter([this, window]()
		{
			INetworkProvider::OnAuthFail(window);
		});
	}

	wxString NexusProvider::EndorsementStateToString(const ModEndorsement& state) const
	{
		if (state.IsEndorsed())
		{
			return "endorse";
		}
		if (state.IsAbstained())
		{
			return "abstain";
		}
		return "undecided";
	}
	KxCURLSession& NexusProvider::ConfigureRequest(KxCURLSession& request, const wxString& apiKey) const
	{
		request.AddHeader("APIKEY", apiKey.IsEmpty() ? GetAPIKey() : apiKey);
		request.AddHeader("Content-Type", "application/json");
		request.AddHeader("Protocol-Version", "0.15.5");
		request.AddHeader("Application-Version", IApplication::GetInstance()->GetVersion());

		return request;
	}
	bool NexusProvider::ShouldTryLater(const KxCURLReplyBase& reply) const
	{
		// Should I make an enum with these codes?
		return reply.GetResponseCode() == 429;
	}
	wxString NexusProvider::GetAPIURL() const
	{
		return "https://api.nexusmods.com/v1";
	}
	wxString NexusProvider::GetAPIKey(wxString* userName) const
	{
		KxSecretValue apiKey;
		wxString name;
		LoadAuthInfo(name, apiKey);

		if (userName)
		{
			*userName = name;
		}
		return apiKey.GetAsString();
	}
	void NexusProvider::RequestUserAvatar(Nexus::ValidationInfo& info)
	{
		if (!HasUserPicture())
		{
			SetUserPicture(DownloadSmallBitmap(info.GetProfilePicture()));
		}
	}

	bool NexusProvider::DoAuthenticate(wxWindow* window)
	{
		KxWebSocketClient* client = new KxWebSocketClient("wss://sso.nexusmods.com");

		client->Bind(KxEVT_WEBSOCKET_OPEN, [this, client, window](KxWebSocketEvent& event)
		{
			const wxString id = KVarExp("$(AppGUID)");
			KxJSONObject json = {{"id", id}, {"appid", "Kortex"}};
			client->Send(KxJSON::Save(json));
			KxShell::Execute(window, KxShell::GetDefaultViewer("html"), "open", KxString::Format("https://www.nexusmods.com/sso?id=%1", id));
		});
		client->Bind(KxEVT_WEBSOCKET_MESSAGE, [this, client, window](KxWebSocketEvent& event)
		{
			wxString apiKey = event.GetTextMessage();
			client->Close();

			auto info = GetValidationInfo(apiKey);
			if (info->IsOK() && info->GetAPIKey() == apiKey)
			{
				if (SaveAuthInfo(info->GetUserName(), apiKey))
				{
					RequestUserAvatar(*info);
					OnAuthSuccess(window);
					return;
				}
			}
			OnAuthFail(window);
		});
		client->Bind(KxEVT_WEBSOCKET_CLOSE, [this, client, window](KxWebSocketEvent& event)
		{
			if (!HasAuthInfo())
			{
				OnAuthFail(window);
			}

			INetworkManager::GetInstance()->OnAuthStateChanged();
			delete client;
		});
		client->Bind(KxEVT_WEBSOCKET_FAIL, [this, client, window](KxWebSocketEvent& event)
		{
			OnAuthFail(window);
			delete client;
		});
		return true;
	}
	bool NexusProvider::DoValidateAuth(wxWindow* window)
	{
		auto info = GetValidationInfo();
		if (info)
		{
			RequestUserAvatar(*info);
			return info->GetAPIKey() == GetAPIKey();
		}
		return false;
	}
	bool NexusProvider::DoSignOut(wxWindow* window)
	{
		return INetworkProvider::DoSignOut(window);
	}
	bool NexusProvider::DoIsAuthenticated() const
	{
		return INetworkProvider::DoIsAuthenticated();
	}

	NexusProvider::NexusProvider()
		:INetworkProvider(wxS("Nexus"))
	{
	}

	KImageEnum NexusProvider::GetIcon() const
	{
		return KIMG_SITE_NEXUS;
	}
	wxString NexusProvider::GetName() const
	{
		return "Nexus";
	}
	wxString NexusProvider::GetGameID(const GameID& id) const
	{
		// If invalid profile is passed, return ID for current profile.
		if (id.IsOK())
		{
			// TES
			if (id == GameIDs::Morrowind)
			{
				return "Morrowind";
			}
			if (id == GameIDs::Oblivion)
			{
				return "Oblivion";
			}
			if (id == GameIDs::Skyrim)
			{
				return "Skyrim";
			}
			if (id == GameIDs::SkyrimSE)
			{
				return "SkyrimSpecialEdition";
			}

			// Fallout
			if (id == GameIDs::Fallout3)
			{
				return "Fallout3";
			}
			if (id == GameIDs::FalloutNV)
			{
				return "NewVegas";
			}
			if (id == GameIDs::Fallout4)
			{
				return "Fallout4";
			}
		}
		return INetworkProvider::GetGameID(id);
	}
	wxString& NexusProvider::ConvertDescriptionToHTML(wxString& description) const
	{
		auto RAW = [](const auto& s)
		{
			return wxString::FromUTF8Unchecked(s);
		};

		// Trimming
		KxString::Trim(description, true, true);

		// Quotes: \" -> " and \' -> '
		description.Replace("\\\"", "\"", true);
		description.Replace("\\'", "'", true);

		// URL: [url=address]text[/url]
		{
			wxRegEx regEx(RAW(u8R"(\[url=([^\]]+)\]([^\[]+)\[\/url\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.ReplaceAll(&description, RAW(u8R"(<a href="\1">\2</a>)"));
		}

		// URL: [url]address[/url]
		{
			wxRegEx regEx(RAW(u8R"(\[url\]([^\[]+)\[\/url\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.ReplaceAll(&description, RAW(u8R"(<a href="\1">\1</a>)"));
		}

		// URL: [NEXUS ID: number]
		{
			wxRegEx regEx(RAW(u8R"(\[NEXUS\s?ID\:\s?(\d+)\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.ReplaceAll(&description, wxString::Format(RAW(u8R"(<a href="%s\/\1">\0</a>)"), GetModURLBasePart()));
		}

		// Image: [img]address[/img]
		{
			wxRegEx regEx(RAW(u8R"(\[img\]([^\[]+)\[\/img\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.ReplaceAll(&description, RAW(u8R"(<img src="\1"/>)"));
		}

		// Align: [left|right|center|justify]text[/left|right|center|justify]
		{
			wxRegEx regEx(RAW(u8R"(\[(left|right|center|justify)\](.*)\[\/(?:left|right|center|justify)\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.ReplaceAll(&description, RAW(u8R"(<div align="\1">\2</div>)"));
		}

		// Font size: [size=string|number]text[/size]
		{
			wxRegEx regEx(RAW(u8R"((?s)\[size=\\?"?(\d+)\\?"?\](.*?)\[\/size\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.ReplaceAll(&description, RAW(u8R"(<font size="\1">\2</font>)"));
		}

		// Font color: [color="name"]text[/color]
		{
			wxRegEx regEx(RAW(u8R"((?s)\[color=\\?"?(\w+|\#\d+)\\?"?\](.*?)\[\/color\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.ReplaceAll(&description, RAW(u8R"(<font color="\1">\2</font>)"));
		}

		// Font color: [color=#AABBCCDD]text[/color]
		{
			wxRegEx regEx(RAW(u8R"(\[color=#([ABCDEF\d]+)\](.+)\[\/color\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.ReplaceAll(&description, RAW(u8R"(<font color="#\1">\2</font>)"));
		}

		// Font color: [color=#AABBCCDD]text[/color]
		{
			wxRegEx regEx(RAW(u8R"((?s)\[list\](.*?)\[\/list\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			regEx.ReplaceAll(&description, RAW(u8R"(<ul>\1</ul>)"));
		}

		// Simple container tags: [b]text[/b]
		auto ExpSimple = [&description, &RAW](const wxString& tagName, const wxString& sTagNameRepl = wxEmptyString)
		{
			wxString regExQuery = wxString::Format(RAW(u8R"((?s)\[%s\](.*?)\[\/%s\])"), tagName, tagName);
			wxString repl = wxString::Format(RAW(u8R"(<%s>\1</%s>)"), sTagNameRepl, sTagNameRepl);

			wxRegEx regEx(regExQuery, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			if (regEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE))
			{
				regEx.ReplaceAll(&description, repl);
			}
		};

		ExpSimple("b", "b");
		ExpSimple("i", "i");
		ExpSimple("u", "u");
		ExpSimple("\\*", "li");

		// Buggy
		if constexpr(false)
		{
			// URL. Matches string starting only with 'http://', 'https://', 'ftp://' or 'www.'.
			// This tries not to match addresses inside <a></a> tags. And this better to be the last replacement.
			wxRegEx regEx(RAW(u8R"((?:[^<a href="">])(http:\/\/|https:\/\/\ftp:\/\/|www.)([\w_-]+(?:(?:\.[\w_-]+)+))([\w.,@?^=%&:\/~+#-]*[\w@?^=%&\/~+#-])?(?:[^<\/a>]))"), wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
			if (regEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE))
			{
				regEx.ReplaceAll(&description, RAW(u8R"(<a href="\1\2\3">\0</a>)"));
			}
		}

		return description;
	}
	wxString NexusProvider::GetModURLBasePart(const GameID& id) const
	{
		return wxString::Format("https://www.nexusmods.com/%s/mods", GetGameID(id)).MakeLower();
	}
	wxString NexusProvider::GetModURL(ModID modID, const wxString& modSignature, const GameID& id)
	{
		return KxString::Format("%1/%2", GetModURLBasePart(id), modID.GetValue());
	}

	std::unique_ptr<IModInfo> NexusProvider::GetModInfo(ModID modID, const wxAny& extraInfo, const GameID& id) const
	{
		KxCURLSession connection(KxString::Format("%1/games/%2/mods/%3", GetAPIURL(), GetGameID(id), modID.GetValue()));
		KxCURLReply reply = ConfigureRequest(connection).Send();

		auto info = std::make_unique<Nexus::ModInfo>();
		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			info->SetShouldTryLater();
			return info;
		}

		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			auto& data = info->m_Data;

			data.ID = modID;
			data.Name = json["name"].get<wxString>();
			data.m_Summary = json["summary"].get<wxString>();
			data.m_Description = json["description"].get<wxString>();
			data.Author = json["author"].get<wxString>();
			data.Uploader = json["uploaded_by"].get<wxString>();
			data.UploaderProfile = json["uploaded_users_profile_url"].get<wxString>();
			data.MainImage = json["picture_url"].get<wxString>();

			data.Version = json["version"].get<wxString>();
			data.UploadDate = ReadDateTime(json["created_time"]);
			data.m_LastUpdateDate = ReadDateTime(json["updated_time"]);

			data.ContainsAdultContent = json["contains_adult_content?"];

			// Primary file
			if (auto primaryFileIt = json.find("primary_file"); primaryFileIt != json.end())
			{
				auto fileInfo = std::make_unique<Nexus::ModFileInfo>();
				ReadFileInfo(*primaryFileIt, fileInfo->m_Data);
				info->m_PrimaryFile = std::move(fileInfo);
			}

			// Endorsement state
			auto endorsementStateIt = json.find("endorsement");
			if (endorsementStateIt != json.end())
			{
				if (*endorsementStateIt == "Endorse")
				{
					data.EndorsementState = ModEndorsement::Endorsed();
				}
				else if (*endorsementStateIt == "Abstain")
				{
					data.EndorsementState = ModEndorsement::Abstained();
				}
				else
				{
					data.EndorsementState = ModEndorsement::Undecided();
				}
			}
		}
		catch (...)
		{
			ReportRequestError(*this, reply);
			return nullptr;
		}

		if (info->IsOK())
		{
			return info;
		}

		ReportRequestError(*this, reply);
		return nullptr;
	}
	std::unique_ptr<IModFileInfo> NexusProvider::GetFileInfo(ModID modID, ModFileID fileID, const wxAny& extraInfo, const GameID& id) const
	{
		KxCURLSession connection(KxString::Format("%1/games/%2/mods/%3/files/%4", GetAPIURL(), GetGameID(id), modID.GetValue(), fileID.GetValue()));
		KxCURLReply reply = ConfigureRequest(connection).Send();

		auto info = std::make_unique<Nexus::ModFileInfo>();
		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			info->SetShouldTryLater();
			return info;
		}

		try
		{
			KxJSONObject json = KxJSON::Load(reply);

			info->m_Data.ModID = modID;
			ReadFileInfo(json, info->m_Data);
		}
		catch (...)
		{
			ReportRequestError(*this, reply);
			return nullptr;
		}

		if (info->IsOK())
		{
			return info;
		}

		ReportRequestError(*this, reply);
		return nullptr;
	}
	IModFileInfo::Vector NexusProvider::GetFilesList(ModID modID, const wxAny& extraInfo, const GameID& id) const
	{
		KxCURLSession connection(KxString::Format("%1/games/%2/mods/%3/files", GetAPIURL(), GetGameID(id), modID.GetValue()));
		KxCURLReply reply = ConfigureRequest(connection).Send();

		IModFileInfo::Vector infoVector;
		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			infoVector.emplace_back(std::make_unique<Nexus::ModFileInfo>())->SetShouldTryLater();
			return infoVector;
		}

		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			infoVector.reserve(json.size());

			for (const KxJSONObject& value: json["files"])
			{
				auto info = std::make_unique<Nexus::ModFileInfo>();
				info->m_Data.ModID = modID;
				ReadFileInfo(value, info->m_Data);

				if (info->IsOK())
				{
					infoVector.emplace_back(std::move(info));
				}
			}
		}
		catch (...)
		{
			ReportRequestError(*this, reply);
			infoVector.clear();
		}
		return infoVector;
	}
	IModDownloadInfo::Vector NexusProvider::GetFileDownloadLinks(ModID modID, ModFileID fileID, const wxAny& extraInfo, const GameID& id) const
	{
		wxString query = KxString::Format("%1/games/%2/mods/%3/files/%4/download_link",
										  GetAPIURL(),
										  GetGameID(id),
										  modID.GetValue(),
										  fileID.GetValue()
		);

		Nexus::Internal::ReplyStructs::ModDownloadInfoNXM nxmExtraInfo;
		if (extraInfo.GetAs(&nxmExtraInfo))
		{
			query += KxString::Format("?key=%1&expires=%2", nxmExtraInfo.Key, nxmExtraInfo.Expires);
		}

		KxCURLSession connection(query);
		KxCURLReply reply = ConfigureRequest(connection).Send();

		IModDownloadInfo::Vector infoVector;
		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			infoVector.emplace_back(std::make_unique<Nexus::ModDownloadInfo>())->SetShouldTryLater();
			return infoVector;
		}

		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			infoVector.reserve(json.size());

			for (const KxJSONObject& value: json)
			{
				auto info = std::make_unique<Nexus::ModDownloadInfo>();
				auto& data = info->m_Data;

				data.Name = value["name"].get<wxString>();
				data.ShortName = value["short_name"].get<wxString>();
				data.URL = ConvertUnicodeEscapes(value["URI"].get<wxString>());

				if (info->IsOK())
				{
					infoVector.emplace_back(std::move(info));
				}
			}
		}
		catch (...)
		{
			ReportRequestError(*this, reply);
			infoVector.clear();
		}
		return infoVector;
	}
	std::unique_ptr<IModEndorsementInfo> NexusProvider::EndorseMod(ModID modID, ModEndorsement state, const wxAny& extraInfo, const GameID& id)
	{
		KxCURLSession connection(KxString::Format("%1/games/%2/mods/%3/%4", GetAPIURL(), GetGameID(id), modID.GetValue(), EndorsementStateToString(state)));

		// I don't know why this request needs mod version, it's works even with fake version.
		connection.SetPostData(KxJSON::Save(KxJSONObject {{"Version", "x"}}));
		KxCURLReply reply = ConfigureRequest(connection).Send();

		auto info = std::make_unique<Nexus::ModEndorsementInfo>();
		auto& data = info->m_Data;

		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			info->SetShouldTryLater();
			return info;
		}

		try
		{
			KxJSONObject json = KxJSON::Load(reply);

			data.Message = json["message"].get<wxString>();

			auto statusIt = json.find("status");
			if (statusIt != json.end())
			{
				if (*statusIt == "Endorsed")
				{
					data.Endorsement = ModEndorsement::Endorsed();
				}
				else if (*statusIt == "Abstained")
				{
					data.Endorsement = ModEndorsement::Abstained();
				}
				else
				{
					data.Endorsement = ModEndorsement::Undecided();
				}
			}
		}
		catch (...)
		{
			ReportRequestError(*this, reply);
			return nullptr;
		}

		if (info->IsOK())
		{
			return info;
		}

		ReportRequestError(*this, reply);
		return nullptr;
	}

	std::unique_ptr<Nexus::ValidationInfo> NexusProvider::GetValidationInfo(const wxString& apiKey) const
	{
		KxCURLSession connection(KxString::Format("%1/users/validate", GetAPIURL()));
		KxCURLReply reply = ConfigureRequest(connection, apiKey).Send();

		auto info = std::make_unique<Nexus::ValidationInfo>();
		auto& data = info->m_Data;

		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			info->SetShouldTryLater();
			return info;
		}

		try
		{
			KxJSONObject json = KxJSON::Load(reply);

			data.UserID = json["user_id"];
			data.UserName = json["name"].get<wxString>();
			data.APIKey = json["key"].get<wxString>();
			data.EMail = json["email"].get<wxString>();
			data.ProfilePicture = json["profile_url"].get<wxString>();
			data.IsPremium = json["is_premium?"];
			data.IsSupporter = json["is_supporter?"];
		}
		catch (...)
		{
			ReportRequestError(*this, reply);
			return nullptr;
		}
		
		if (info->IsOK())
		{
			return info;
		}

		ReportRequestError(*this, reply);
		return nullptr;
	}
	std::unique_ptr<Nexus::GameInfo> NexusProvider::GetGameInfo(const GameID& id) const
	{
		KxCURLSession connection(KxString::Format("%1/games/%2", GetAPIURL(), GetGameID(id)));
		KxCURLReply reply = ConfigureRequest(connection).Send();

		auto info = std::make_unique<Nexus::GameInfo>();
		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			info->SetShouldTryLater();
			return info;
		}

		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			ReadGameInfo(json, info->m_Data);
		}
		catch (...)
		{
			ReportRequestError(*this, reply);
			return nullptr;
		}
		
		if (info->IsOK())
		{
			return info;
		}

		ReportRequestError(*this, reply);
		return nullptr;
	}
	Nexus::GameInfo::Vector NexusProvider::GetGamesList() const
	{
		KxCURLSession connection(KxString::Format("%1/games", GetAPIURL()));
		KxCURLReply reply = ConfigureRequest(connection).Send();

		Nexus::GameInfo::Vector infoVector;
		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			infoVector.emplace_back(std::make_unique<Nexus::GameInfo>())->SetShouldTryLater();
			return infoVector;
		}

		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			infoVector.reserve(json.size());

			for (const KxJSONObject& value: json)
			{
				auto info = std::make_unique<Nexus::GameInfo>();
				ReadGameInfo(value, info->m_Data);

				if (info->IsOK())
				{
					infoVector.emplace_back(std::move(info));
				}
			}
		}
		catch (...)
		{
			ReportRequestError(*this, reply);
			infoVector.clear();
		}
		return infoVector;
	}
	Nexus::IssueInfo::Vector NexusProvider::GetIssues() const
	{
		KxCURLSession connection(KxString::Format("%1/feedbacks/list_user_issues/", GetAPIURL()));
		KxCURLReply reply = ConfigureRequest(connection).Send();

		Nexus::IssueInfo::Vector infoVector;
		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			infoVector.emplace_back(std::make_unique<Nexus::IssueInfo>())->SetShouldTryLater();
			return infoVector;
		}

		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			infoVector.reserve(json.size());

			for (const KxJSONObject& value: json["issues"])
			{
				auto info = std::make_unique<Nexus::IssueInfo>();
				if (info->IsOK())
				{
					infoVector.emplace_back(std::move(info));
				}
			}
		}
		catch (...)
		{
			ReportRequestError(*this, reply);
			infoVector.clear();
		}
		return infoVector;
	}

	GameID NexusProvider::TranslateNxmGameID(const wxString& id) const
	{
		if (!id.IsEmpty())
		{
			// TES
			if (KxComparator::IsEqual(id, "morrowind"))
			{
				return GameIDs::Morrowind;
			}
			if (KxComparator::IsEqual(id, "oblivion"))
			{
				return GameIDs::Oblivion;
			}
			if (KxComparator::IsEqual(id, "skyrim"))
			{
				return GameIDs::Skyrim;
			}
			if (KxComparator::IsEqual(id, "skyrimse"))
			{
				return GameIDs::SkyrimSE;
			}

			// Fallout
			if (KxComparator::IsEqual(id, "fallout3"))
			{
				return GameIDs::Fallout3;
			}
			if (KxComparator::IsEqual(id, "falloutnv"))
			{
				return GameIDs::FalloutNV;
			}
			if (KxComparator::IsEqual(id, "fallout4"))
			{
				return GameIDs::Fallout4;
			}
		}
		return GameIDs::NullGameID;
	}
	wxString NexusProvider::ConstructNXM(const IModFileInfo& fileInfo, const GameID& id) const
	{
		return KxString::Format("nxm://%1/mods/%2/files/%3", GetGameID(id), fileInfo.GetModID().GetValue(), fileInfo.GetID().GetValue()).Lower();
	}
}
