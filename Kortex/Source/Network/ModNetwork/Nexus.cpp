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

	struct CategoryDef: public KxIndexedEnum::Definition<CategoryDef, ModFileCategory, wxString>
	{
		inline static const TItem ms_Index[] = 
		{
			{ModFileCategory::Main, wxS("MAIN")},
			{ModFileCategory::Optional, wxS("OPTIONAL")},
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
	void ReadFileInfo(const TJsonValue& json, ModFileReply& info)
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
		info.Category = CategoryDef::FromString(json["category_name"].get<wxString>(), ModFileCategory::Unknown);
	}
	void ReadGameInfo(const TJsonValue& json, NexusGameReply& info)
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

	void ReportRequestError(const wxString& message)
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
	bool TestRequestError(const KxCURLReplyBase& reply, const wxString& message = {}, bool noErrorReport = false)
	{
		if (reply.GetResponseCode() == KxHTTPStatusCode::TooManyRequests)
		{
			if (!noErrorReport)
			{
				INotificationCenter::GetInstance()->NotifyUsing<INetworkManager>(KTrf("NetworkManager.RequestQuotaReched",
																				 NexusModNetwork::GetInstance()->GetName()),
																				 KxICON_WARNING);
			}
			return true;
		}
		else if (!reply.IsOK())
		{
			if (!noErrorReport)
			{
				ReportRequestError(message);
			}
			return true;
		}
		return false;
	}
}

namespace Kortex::NetworkManager
{
	wxWindow* NexusModNetwork::GetInvokingWindow() const
	{
		return KMainWindow::GetInstance();
	}
	KxStandardID NexusModNetwork::OnAuthSuccess()
	{
		m_WebSocketClient.reset();
		m_IsAuthenticated = true;

		IEvent::CallAfter([this]()
		{
			IAuthenticableModNetwork::OnAuthSuccess();
			INetworkManager::GetInstance()->OnAuthStateChanged();
		});
		return KxID_OK;
	}
	KxStandardID NexusModNetwork::OnAuthFail()
	{
		m_WebSocketClient.reset();
		m_UserToken.clear();
		m_SessionGUID = UUID {0};
		m_IsAuthenticated = false;

		IEvent::CallAfter([this]()
		{
			IAuthenticableModNetwork::OnAuthFail();
			INetworkManager::GetInstance()->OnAuthStateChanged();
		});
		return KxID_OK;
	}

	wxString NexusModNetwork::EndorsementStateToString(const ModEndorsement& state) const
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
	std::unique_ptr<KxCURLSession> NexusModNetwork::NewCURLSession(const wxString& address, const wxString& apiKey) const
	{
		auto session = INetworkManager::GetInstance()->NewCURLSession(address);
		session->AddHeader(wxS("APIKey"), apiKey.IsEmpty() ? GetAPIKey() : apiKey);
		session->AddHeader(wxS("Content-Type"), wxS("application/json"));
		session->AddHeader(wxS("Protocol-Version"), wxS("0.15.5"));

		session->Bind(KxEVT_CURL_RESPONSE_HEADER, &NexusModNetwork::OnResponseHeader, const_cast<NexusModNetwork*>(this));

		return session;
	}
	void NexusModNetwork::OnResponseHeader(KxCURLEvent& event)
	{
		const wxString headerName = event.GetHeaderKey();

		auto ToInt = [&event]() -> std::optional<int>
		{
			if (long intValue = -1; event.GetHeaderValue().ToCLong(&intValue))
			{
				return intValue;
			}
			return std::nullopt;
		};
		auto TestInt = [&headerName, &ToInt](const wxChar* name, int& ref)
		{
			if (headerName == name)
			{
				if (auto value = ToInt())
				{
					ref = *value;
				}
			}
		};
		auto TestISODate = [&headerName, &event](const wxChar* name, wxDateTime& ref)
		{
			if (headerName == name)
			{
				ref.ParseISOCombined(event.GetHeaderValue());
			}
		};

		TestInt(wxS("X-RL-Hourly-Limit"), m_LimitsData.HourlyLimit);
		TestInt(wxS("X-RL-Hourly-Remaining"), m_LimitsData.HourlyRemaining);
		TestISODate(wxS("X-RL-Hourly-Reset"), m_LimitsData.HourlyLimitReset);

		TestInt(wxS("X-RL-Daily-Limit"), m_LimitsData.DailyLimit);
		TestInt(wxS("X-RL-Daily-Remaining"), m_LimitsData.DailyRemaining);
		TestISODate(wxS("X-RL-Reset-Reset"), m_LimitsData.DailyLimitReset);
	}

