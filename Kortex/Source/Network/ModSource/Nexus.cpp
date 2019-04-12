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
	void ReadGameInfo(const TJsonValue& json, NetworkManager::NexusGameReply& info)
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
	bool TestRequestError(const KxCURLReplyBase& reply, const wxString& message = {})
	{
		if (reply.GetResponseCode() == KxHTTPStatusCode::TooManyRequests)
		{
			INotificationCenter::GetInstance()->NotifyUsing<INetworkManager>(KTrf("Network.RequestQuotaReched", NexusSource::GetInstance()->GetName()), KxICON_WARNING);
			return true;
		}
		else if (!reply.IsOK())
		{
			ReportRequestError(message);
			return true;
		}
		return false;
	}
}

namespace Kortex::NetworkManager
{
	wxWindow* NexusSource::GetInvokingWindow() const
	{
		return KMainWindow::GetInstance();
	}
	KxStandardID NexusSource::OnAuthSuccess()
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
	KxStandardID NexusSource::OnAuthFail()
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

	wxString NexusSource::EndorsementStateToString(const ModEndorsement& state) const
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
	KxCURLSession& NexusSource::ConfigureRequest(KxCURLSession& request, const wxString& apiKey) const
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

	wxString NexusSource::GetAPIURL() const
	{
		return "https://api.nexusmods.com/v1";
	}
	wxString NexusSource::GetAPIKey(wxString* userName) const
	{
		if (auto credentials = LoadCredentials())
		{
			KxUtility::SetIfNotNull(userName, credentials->UserID);
			return credentials->Password.GetAsString();
		}
		return {};
	}
	void NexusSource::RequestUserAvatar(const NexusValidationReply& info)
	{
		if (!HasUserPicture())
		{
			SetUserPicture(DownloadSmallBitmap(info.ProfilePicture));
		}
	}

	NexusSource::NexusSource()
		:m_CredentialsStore(wxS("Kortex/NexusMods")), m_UserAgent(GetUserAgent(NetworkSoftware::CURL))
	{
	}

	bool NexusSource::Authenticate()
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
			m_WebSocketClient->Send(KxJSON::Save(json));

			const wxString openURL = KxString::Format("https://www.nexusmods.com/sso?id=%1&application=%2", guid, appID);
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
						auto info = GetValidationInfo(apiKey);

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
	bool NexusSource::ValidateAuth()
	{
		if (LoadCredentials())
		{
			if (auto info = GetValidationInfo())
			{
				RequestUserAvatar(*info);

				m_IsAuthenticated = info->APIKey == GetAPIKey();
				return m_IsAuthenticated;
			}
		}

		m_IsAuthenticated = false;
		return m_IsAuthenticated;
	}
	bool NexusSource::SignOut()
	{
		m_IsAuthenticated = false;
		return m_CredentialsStore.Delete();
	}
	bool NexusSource::IsAuthenticated() const
	{
		return m_IsAuthenticated;
	}

	KImageEnum NexusSource::GetIcon() const
	{
		return KIMG_SITE_NEXUS;
	}
	wxString NexusSource::GetName() const
	{
		return "Nexus";
	}
	wxString NexusSource::GetGameID(const GameID& id) const
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
	wxString& NexusSource::ConvertDescriptionToHTML(wxString& description) const
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
	wxString NexusSource::GetModURLBasePart(const GameID& id) const
	{
		return wxString::Format("https://www.nexusmods.com/%s/mods", GetGameID(id)).MakeLower();
	}
	wxString NexusSource::GetModURL(const ModRepositoryRequest& request)
	{
		return KxString::Format("%1/%2", GetModURLBasePart(request.GetGameID()), request.GetModID().GetValue());
	}

	bool NexusSource::RestoreBrokenDownload(const KxFileItem& fileItem, IDownloadEntry& download)
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

	std::optional<ModInfoReply> NexusSource::GetModInfo(const ModRepositoryRequest& request) const
	{
		KxCURLSession connection(KxString::Format("%1/games/%2/mods/%3",
												  GetAPIURL(),
												  GetGameID(request.GetGameID()),
												  request.GetModID().GetValue())
		);
		KxCURLReply reply = ConfigureRequest(connection).Send();
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
	std::optional<ModEndorsementReply> NexusSource::EndorseMod(const ModRepositoryRequest& request, ModEndorsement state)
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
	
	std::optional<ModFileReply> NexusSource::GetModFileInfo(const ModRepositoryRequest& request) const
	{
		KxCURLSession connection(KxString::Format("%1/games/%2/mods/%3/files/%4",
												  GetAPIURL(),
												  GetGameID(request),
												  request.GetModID().GetValue(),
												  request.GetFileID().GetValue())
		);
		KxCURLReply reply = ConfigureRequest(connection).Send();
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
	std::vector<ModFileReply> NexusSource::GetModFiles(const ModRepositoryRequest& request) const
	{
		KxCURLSession connection(KxString::Format("%1/games/%2/mods/%3/files",
												  GetAPIURL(),
												  GetGameID(request),
												  request.GetModID().GetValue())
		);
		KxCURLReply reply = ConfigureRequest(connection).Send();
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
	std::vector<ModDownloadReply> NexusSource::GetFileDownloads(const ModRepositoryRequest& request) const
	{
		wxString query = KxString::Format("%1/games/%2/mods/%3/files/%4/download_link",
										  GetAPIURL(),
										  GetGameID(request),
										  request.GetModID().GetValue(),
										  request.GetFileID().GetValue()
		);

		NexusDownloadExtraReply nxmExtraInfo;
		if (request.GetExtraInfo(nxmExtraInfo))
		{
			query += KxString::Format("?key=%1&expires=%2", nxmExtraInfo.Key, nxmExtraInfo.Expires);
		}
		KxCURLSession connection(query);

		KxCURLReply reply = ConfigureRequest(connection).Send();
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

	std::optional<NexusValidationReply> NexusSource::GetValidationInfo(const wxString& apiKey) const
	{
		KxCURLSession connection(KxString::Format("%1/users/validate", GetAPIURL()));
		KxCURLReply reply = ConfigureRequest(connection, apiKey).Send();
		if (TestRequestError(reply, reply))
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
	std::optional<NexusGameReply> NexusSource::GetGameInfo(const GameID& id) const
	{
		KxCURLSession connection(KxString::Format("%1/games/%2", GetAPIURL(), GetGameID(id)));
		KxCURLReply reply = ConfigureRequest(connection).Send();
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
	std::vector<NexusGameReply> NexusSource::GetGamesList() const
	{
		KxCURLSession connection(KxString::Format("%1/games", GetAPIURL()));
		KxCURLReply reply = ConfigureRequest(connection).Send();
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

	GameID NexusSource::TranslateNxmGameID(const wxString& id) const
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
	wxString NexusSource::ConstructNXM(const ModFileReply& fileInfo, const GameID& id) const
	{
		return KxString::Format("nxm://%1/mods/%2/files/%3", GetGameID(id), fileInfo.ModID.GetValue(), fileInfo.ID.GetValue()).Lower();
	}
}
