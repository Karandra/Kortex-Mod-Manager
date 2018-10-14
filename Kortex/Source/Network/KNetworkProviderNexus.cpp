#include "stdafx.h"
#include "KNetwork.h"
#include "KNetworkProviderNexus.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxWebSockets.h>
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxJSON.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxString.h>

void KNetworkProviderNexus::OnAuthSuccess(wxWindow* window)
{
	KNetwork::GetInstance()->CallAfter([this, window]()
	{
		KNetworkProvider::OnAuthSuccess(window);
	});
}
void KNetworkProviderNexus::OnAuthFail(wxWindow* window)
{
	KNetwork::GetInstance()->CallAfter([this, window]()
	{
		KNetworkProvider::OnAuthFail(window);
	});
}

wxString KNetworkProviderNexus::EndorsementStateToString(EndorsementState::Value state) const
{
	switch (state)
	{
		case EndorsementState::Endorse:
		{
			return "endorse";
		}
		case EndorsementState::Abstain:
		{
			return "abstain";
		}
	};
	return "undecided";
}
KxCURLSession& KNetworkProviderNexus::ConfigureRequest(KxCURLSession& request, const wxString& apiKey) const
{
	request.AddHeader("APIKEY", apiKey.IsEmpty() ? GetAPIKey() : apiKey);
	request.AddHeader("Content-Type", "application/json");
	request.AddHeader("Protocol-Version", "0.15.5");
	request.AddHeader("Application-Version", KApp::Get().GetAppVersion());

	return request;
}
bool KNetworkProviderNexus::ShouldTryLater(const KxCURLReplyBase& reply) const
{
	return reply.GetResponseCode() == 429;
}
wxString KNetworkProviderNexus::GetAPIURL() const
{
	return "https://api.nexusmods.com/v1";
}
wxString KNetworkProviderNexus::GetAPIKey(wxString* userName) const
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
void KNetworkProviderNexus::RequestUserAvatar(ValidationInfo& info)
{
	if (!HasUserPicture())
	{
		SetUserPicture(DownloadSmallBitmap(info.GetProfilePictureURL()));
	}
}

wxString& KNetworkProviderNexus::ConvertChangeLog(wxString& changeLog) const
{
	changeLog.Replace(wxS("<br>"), wxS("\r\n"));
	changeLog.Replace(wxS("<br/>"), wxS("\r\n"));
	changeLog.Replace(wxS("<br />"), wxS("\r\n"));
	changeLog.Replace(wxS("</br>"), wxS("\r\n"));

	changeLog.Replace(wxS("\n\r\n"), wxS("\r\n"));
	KxString::Trim(changeLog, true, true);

	return changeLog;
}
wxString& KNetworkProviderNexus::ConvertDisplayName(wxString& name) const
{
	name.Replace(wxS("_"), wxS(" "));
	KxString::Trim(name, true, true);

	return name;
}

