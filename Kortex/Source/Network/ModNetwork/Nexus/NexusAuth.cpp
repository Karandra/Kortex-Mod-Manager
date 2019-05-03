#include "stdafx.h"
#include "NexusAuth.h"
#include "Nexus.h"
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Application.hpp>
#include <Kortex/Events.hpp>
#include "UI/KMainWindow.h"
#include "Utility/String.h"
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxJSON.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxWebSocket.h>

namespace Kortex::NetworkManager
{
	wxWindow* NexusAuth::GetInvokingWindow() const
	{
		return KMainWindow::GetInstance();
	}
	void NexusAuth::OnAuthSuccess()
	{
		m_WebSocketClient.reset();
		ModNetworkAuth::OnAuthSuccess();
	}
	void NexusAuth::OnAuthFail()
	{
		ResetSessionInfo();
		ModNetworkAuth::OnAuthFail();
	}
	void NexusAuth::OnAuthReset()
	{
		ResetSessionInfo();
		return ModNetworkAuth::OnAuthReset();
	}
	void NexusAuth::ResetSessionInfo()
	{
		m_WebSocketClient.reset();
		m_UserToken.clear();
		m_SessionGUID = {};
		m_LastValidationReply.reset();
	}
	
	void NexusAuth::OnToolBarMenu(KxMenu& menu)
	{
		if (m_LastValidationReply)
		{
			if (m_LastValidationReply->IsPremium)
			{
				KxMenuItem* item = menu.AddItem(KTr("NetworkManager.Nexus.UserIsPremium"), wxEmptyString, wxITEM_CHECK);
				item->Enable(false);
				item->Check();
			}
			else if (m_LastValidationReply->IsSupporter)
			{
				KxMenuItem* item = menu.AddItem(KTr("NetworkManager.Nexus.UserIsSupporter"), wxEmptyString, wxITEM_CHECK);
				item->Enable(false);
				item->Check();
			}
		}
	}
	void NexusAuth::RequestUserPicture(const NexusValidationReply& info)
	{
		if (!HasUserPicture() && !LoadUserPicture())
		{
			SetUserPicture(DownloadSmallBitmap(info.ProfilePicture));
		}
	}
	std::optional<NexusValidationReply> NexusAuth::DoGetValidationInfo(const wxString& apiKey, bool noErrorReport)
	{
		auto connection = m_Nexus.NewCURLSession(KxString::Format(wxS("%1/users/validate"), m_Nexus.GetAPIURL()), apiKey);
		KxCURLReply reply = connection->Send();
		if (!m_Utility.TestRequestError(reply, reply, noErrorReport).IsSuccessful())
		{
			const bool stateChanged = m_LastValidationReply.has_value();
			m_LastValidationReply.reset();

			// If we were authenticated notify network manager that auth state was lost
			if (stateChanged)
			{
				OnAuthReset();
			}
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
			m_LastValidationReply.reset();
			return std::nullopt;
		}

		m_LastValidationReply = info;
		return info;
	}

	NexusAuth::NexusAuth(NexusModNetwork& nexus, NexusUtility& utility)
		:m_Nexus(nexus), m_Utility(utility), m_CredentialsStore(wxS("Kortex/NexusMods"))
	{
	}

	bool NexusAuth::IsAuthenticated() const
	{
		return m_LastValidationReply.has_value();
	}
	void NexusAuth::Authenticate()
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
								RequestUserPicture(*info);
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

		m_WebSocketClient->Connect();
	}
	void NexusAuth::ValidateAuth()
	{
		// Load data from credentials store
		if (auto credentials = LoadCredentials())
		{
			// Get API Key from there
			const wxString apiKey = credentials->Password.GetAsString();
			if (!apiKey.IsEmpty())
			{
				// If succeed compare it with key that Nexus returns
				auto info = DoGetValidationInfo(apiKey, true);
				if (info && info->APIKey == apiKey)
				{
					RequestUserPicture(*info);
					return;
				}
			}
		}

		SetUserPicture(wxNullBitmap);
		m_LastValidationReply.reset();
	}
	void NexusAuth::SignOut()
	{
		m_LastValidationReply.reset();
		m_CredentialsStore.Delete();
	}

	std::optional<NexusValidationReply> NexusAuth::GetValidationInfo()
	{
		return DoGetValidationInfo({}, false);
	}
}
