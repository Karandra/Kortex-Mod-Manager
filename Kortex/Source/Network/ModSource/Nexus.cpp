#include "stdafx.h"
#include "Nexus.h"
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Application.hpp>
#include <Kortex/Events.hpp>
#include "UI/KMainWindow.h"
#include "Utility/String.h"
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxJSON.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxIndexedEnum.h>
#include <KxFramework/KxWebSocket.h>

namespace
{
	using namespace Kortex;
	using namespace Kortex::NetworkManager;
	using TJsonValue = typename nlohmann::json::value_type;

	struct CategoryDef: public KxIndexedEnum::Definition<CategoryDef, IModFileInfo::CategoryID, wxString>
	{
		inline static const TItem ms_Index[] = 
		{
			{IModFileInfo::CategoryID::Main, wxS("MAIN")},
			{IModFileInfo::CategoryID::Optional, wxS("OPTIONAL")},
		};
	};

	enum class NetworkSoftware
	{
		CURL,
		WebSocket,
	};
	wxString GetUserAgent(NetworkSoftware networkSoftware)
	{
		const IApplication* app = IApplication::GetInstance();
		KxFormat formatter("%1/%2 (Windows_NT %3; %4) %5/%6");

		// Application name and version
		formatter(app->GetShortName());
		formatter(app->GetVersion());

		// Windows version
		auto versionInfo = KxSystem::GetVersionInfo();
		formatter(KxString::Format("%1.%2.%3", versionInfo.MajorVersion, versionInfo.MinorVersion, versionInfo.BuildNumber));

		// System "bitness" (x86/x64)
		formatter(KVarExp("$(SystemArchitectureName)"));

		// Network software
		switch (networkSoftware)
		{
			case NetworkSoftware::CURL:
			{
				formatter(KxCURL::GetLibraryName());
				formatter(KxCURL::GetLibraryVersion());
				break;
			}
			case NetworkSoftware::WebSocket:
			{
				formatter(KxWebSocket::GetLibraryName());
				formatter(KxWebSocket::GetLibraryVersion());
				break;
			}
			default:
			{
				formatter("Unknown");
				formatter("0.0");
			}
		};

		return formatter;
	}
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
		info.DisplayName = ConvertDisplayName(json["name"].get<wxString>());
		info.Version = json["version"].get<wxString>();
		info.ChangeLog = ConvertChangeLog(json["changelog_html"].get<wxString>());
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
		INotificationCenter::GetInstance()->NotifyUsing<INetworkManager>(KTrf("Network.RequestQuotaReched", nexus.GetName()), KxICON_WARNING);
	}
	void ReportRequestError(const NexusProvider& nexus, const wxString& message)
	{
		try
		{
			KxJSONObject json = KxJSON::Load(message);
			INotificationCenter::GetInstance()->NotifyUsing<INetworkManager>(json["message"].get<wxString>(), KxICON_ERROR);
		}
		catch (...)
		{
			INotificationCenter::GetInstance()->NotifyUsing<INetworkManager>(message, KxICON_ERROR);
		}
	}
}

namespace Kortex::NetworkManager
{
	wxWindow* NexusProvider::GetInvokingWindow() const
	{
		return KMainWindow::GetInstance();
	}
	KxStandardID NexusProvider::OnAuthSuccess()
	{
		m_WebSocketClient.reset();
		m_IsAuthenticated = true;

		IEvent::CallAfter([this]()
		{
			IAuthenticableModSource::OnAuthSuccess();
			INetworkManager::GetInstance()->OnAuthStateChanged();
		});
		return KxID_OK;
	}
	KxStandardID NexusProvider::OnAuthFail()
	{
		m_WebSocketClient.reset();
		m_UserToken.clear();
		m_SessionGUID = UUID {0};
		m_IsAuthenticated = false;

		IEvent::CallAfter([this]()
		{
			IAuthenticableModSource::OnAuthFail();
			INetworkManager::GetInstance()->OnAuthStateChanged();
		});
		return KxID_OK;
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
		const IApplication* app = IApplication::GetInstance();

		request.AddHeader("APIKey", apiKey.IsEmpty() ? GetAPIKey() : apiKey);
		request.AddHeader("Content-Type", "application/json");
		request.AddHeader("Protocol-Version", "0.15.5");
		request.AddHeader("Application-Name", app->GetShortName());
		request.AddHeader("Application-Version", app->GetVersion());
		request.SetUserAgent(m_UserAgent);

		return request;
	}
	bool NexusProvider::ShouldTryLater(const KxCURLReplyBase& reply) const
	{
		return reply.GetResponseCode() == KxHTTPStatusCode::TooManyRequests;
	}
	wxString NexusProvider::GetAPIURL() const
	{
		return "https://api.nexusmods.com/v1";
	}
	wxString NexusProvider::GetAPIKey(wxString* userName) const
	{
		if (auto credentials = LoadCredentials())
		{
			KxUtility::SetIfNotNull(userName, credentials->UserID);
			return credentials->Password.GetAsString();
		}
		return {};
	}
	void NexusProvider::RequestUserAvatar(Nexus::ValidationInfo& info)
	{
		if (!HasUserPicture())
		{
			SetUserPicture(DownloadSmallBitmap(info.GetProfilePicture()));
		}
	}

