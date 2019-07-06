#include "stdafx.h"
#include "Nexus.h"
#include <Kortex/NetworkManager.hpp>
#include <Kortex/GameInstance.hpp>
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
#include <KxFramework/KxCoroutine.h>

namespace Kortex::Variables
{
	constexpr auto NexusDomainName = wxS("NexusDomainName");
}

namespace Kortex::NetworkManager
{
	void NexusModNetwork::OnAuthenticated()
	{
		if (m_UpdateChecker.CanIssueNewAutomaticCheck())
		{
			KxCoroutine::Run([this, wait = true](KxCoroutine& coroutine) mutable
			{
				if (wait)
				{
					wait = false;
					return coroutine.YieldWait(wxTimeSpan::Seconds(3));
				}
				else
				{
					m_UpdateChecker.DoRunUpdateCheck();
					return coroutine.YieldStop();
				}
			});
		}
	}

	std::unique_ptr<KxCURLSession> NexusModNetwork::NewCURLSession(const wxString& address, const wxString& apiKey) const
	{
		auto session = INetworkManager::GetInstance()->NewCURLSession(address);

		// Add Nexus specific headers
		session->AddHeader(wxS("APIKey"), apiKey.IsEmpty() ? GetAPIKey() : apiKey);
		session->AddHeader(wxS("Content-Type"), wxS("application/json"));
		session->AddHeader(wxS("Protocol-Version"), wxS("0.15.5"));

		// Allow repository component to read query limits info from response headers
		session->Bind(KxEVT_CURL_RESPONSE_HEADER, &NexusRepository::OnResponseHeader, const_cast<NexusRepository*>(&m_Repository));

		return session;
	}
	wxString NexusModNetwork::GetAPIURL() const
	{
		return wxS("https://api.nexusmods.com/v1");
	}
	wxString NexusModNetwork::GetAPIKey() const
	{
		if (const NexusValidationReply* info = m_Auth.GetLastValidationReply())
		{
			return info->APIKey;
		}
		return {};
	}
	
	void NexusModNetwork::OnInit()
	{
	}
	void NexusModNetwork::OnExit()
	{
	}
	void NexusModNetwork::OnLoadInstance(IGameInstance& instance, const KxXMLNode& networkNode)
	{
		m_UpdateChecker.OnLoadInstance(instance, networkNode.GetFirstChildElement(wxS("UpdateChecker")));
	}

	NexusModNetwork::NexusModNetwork()
		:m_Utility(*this),
		m_Auth(*this, m_Utility),
		m_Repository(*this, m_Utility, m_Auth),
		m_UpdateChecker(*this, m_Utility, m_Repository)
	{
		AddComponent(m_Utility);
		AddComponent<ModNetworkAuth>(m_Auth);
		AddComponent<ModNetworkRepository>(m_Repository);
		AddComponent<ModNetworkUpdateChecker>(m_UpdateChecker);
	}

	ResourceID NexusModNetwork::GetIcon() const
	{
		return ImageResourceID::ModNetwork_Nexus;
	}
	wxString NexusModNetwork::GetName() const
	{
		return wxS("NexusMods");
	}
	
	wxString NexusModNetwork::TranslateGameIDToNetwork(const GameID& id) const
	{
		if (id.IsOK())
		{
			for (const auto& instance: IGameInstance::GetShallowInstances())
			{
				if (instance->GetGameID() == id)
				{
					return instance->GetVariables().GetVariable(Variables::NexusDomainName);
				}
			}
			return {};
		}
		else
		{
			// If invalid ID is passed, return ID for current game
			return IGameInstance::GetActive()->GetVariables().GetVariable(Variables::NexusDomainName);
		}
	}
	GameID NexusModNetwork::TranslateGameIDFromNetwork(const wxString& id) const
	{
		if (!id.IsEmpty())
		{
			for (const auto& instance: IGameInstance::GetShallowInstances())
			{
				if (instance->GetVariables().GetVariable(Variables::NexusDomainName) == id)
				{
					return instance->GetGameID();
				}
			}
		}
		return {};
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

	std::optional<NexusGameReply> NexusModNetwork::GetGameInfo(const GameID& id) const
	{
		auto connection = NewCURLSession(KxString::Format("%1/games/%2",
										 GetAPIURL(), 
										 TranslateGameIDToNetwork(id))
		);
		KxCURLReply reply = connection->Send();
		if (m_Utility.TestRequestError(reply, reply))
		{
			return std::nullopt;
		}

		NexusGameReply info;
		try
		{
			KxJSONObject json = KxJSON::Load(reply);
			m_Utility.ReadGameInfo(json, info);
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
		if (m_Utility.TestRequestError(reply, reply))
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
				m_Utility.ReadGameInfo(value, info);
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
	bool NexusModNetwork::ParseNXM(const wxString& link, GameID& gameID, NetworkModInfo& modInfo, NexusNXMLinkData& linkData) const
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

			return gameID && modID && fileID;
		}
		return false;
	}

	void NexusModNetwork::OnToolBarMenu(KxMenu& menu)
	{
		m_Auth.OnToolBarMenu(menu);
	}
}