bool KNetworkProviderNexus::DoAuthenticate(wxWindow* window)
{
	KxWebSocketClient* client = new KxWebSocketClient("wss://sso.nexusmods.com");

	client->Bind(KxEVT_WEBSOCKET_OPEN, [this, client, window](KxWebSocketEvent& event)
	{
		wxString id = KNetwork::GetInstance()->GetUniqueID();

		KxJSONObject json = {{"id", id}, {"appid", "Kortex"}};
		client->Send(KxJSON::Save(json));
		KxShell::Execute(window, KxShell::GetDefaultViewer("html"), "open", wxString::Format("https://www.nexusmods.com/sso?id=%s", id));
	});
	client->Bind(KxEVT_WEBSOCKET_MESSAGE, [this, client, window](KxWebSocketEvent& event)
	{
		wxString apiKey = event.GetTextMessage();
		client->Close();

		ValidationInfo info = GetValidationInfo(apiKey);
		if (info.IsOK() && info.GetAPIKey() == apiKey)
		{
			if (SaveAuthInfo(info.GetUserName(), apiKey))
			{
				RequestUserAvatar(info);
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

		KNetwork::GetInstance()->OnAuthStateChanged();
		delete client;
	});
	client->Bind(KxEVT_WEBSOCKET_FAIL, [this, client, window](KxWebSocketEvent& event)
	{
		OnAuthFail(window);
	});
	return true;
}
bool KNetworkProviderNexus::DoValidateAuth(wxWindow* window)
{
	ValidationInfo info = GetValidationInfo();
	RequestUserAvatar(info);
	return info.IsOK() && info.GetAPIKey() == GetAPIKey();
}
bool KNetworkProviderNexus::DoSignOut(wxWindow* window)
{
	return KNetworkProvider::SignOut(window);
}
bool KNetworkProviderNexus::DoIsAuthenticated() const
{
	return KNetworkProvider::DoIsAuthenticated();
}

KNetworkProviderNexus::KNetworkProviderNexus(KNetworkProviderID providerID)
	:KNetworkProvider(providerID, wxS("Nexus"))
{
}

KImageEnum KNetworkProviderNexus::GetIcon() const
{
	return KIMG_SITE_NEXUS;
}
wxString KNetworkProviderNexus::GetName() const
{
	return "Nexus";
}
wxString KNetworkProviderNexus::GetGameID(const KGameID& id) const
{
	// If invalid profile is passed, return ID for current profile.
	if (id.IsOK())
	{
		// TES
		if (id == KGameIDs::Morrowind)
		{
			return "Morrowind";
		}
		if (id == KGameIDs::Oblivion)
		{
			return "Oblivion";
		}
		if (id == KGameIDs::Skyrim)
		{
			return "Skyrim";
		}
		if (id == KGameIDs::SkyrimSE)
		{
			return "SkyrimSpecialEdition";
		}

		// Fallout
		if (id == KGameIDs::Fallout3)
		{
			return "Fallout3";
		}
		if (id == KGameIDs::FalloutNV)
		{
			return "NewVegas";
		}
		if (id == KGameIDs::Fallout4)
		{
			return "Fallout4";
		}
	}
	return KNetworkProvider::GetGameID(id);
}
wxString& KNetworkProviderNexus::ConvertDescriptionToHTML(wxString& description) const
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
		wxRegEx tRegEx(RAW(u8R"(\[url=([^\]]+)\]([^\[]+)\[\/url\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.ReplaceAll(&description, RAW(u8R"(<a href="\1">\2</a>)"));
	}

	// URL: [url]address[/url]
	{
		wxRegEx tRegEx(RAW(u8R"(\[url\]([^\[]+)\[\/url\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.ReplaceAll(&description, RAW(u8R"(<a href="\1">\1</a>)"));
	}

	// URL: [NEXUS ID: number]
	{
		wxRegEx tRegEx(RAW(u8R"(\[NEXUS\s?ID\:\s?(\d+)\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.ReplaceAll(&description, wxString::Format(RAW(u8R"(<a href="%s\/\1">\0</a>)"), GetModURLBasePart()));
	}

	// Image: [img]address[/img]
	{
		wxRegEx tRegEx(RAW(u8R"(\[img\]([^\[]+)\[\/img\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.ReplaceAll(&description, RAW(u8R"(<img src="\1"/>)"));
	}

	// Align: [left|right|center|justify]text[/left|right|center|justify]
	{
		wxRegEx tRegEx(RAW(u8R"(\[(left|right|center|justify)\](.*)\[\/(?:left|right|center|justify)\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.ReplaceAll(&description, RAW(u8R"(<div align="\1">\2</div>)"));
	}

	// Font size: [size=string|number]text[/size]
	{
		wxRegEx tRegEx(RAW(u8R"((?s)\[size=\\?"?(\d+)\\?"?\](.*?)\[\/size\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.ReplaceAll(&description, RAW(u8R"(<font size="\1">\2</font>)"));
	}

	// Font color: [color="name"]text[/color]
	{
		wxRegEx tRegEx(RAW(u8R"((?s)\[color=\\?"?(\w+|\#\d+)\\?"?\](.*?)\[\/color\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.ReplaceAll(&description, RAW(u8R"(<font color="\1">\2</font>)"));
	}

	// Font color: [color=#AABBCCDD]text[/color]
	{
		wxRegEx tRegEx(RAW(u8R"(\[color=#([ABCDEF\d]+)\](.+)\[\/color\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.ReplaceAll(&description, RAW(u8R"(<font color="#\1">\2</font>)"));
	}

	// Font color: [color=#AABBCCDD]text[/color]
	{
		wxRegEx tRegEx(RAW(u8R"((?s)\[list\](.*?)\[\/list\])"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		tRegEx.ReplaceAll(&description, RAW(u8R"(<ul>\1</ul>)"));
	}

	// Simple container tags: [b]text[/b]
	auto ExpSimple = [&description, &RAW](const wxString& tagName, const wxString& sTagNameRepl = wxEmptyString)
	{
		wxString regEx = wxString::Format(RAW(u8R"((?s)\[%s\](.*?)\[\/%s\])"), tagName, tagName);
		wxString sRepl = wxString::Format(RAW(u8R"(<%s>\1</%s>)"), sTagNameRepl, sTagNameRepl);

		wxRegEx tRegEx(regEx, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		if (tRegEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE))
		{
			tRegEx.ReplaceAll(&description, sRepl);
		}
	};

	ExpSimple("b", "b");
	ExpSimple("i", "i");
	ExpSimple("u", "u");
	ExpSimple("\\*", "li");

	// Buggy
	#if 0
	// URL. Matches string starting only with 'http://', 'https://', 'ftp://' or 'www.'.
	// This tries not to match addresses inside <a></a> tags. And this better to be the last replacement.
	{
		wxRegEx tRegEx(RAW(u8R"((?:[^<a href="">])(http:\/\/|https:\/\/\ftp:\/\/|www.)([\w_-]+(?:(?:\.[\w_-]+)+))([\w.,@?^=%&:\/~+#-]*[\w@?^=%&\/~+#-])?(?:[^<\/a>]))"), wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
		if (tRegEx.Matches(description, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE))
		{
			tRegEx.ReplaceAll(&description, RAW(u8R"(<a href="\1\2\3">\0</a>)"));
		}
	}
	#endif

	return description;
}
wxString KNetworkProviderNexus::GetModURLBasePart(const KGameID& id) const
{
	return wxString::Format("https://www.nexusmods.com/%s/mods", GetGameID(id)).MakeLower();
}
wxString KNetworkProviderNexus::GetModURL(int64_t modID, const wxString& modSignature, const KGameID& id)
{
	return wxString::Format("%s/%lld", GetModURLBasePart(id), modID);
}

KNetworkProvider::ModInfo KNetworkProviderNexus::GetModInfo(int64_t modID, const KGameID& id) const
{
	KxCURLSession connection(wxString::Format("%s/games/%s/mods/%lld", GetAPIURL(), GetGameID(id), modID));
	KxCURLReply reply = ConfigureRequest(connection).Send();

	ModInfo info;
	if (ShouldTryLater(reply))
	{
		info.SetShouldTryLater();
		return info;
	}

	try
	{
		KxJSONObject json = KxJSON::Load(reply);

		info.m_ID = modID;
		info.m_Name = json["name"].get<wxString>();
		info.m_Summary = json["summary"].get<wxString>();
		info.m_Description = json["description"].get<wxString>();
		info.m_Author = json["author"].get<wxString>();
		info.m_Uploader = json["uploaded_by"].get<wxString>();
		info.m_UploaderProfileURL = json["uploaded_users_profile_url"].get<wxString>();
		info.m_MainImageURL = json["picture_url"].get<wxString>();
		
		info.m_Version = json["version"].get<wxString>();
		info.m_UploadDate = ReadDateTime(json["created_time"]);
		info.m_LastUpdateDate = ReadDateTime(json["updated_time"]);

		info.m_ContainsAdultContent = json["contains_adult_content?"];

		// Primary file
		auto primaryFile = json.find("primary_file");
		if (primaryFile != json.end())
		{
			ReadFileInfo(*primaryFile, info.m_PrimaryFile);
		}

		// Endorsement state
		auto endorsementState = json.find("endorsement");
		if (endorsementState != json.end())
		{
			if (*endorsementState == "Endorse")
			{
				info.SetEndorsed();
			}
			else if (*endorsementState == "Abstain")
			{
				info.SetAbstained();
			}
			else
			{
				info.SetUndecided();
			}
		}
	}
	catch (...)
	{
		info.Reset();
	}
	return info;
}
KNetworkProvider::FileInfo KNetworkProviderNexus::GetFileInfo(int64_t modID, int64_t fileID, const KGameID& id) const
{
	KxCURLSession connection(wxString::Format("%s/games/%s/mods/%lld/files/%lld", GetAPIURL(), GetGameID(id), modID, fileID));
	KxCURLReply reply = ConfigureRequest(connection).Send();

	FileInfo info;
	if (ShouldTryLater(reply))
	{
		info.SetShouldTryLater();
		return info;
	}

	try
	{
		KxJSONObject json = KxJSON::Load(reply);
		
		info.SetModID(modID);
		ReadFileInfo(json, info);
	}
	catch (...)
	{
		info.Reset();
	}
	return info;
}
KNetworkProvider::FileInfo::Vector KNetworkProviderNexus::GetFilesList(int64_t modID, const KGameID& id) const
{
	KxCURLSession connection(wxString::Format("%s/games/%s/mods/%lld/files", GetAPIURL(), GetGameID(id), modID));
	KxCURLReply reply = ConfigureRequest(connection).Send();

	FileInfo::Vector infoVector;
	if (ShouldTryLater(reply))
	{
		infoVector.emplace_back().SetShouldTryLater();
		return infoVector;
	}

	try
	{
		KxJSONObject json = KxJSON::Load(reply);
		infoVector.reserve(json.size());

		for (const KxJSONObject& value: json["files"])
		{
			FileInfo& info = infoVector.emplace_back();
			info.SetModID(modID);
			ReadFileInfo(value, info);
		}
	}
	catch (...)
	{
		infoVector.clear();
	}
	return infoVector;
}
KNetworkProvider::DownloadInfo::Vector KNetworkProviderNexus::GetFileDownloadLinks(int64_t modID, int64_t fileID, const KGameID& id) const
{
	KxCURLSession connection(wxString::Format("%s/games/%s/mods/%lld/files/%lld/download_link", GetAPIURL(), GetGameID(id), modID, fileID));
	KxCURLReply reply = ConfigureRequest(connection).Send();

	DownloadInfo::Vector infoVector;
	if (ShouldTryLater(reply))
	{
		infoVector.emplace_back().SetShouldTryLater();
		return infoVector;
	}

	try
	{
		KxJSONObject json = KxJSON::Load(reply);
		infoVector.reserve(json.size());

		for (const KxJSONObject& value: json)
		{
			DownloadInfo& info = infoVector.emplace_back();

			info.m_Name = value["name"].get<wxString>();
			info.m_ShortName = value["short_name"].get<wxString>();
			info.m_URL = value["URI"].get<wxString>();
		}
	}
	catch (...)
	{
		infoVector.clear();
	}
	return infoVector;
}
KNetworkProvider::EndorsedInfo KNetworkProviderNexus::EndorseMod(int64_t modID, EndorsementState::Value state, const KGameID& id)
{
	KxCURLSession connection(wxString::Format("%s/games/%s/mods/%lld/%s", GetAPIURL(), GetGameID(id), modID, EndorsementStateToString(state)));
	
	// I don't know why this request needs mod version, it's works even with fake version.
	KxJSONObject data = {{"Version", "x"}};
	connection.SetPostData(KxJSON::Save(data));

	KxCURLReply reply = ConfigureRequest(connection).Send();
	
	EndorsedInfo info;
	if (ShouldTryLater(reply))
	{
		info.SetShouldTryLater();
		return info;
	}
	
	try
	{
		KxJSONObject json = KxJSON::Load(reply);

		info.m_Message = json["message"].get<wxString>();

		auto status = json.find("status");
		if (status != json.end())
		{
			if (*status == "Endorsed")
			{
				info.SetEndorsed();
			}
			else if (*status == "Abstained")
			{
				info.SetAbstained();
			}
		}
	}
	catch (...)
	{
		info.Reset();
	}
	return info;
}

KNetworkProviderNexus::ValidationInfo KNetworkProviderNexus::GetValidationInfo(const wxString& apiKey) const
{
	KxCURLSession connection(GetAPIURL() + "/users/validate");
	KxCURLReply reply = ConfigureRequest(connection, apiKey).Send();

	ValidationInfo info;
	if (ShouldTryLater(reply))
	{
		info.SetShouldTryLater();
		return info;
	}

	try
	{
		KxJSONObject json = KxJSON::Load(reply);

		info.m_UserName = json["name"].get<wxString>();
		info.m_APIKey = json["key"].get<wxString>();
		info.m_EMail = json["email"].get<wxString>();
		info.m_ProfilePictureURL = json["profile_url"].get<wxString>();
		info.m_UserID = json["user_id"];
		info.m_IsPremium = json["is_premium?"];
		info.m_IsSupporter = json["is_supporter?"];
	}
	catch (...)
	{
		info.Reset();
	}
	return info;
}
KNetworkProviderNexus::GameInfo KNetworkProviderNexus::GetGameInfo(const KGameID& id) const
{
	KxCURLSession connection(GetAPIURL() + "/games/" + GetGameID(id));
	KxCURLReply reply = ConfigureRequest(connection).Send();

	GameInfo info;
	if (ShouldTryLater(reply))
	{
		info.SetShouldTryLater();
		return info;
	}

	try
	{
		KxJSONObject json = KxJSON::Load(reply);
		ReadGameInfo(json, info);
	}
	catch (...)
	{
		info.Reset();
	}
	return info;
}
KNetworkProviderNexus::GameInfo::Vector KNetworkProviderNexus::GetGamesList() const
{
	KxCURLSession connection(GetAPIURL() + "/games");
	KxCURLReply reply = ConfigureRequest(connection).Send();

	GameInfo::Vector infoVector;
	if (ShouldTryLater(reply))
	{
		infoVector.emplace_back().SetShouldTryLater();
		return infoVector;
	}

	try
	{
		KxJSONObject json = KxJSON::Load(reply);
		infoVector.reserve(json.size());

		for (const KxJSONObject& value: json)
		{
			GameInfo& info = infoVector.emplace_back();
			ReadGameInfo(value, info);
		}
	}
	catch (...)
	{
		infoVector.clear();
	}
	return infoVector;
}
KNetworkProviderNexus::IssueInfo::Vector KNetworkProviderNexus::GetIssues() const
{
	KxCURLSession connection(wxString::Format("%s/feedbacks/list_user_issues/", GetAPIURL()));
	KxCURLReply reply = ConfigureRequest(connection).Send();

	IssueInfo::Vector infoVector;
	if (ShouldTryLater(reply))
	{
		infoVector.emplace_back().SetShouldTryLater();
		return infoVector;
	}

	try
	{
		KxJSONObject json = KxJSON::Load(reply);
		infoVector.reserve(json.size());

		for (const KxJSONObject& value: json["issues"])
		{
			IssueInfo& info = infoVector.emplace_back();

			//info.m_Name = value["name"].get<wxString>();
		}
	}
	catch (...)
	{
		infoVector.clear();
	}
	return infoVector;
}

wxString KNetworkProviderNexus::ConstructNXM(const FileInfo& fileInfo, const KGameID& id) const
{
	return wxString::Format("nxm://%s/mods/%lld/files/%lld", GetGameID(id), fileInfo.GetModID(), fileInfo.GetID()).Lower();
}