	NexusProvider::NexusProvider()
		:m_CredentialsStore(wxS("Kortex/NexusMods")), m_UserAgent(GetUserAgent(NetworkSoftware::CURL))
	{
	}

	bool NexusProvider::Authenticate()
	{
		m_WebSocketClient = KxWebSocket::NewSecureClient("wss://sso.nexusmods.com");
		m_WebSocketClient->SetUserAgent(GetUserAgent(NetworkSoftware::WebSocket));

		m_WebSocketClient->Bind(KxEVT_WEBSOCKET_CONNECTING, [this](KxWebSocketEvent& event)
		{
			m_WebSocketClient->AddHeader("Application-Name", IApplication::GetInstance()->GetShortName());
			m_WebSocketClient->AddHeader("Application-Version", IApplication::GetInstance()->GetVersion());
		});
		m_WebSocketClient->Bind(KxEVT_WEBSOCKET_OPEN, [this](KxWebSocketEvent& event)
		{
			if (m_SessionGUID.IsNull())
			{
				m_SessionGUID.Create();
			}

			const wxString guid = m_SessionGUID.ToString().MakeLower();
			const wxString appID = wxS("kortex");
			KxJSONObject json =
			{
				{"id", guid},
				{"appid", appID},
				{"token", nullptr},
				{"protocol", 2}
			};
			if (!m_UserToken.IsEmpty())
			{
				json["token"] = m_UserToken;
			}
			INotificationCenter::GetInstance()->Notify("GUID", guid, KxICON_INFO);

			m_WebSocketClient->Send(KxJSON::Save(json));
			wxString openURL = KxString::Format("https://www.nexusmods.com/sso?id=%1&application=%2", guid, appID);
			KxShell::Execute(GetInvokingWindow(), KxShell::GetDefaultViewer("html"), "open", openURL);
		});
		m_WebSocketClient->Bind(KxEVT_WEBSOCKET_MESSAGE, [this](KxWebSocketEvent& event)
		{
			try
			{
				const KxJSONObject json = KxJSON::Load(event.GetTextMessage());
				if (json["success"] == true)
				{
					const KxJSONObject& data = json["data"];

					// Just connected, save token
					if (auto it = data.find("connection_token"); it != data.end())
					{
						m_UserToken = it->get<wxString>();
						INotificationCenter::GetInstance()->Notify("UserToken", m_UserToken, KxICON_INFO);

						if (m_UserToken.IsEmpty())
						{
							OnAuthFail();
						}
						return;
					}

					// Authenticated, request and validate user info
					if (auto it = data.find("api_key"); it != data.end())
					{
						const wxString apiKey = it->get<wxString>();
						INotificationCenter::GetInstance()->Notify("API Key", apiKey, KxICON_INFO);

						auto info = GetValidationInfo(apiKey);
						if (info->IsOK() && info->GetAPIKey() == apiKey)
						{
							if (SaveCredentials(Credentials(info->GetUserName(), apiKey)))
							{
								RequestUserAvatar(*info);
								OnAuthSuccess();
								return;
							}
						}

						OnAuthFail();
						return;
					}
				}
			}
			catch (...)
			{
			}
			OnAuthFail();
		});
		m_WebSocketClient->Bind(KxEVT_WEBSOCKET_CLOSE, [this](KxWebSocketEvent& event)
		{
			INotificationCenter::GetInstance()->Notify("WSS", "OnClose", KxICON_INFO);

			if (!LoadCredentials())
			{
				OnAuthFail();
			}
			m_WebSocketClient.reset();
		});
		m_WebSocketClient->Bind(KxEVT_WEBSOCKET_FAIL, [this](KxWebSocketEvent& event)
		{
			INotificationCenter::GetInstance()->Notify("WSS", "OnFail", KxICON_ERROR);
			OnAuthFail();
		});

		return m_WebSocketClient->Connect();
	}
	bool NexusProvider::ValidateAuth()
	{
		if (auto info = GetValidationInfo())
		{
			RequestUserAvatar(*info);
			m_IsAuthenticated = info->GetAPIKey() == GetAPIKey();
		}
		else
		{
			m_IsAuthenticated = false;
		}
		return m_IsAuthenticated;
	}
	bool NexusProvider::SignOut()
	{
		m_IsAuthenticated = false;
		return m_CredentialsStore.Delete();
	}
	bool NexusProvider::IsAuthenticated() const
	{
		return m_IsAuthenticated;
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
		return IModSource::GetGameID(id);
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
	wxString NexusProvider::GetModURL(const ProviderRequest& request)
	{
		return KxString::Format("%1/%2", GetModURLBasePart(request.GetGameID()), request.GetModID().GetValue());
	}

	bool NexusProvider::RestoreBrokenDownload(const wxString& filePath, IDownloadEntry& download)
	{
		wxString name = filePath.AfterLast(wxS('\\'));
		wxRegEx reg(u8R"((.*?)\-(\d+)\-(.*)\.)", wxRE_EXTENDED|wxRE_ADVANCED|wxRE_ICASE);
		if (reg.Matches(name))
		{
			// Mod ID
			ModID modID(reg.GetMatch(name, 2));
			if (modID)
			{
				ProviderRequest request(modID, {}, download.GetTargetGameID());
				for (auto& fileInfo: GetFilesList(request))
				{
					if (fileInfo->GetName() == name)
					{
						// Fix size discrepancy caused by Nexus sending size in kilobytes
						constexpr const int64_t oneKB = 1024 * 1024;
						const int64_t downloadedSize = download.GetDownloadedSize();
						const int64_t difference = downloadedSize - fileInfo->GetSize();
						if (difference > 0 && difference <= oneKB)
						{
							fileInfo->SetSize(downloadedSize);
						}

						download.SetFileInfo(std::move(fileInfo));
						return true;
					}
				}
			}

			IModFileInfo& fileInfo = download.GetFileInfo();

			// If we got here, file is not found on Nexus, but we can try to restore as much as possible from the file name itself.
			// Set mod ID
			fileInfo.SetModID(modID);

			// Display name
			wxString name = reg.GetMatch(fileInfo.GetName(), 1);
			name.Replace("_", " ");
			fileInfo.SetDisplayName(name);

			// File version
			wxString version = reg.GetMatch(fileInfo.GetName(), 2);
			version.Replace("-", ".");
			fileInfo.SetVersion(version);

			// Still return fail status
			return false;
		}
		return false;
	}

	std::unique_ptr<IModInfo> NexusProvider::GetModInfo(const ProviderRequest& request) const
	{
		KxCURLSession connection(KxString::Format("%1/games/%2/mods/%3",
												  GetAPIURL(),
												  GetGameID(request.GetGameID()),
												  request.GetModID().GetValue())
		);
		KxCURLReply reply = ConfigureRequest(connection).Send();
		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			return nullptr;
		}

		auto info = std::make_unique<Nexus::ModInfo>();
		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			auto& data = info->m_Data;

			data.ID = request.GetModID();
			data.Name = json["name"].get<wxString>();
			data.m_Summary = json["summary"].get<wxString>();
			data.m_Description = json["description"].get<wxString>();
			data.Author = json["author"].get<wxString>();
			data.Uploader = json["uploaded_by"].get<wxString>();
			data.UploaderProfile = json["uploaded_users_profile_url"].get<wxString>();
			data.MainImage = json["picture_url"].get<wxString>();

			data.Version = json["version"].get<wxString>();
			data.UploadDate = ReadDateTime(json["created_time"]);
			data.LastUpdateDate = ReadDateTime(json["updated_time"]);

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
	std::unique_ptr<IModFileInfo> NexusProvider::GetFileInfo(const ProviderRequest& request) const
	{
		KxCURLSession connection(KxString::Format("%1/games/%2/mods/%3/files/%4",
												  GetAPIURL(),
												  GetGameID(request),
												  request.GetModID().GetValue(),
												  request.GetFileID().GetValue())
		);
		KxCURLReply reply = ConfigureRequest(connection).Send();
		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			return nullptr;
		}

		auto info = std::make_unique<Nexus::ModFileInfo>();
		try
		{
			KxJSONObject json = KxJSON::Load(reply);

			info->m_Data.ModID = request.GetModID();
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
	IModFileInfo::Vector NexusProvider::GetFilesList(const ProviderRequest& request) const
	{
		KxCURLSession connection(KxString::Format("%1/games/%2/mods/%3/files",
												  GetAPIURL(),
												  GetGameID(request),
												  request.GetModID().GetValue())
		);
		KxCURLReply reply = ConfigureRequest(connection).Send();
		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			return {};
		}

		IModFileInfo::Vector infoVector;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			infoVector.reserve(json.size());

			for (const KxJSONObject& value: json["files"])
			{
				auto info = std::make_unique<Nexus::ModFileInfo>();
				info->m_Data.ModID = request.GetModID();
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
	IModDownloadInfo::Vector NexusProvider::GetFileDownloadLinks(const ProviderRequest& request) const
	{
		wxString query = KxString::Format("%1/games/%2/mods/%3/files/%4/download_link",
										  GetAPIURL(),
										  GetGameID(request),
										  request.GetModID().GetValue(),
										  request.GetFileID().GetValue()
		);

		Nexus::Internal::ReplyStructs::ModDownloadInfoNXM nxmExtraInfo;
		if (request.GetExtraInfo(nxmExtraInfo))
		{
			query += KxString::Format("?key=%1&expires=%2", nxmExtraInfo.Key, nxmExtraInfo.Expires);
		}
		KxCURLSession connection(query);

		KxCURLReply reply = ConfigureRequest(connection).Send();
		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			return {};
		}

		IModDownloadInfo::Vector infoVector;
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
	std::unique_ptr<IModEndorsementInfo> NexusProvider::EndorseMod(const ProviderRequest& request, ModEndorsement state)
	{
		KxCURLSession connection(KxString::Format("%1/games/%2/mods/%3/%4",
												  GetAPIURL(),
												  GetGameID(request),
												  request.GetModID().GetValue(),
												  EndorsementStateToString(state))
		);

		KxVersion modVersion;
		if (request.GetExtraInfo(modVersion))
		{
			connection.SetPostData(KxJSON::Save(KxJSONObject {{"Version", modVersion.ToString()}}));
		}
		else
		{
			connection.SetPostData(KxJSON::Save(KxJSONObject {{"Version", "x"}}));
		}
		KxCURLReply reply = ConfigureRequest(connection).Send();

		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			return nullptr;
		}

		auto info = std::make_unique<Nexus::ModEndorsementInfo>();
		try
		{
			KxJSONObject json = KxJSON::Load(reply);

			auto& data = info->m_Data;
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
		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			return nullptr;
		}

		auto info = std::make_unique<Nexus::ValidationInfo>();
		try
		{
			KxJSONObject json = KxJSON::Load(reply);

			auto& data = info->m_Data;
			data.UserID = json["user_id"];
			data.UserName = json["name"].get<wxString>();
			data.APIKey = json["key"].get<wxString>();
			data.EMail = json["email"].get<wxString>();
			data.ProfilePicture = json["profile_url"].get<wxString>();
			data.IsPremium = json["is_premium"];
			data.IsSupporter = json["is_supporter"];
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
		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			return nullptr;
		}

		auto info = std::make_unique<Nexus::GameInfo>();
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
		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			return {};
		}

		Nexus::GameInfo::Vector infoVector;
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
		if (ShouldTryLater(reply))
		{
			ReportRequestQuoteReached(*this);
			return {};
		}

		Nexus::IssueInfo::Vector infoVector;
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