	wxString NexusModNetwork::GetAPIURL() const
	{
		return wxS("https://api.nexusmods.com/v1");
	}
	wxString NexusModNetwork::GetAPIKey(wxString* userName) const
	{
		if (auto credentials = LoadCredentials())
		{
			KxUtility::SetIfNotNull(userName, credentials->UserID);
			return credentials->Password.GetAsString();
		}
		return {};
	}
	void NexusModNetwork::RequestUserAvatar(const NexusValidationReply& info)
	{
		if (!HasUserPicture() && !LoadUserPicture())
		{
			SetUserPicture(DownloadSmallBitmap(info.ProfilePicture));
		}
	}

	NexusModNetwork::NexusModNetwork()
		:m_CredentialsStore(wxS("Kortex/NexusMods"))
	{
	}

	bool NexusModNetwork::Authenticate()
	{
		m_WebSocketClient = INetworkManager::GetInstance()->NewWebSocketClient(wxS("wss://sso.nexusmods.com"));

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
			m_WebSocketClient->Send(KxJSON::Save(json));

			const wxString openURL = KxString::Format(wxS("https://www.nexusmods.com/sso?id=%1&application=%2"), guid, appID);
			KxShell::Execute(GetInvokingWindow(), KxShell::GetDefaultViewer(wxS("html")), wxS("open"), openURL);
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
						auto info = DoGetValidationInfo(apiKey);

						if (info && info->APIKey == apiKey)
						{
							if (SaveCredentials(Credentials(info->UserName, apiKey)))
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
	bool NexusModNetwork::ValidateAuth()
	{
		// Load API Key from credentials store
		if (wxString apiKey = GetAPIKey(); !apiKey.IsEmpty())
		{
			// If succeed compare it with key that Nexus returns
			if (auto info = DoGetValidationInfo({}, true))
			{
				RequestUserAvatar(*info);

				m_IsAuthenticated = info->APIKey == apiKey;
				return m_IsAuthenticated;
			}
		}

		m_IsAuthenticated = false;
		return m_IsAuthenticated;
	}
	bool NexusModNetwork::SignOut()
	{
		m_IsAuthenticated = false;
		return m_CredentialsStore.Delete();
	}
	bool NexusModNetwork::IsAuthenticated() const
	{
		return m_IsAuthenticated;
	}

	KImageEnum NexusModNetwork::GetIcon() const
	{
		return KIMG_SITE_NEXUS;
	}
	wxString NexusModNetwork::GetName() const
	{
		return wxS("NexusMods");
	}
	
	wxString NexusModNetwork::TranslateGameIDToNetwork(const GameID& id) const
	{
		// If invalid ID is passed, return ID for current instance.
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
		return IModNetwork::TranslateGameIDToNetwork(id);
	}
	GameID NexusModNetwork::TranslateGameIDFromNetwork(const wxString& id) const
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
	void NexusModNetwork::ConvertDescriptionText(wxString& description) const
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
			regEx.ReplaceAll(&description, wxString::Format(RAW(u8R"(<a href="%s\/\1">\0</a>)"), GetModPageBaseURL()));
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
	}
	
	wxString NexusModNetwork::GetModPageBaseURL(const GameID& id) const
	{
		return KxString::Format(wxS("https://www.nexusmods.com/%1/mods"), TranslateGameIDToNetwork(id)).MakeLower();
	}
	wxString NexusModNetwork::GetModPageURL(const ModRepositoryRequest& request)
	{
		return KxString::Format(wxS("%1/%2"), GetModPageBaseURL(request.GetGameID()), request.GetModID().GetValue());
	}

	bool NexusModNetwork::RestoreBrokenDownload(const KxFileItem& fileItem, IDownloadEntry& download)
	{
		wxString name = fileItem.GetName();
		wxRegEx reg(u8R"((.*?)\-(\d+)\-(.*)\.)", wxRE_EXTENDED|wxRE_ADVANCED|wxRE_ICASE);
		if (reg.Matches(name))
		{
			// Mod ID
			ModID modID(reg.GetMatch(name, 2));
			if (modID)
			{
				ModRepositoryRequest request(modID, {}, download.GetTargetGameID());
				for (ModFileReply& fileInfo: GetModFiles(request))
				{
					if (fileInfo.Name == name)
					{
						// Fix size discrepancy caused by Nexus sending size in kilobytes
						constexpr const int64_t oneKB = 1024 * 1024;
						const int64_t downloadedSize = download.GetDownloadedSize();
						const int64_t difference = downloadedSize - fileInfo.Size;
						if (difference > 0 && difference <= oneKB)
						{
							fileInfo.Size = downloadedSize;
						}

						download.GetFileInfo() = fileInfo;
						return true;
					}
				}
			}

			// If we got here, file is not found on Nexus, but we can try to restore as much as possible from the file name itself.
			ModFileReply& fileInfo = download.GetFileInfo();

			// Set mod ID
			fileInfo.ModID = modID;

			// Display name
			wxString displayName = reg.GetMatch(name, 1);
			displayName.Replace("_", " ");
			fileInfo.DisplayName = displayName;

			// File version
			wxString version = reg.GetMatch(name, 2);
			version.Replace("-", ".");
			fileInfo.Version = version;

			// Still return fail status
			return false;
		}
		return false;
	}

	std::optional<ModInfoReply> NexusModNetwork::GetModInfo(const ModRepositoryRequest& request) const
	{
		auto connection = NewCURLSession(KxString::Format("%1/games/%2/mods/%3",
										 GetAPIURL(),
										 TranslateGameIDToNetwork(request),
										 request.GetModID().GetValue())
		);
		KxCURLReply reply = connection->Send();
		if (TestRequestError(reply, reply))
		{
			return std::nullopt;
		}

		ModInfoReply info;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);

			info.ID = request.GetModID();
			info.Name = json["name"].get<wxString>();
			info.Summary = json["summary"].get<wxString>();
			info.Description = json["description"].get<wxString>();
			info.Author = json["author"].get<wxString>();
			info.Uploader = json["uploaded_by"].get<wxString>();
			info.UploaderProfile = json["uploaded_users_profile_url"].get<wxString>();
			info.MainImage = json["picture_url"].get<wxString>();

			info.Version = json["version"].get<wxString>();
			info.UploadDate = ReadDateTime(json["created_time"]);
			info.LastUpdateDate = ReadDateTime(json["updated_time"]);

			info.ContainsAdultContent = json["contains_adult_content"];

			// Primary file
			if (auto primaryFileIt = json.find("primary_file"); primaryFileIt != json.end())
			{
				ReadFileInfo(*primaryFileIt, info.PrimaryFile);
			}

			// Endorsement state
			auto endorsementStateIt = json.find("endorsement");
			if (endorsementStateIt != json.end())
			{
				if (*endorsementStateIt == "Endorse")
				{
					info.EndorsementState = ModEndorsement::Endorsed();
				}
				else if (*endorsementStateIt == "Abstain")
				{
					info.EndorsementState = ModEndorsement::Abstained();
				}
				else
				{
					info.EndorsementState = ModEndorsement::Undecided();
				}
			}
		}
		catch (...)
		{
			ReportRequestError(reply);
			return std::nullopt;
		}
		return info;
	}
	std::optional<ModEndorsementReply> NexusModNetwork::EndorseMod(const ModRepositoryRequest& request, ModEndorsement state)
	{
		auto connection = NewCURLSession(KxString::Format("%1/games/%2/mods/%3/%4",
										 GetAPIURL(),
										 TranslateGameIDToNetwork(request),
										 request.GetModID().GetValue(),
										 EndorsementStateToString(state))
		);

		KxVersion modVersion;
		if (request.GetExtraInfo(modVersion))
		{
			connection->SetPostData(KxJSON::Save(KxJSONObject {{"Version", modVersion.ToString()}}));
		}
		else
		{
			connection->SetPostData(KxJSON::Save(KxJSONObject {{"Version", "x"}}));
		}

		KxCURLReply reply = connection->Send();
		if (TestRequestError(reply, reply))
		{
			return std::nullopt;
		}

		ModEndorsementReply info;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			info.Message = json["message"].get<wxString>();

			if (auto statusIt = json.find("status"); statusIt != json.end())
			{
				if (*statusIt == "Endorsed")
				{
					info.Endorsement = ModEndorsement::Endorsed();
				}
				else if (*statusIt == "Abstained")
				{
					info.Endorsement = ModEndorsement::Abstained();
				}
				else
				{
					info.Endorsement = ModEndorsement::Undecided();
				}
			}
		}
		catch (...)
		{
			ReportRequestError(reply);
			return std::nullopt;
		}
		return info;
	}
	
	std::optional<ModFileReply> NexusModNetwork::GetModFileInfo(const ModRepositoryRequest& request) const
	{
		auto connection = NewCURLSession(KxString::Format("%1/games/%2/mods/%3/files/%4",
										 GetAPIURL(),
										 TranslateGameIDToNetwork(request),
										 request.GetModID().GetValue(),
										 request.GetFileID().GetValue())
		);
		KxCURLReply reply = connection->Send();
		if (TestRequestError(reply, reply))
		{
			return std::nullopt;
		}

		ModFileReply info;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);

			info.ModID = request.GetModID();
			ReadFileInfo(json, info);
		}
		catch (...)
		{
			ReportRequestError(reply);
			return std::nullopt;
		}
		return info;
	}
	std::vector<ModFileReply> NexusModNetwork::GetModFiles(const ModRepositoryRequest& request) const
	{
		auto connection = NewCURLSession(KxString::Format("%1/games/%2/mods/%3/files",
										 GetAPIURL(),
										 TranslateGameIDToNetwork(request),
										 request.GetModID().GetValue())
		);
		KxCURLReply reply = connection->Send();
		if (TestRequestError(reply, reply))
		{
			return {};
		}

		std::vector<ModFileReply> infoVector;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			infoVector.reserve(json.size());

			for (const KxJSONObject& value: json["files"])
			{
				ModFileReply& info = infoVector.emplace_back();
				info.ModID = request.GetModID();
				ReadFileInfo(value, info);
			}
		}
		catch (...)
		{
			infoVector.clear();
		}
		return infoVector;
	}
	std::vector<ModDownloadReply> NexusModNetwork::GetFileDownloads(const ModRepositoryRequest& request) const
	{
		wxString query = KxString::Format("%1/games/%2/mods/%3/files/%4/download_link",
										  GetAPIURL(),
										  TranslateGameIDToNetwork(request),
										  request.GetModID().GetValue(),
										  request.GetFileID().GetValue()
		);

		NexusNXMLinkData nxmExtraInfo;
		if (request.GetExtraInfo(nxmExtraInfo))
		{
			query += KxString::Format("?key=%1&expires=%2", nxmExtraInfo.Key, nxmExtraInfo.Expires);
		}
		auto connection = NewCURLSession(query);

		KxCURLReply reply = connection->Send();
		if (TestRequestError(reply, reply))
		{
			return {};
		}

		std::vector<ModDownloadReply> infoVector;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			infoVector.reserve(json.size());

			for (const KxJSONObject& value: json)
			{
				ModDownloadReply& info = infoVector.emplace_back();
				info.Name = value["name"].get<wxString>();
				info.ShortName = value["short_name"].get<wxString>();
				info.URL = ConvertUnicodeEscapes(value["URI"].get<wxString>());
			}
		}
		catch (...)
		{
			infoVector.clear();
		}
		return infoVector;
	}

	std::optional<NexusValidationReply> NexusModNetwork::DoGetValidationInfo(const wxString& apiKey, bool noErrorReport) const
	{
		auto connection = NewCURLSession(KxString::Format("%1/users/validate",
										 GetAPIURL()),
										 apiKey
		);
		KxCURLReply reply = connection->Send();
		if (TestRequestError(reply, reply, noErrorReport))
		{
			return std::nullopt;
		}

		NexusValidationReply info;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);

			info.UserID = json["user_id"];
			info.UserName = json["name"].get<wxString>();
			info.APIKey = json["key"].get<wxString>();
			info.EMail = json["email"].get<wxString>();
			info.ProfilePicture = json["profile_url"].get<wxString>();
			info.IsPremium = json["is_premium"];
			info.IsSupporter = json["is_supporter"];
		}
		catch (...)
		{
			return std::nullopt;
		}
		return info;
	}

	std::optional<NexusValidationReply> NexusModNetwork::GetValidationInfo() const
	{
		return DoGetValidationInfo({}, false);
	}
	std::optional<NexusGameReply> NexusModNetwork::GetGameInfo(const GameID& id) const
	{
		auto connection = NewCURLSession(KxString::Format("%1/games/%2",
										 GetAPIURL(), 
										 TranslateGameIDToNetwork(id))
		);
		KxCURLReply reply = connection->Send();
		if (TestRequestError(reply, reply))
		{
			return std::nullopt;
		}

		NexusGameReply info;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			ReadGameInfo(json, info);
		}
		catch (...)
		{
			return std::nullopt;
		}
		return info;
	}
	std::vector<NexusGameReply> NexusModNetwork::GetGamesList() const
	{
		auto connection = NewCURLSession(KxString::Format("%1/games",
										 GetAPIURL())
		);
		KxCURLReply reply = connection->Send();
		if (TestRequestError(reply, reply))
		{
			return {};
		}

		std::vector<NexusGameReply> infoVector;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			infoVector.reserve(json.size());

			for (const KxJSONObject& value: json)
			{
				NexusGameReply& info = infoVector.emplace_back();
				ReadGameInfo(value, info);
			}
		}
		catch (...)
		{
			infoVector.clear();
		}
		return infoVector;
	}

	wxString NexusModNetwork::ConstructNXM(const NetworkModInfo& modInfo, const GameID& id, const NexusNXMLinkData& linkData) const
	{
		wxString nxm = KxString::Format(wxS("nxm://%1/mods/%2/files/%3"),
										TranslateGameIDToNetwork(id),
										modInfo.GetModID().GetValue(),
										modInfo.GetFileID().GetValue()
		);
		if (!linkData.IsEmpty())
		{
			nxm += KxString::Format(wxS("?key=%1&expires=%2&user_id=%3"), linkData.Key, linkData.Expires, linkData.UserID);
		}

		nxm.MakeLower();
		return nxm;
	}
	bool NexusModNetwork::ParseNXM(const wxString& link, GameID& gameID, NetworkModInfo& modInfo, NexusNXMLinkData& linkData)
	{
		wxRegEx reg(u8R"(nxm:\/\/(\w+)\/mods\/(\d+)\/files\/(\d+)\?key=(.+)&expires=(.+)&user_id=(.+))", wxRE_ADVANCED|wxRE_ICASE);
		if (reg.Matches(link))
		{
			gameID = TranslateGameIDFromNetwork(reg.GetMatch(link, 1));

			ModID modID(reg.GetMatch(link, 2));
			ModFileID fileID(reg.GetMatch(link, 3));
			modInfo = NetworkModInfo(modID, fileID);

			linkData.Key = reg.GetMatch(link, 4);
			linkData.Expires = reg.GetMatch(link, 5);
			linkData.UserID = reg.GetMatch(link, 6);

			return gameID && modID&& fileID;
		}
		return false;
	}
}
