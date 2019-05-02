#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/IModNetwork.h"
#include "NexusAuth.h"
#include "NexusRepository.h"
#include "NexusUtility.h"
#include "NexusModInfo.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxUUID.h>
class KxCURLEvent;
class KxCURLSession;
class KxCURLReplyBase;

namespace KxWebSocket
{
	class IClient;
}

namespace Kortex::NetworkManager
{
	class NexusModNetwork: public KxRTTI::IExtendInterface<NexusModNetwork, IModNetwork>, public KxSingletonPtr<NexusModNetwork>
	{
		friend class NexusAuth;
		friend class NexusRepository;

		private:
			NexusUtility m_Utility;
			NexusRepository m_Repository;
			NexusAuth m_Auth;

		private:
			wxString GetAPIURL() const;
			wxString GetAPIKey(wxString* userName = nullptr) const;

			std::unique_ptr<KxCURLSession> NewCURLSession(const wxString& address, const wxString& apiKey = {}) const;
			void OnResponseHeader(KxCURLEvent& event);

		public:
			NexusModNetwork();

		public:
			ResourceID GetIcon() const override;
			wxString GetName() const override;

			wxString TranslateGameIDToNetwork(const GameID& id = {}) const override;
			wxString TranslateGameIDToNetwork(const ModRepositoryRequest& request) const
			{
				return TranslateGameIDToNetwork(request.GetGameID());
			}
			GameID TranslateGameIDFromNetwork(const wxString& id) const override;
			void ConvertDescriptionText(wxString& description) const override;

			wxString GetModPageBaseURL(const GameID& id = {}) const override;
			wxString GetModPageURL(const ModRepositoryRequest& request) override;

		public:
			std::optional<NexusGameReply> GetGameInfo(const GameID& id = {}) const;
			std::vector<NexusGameReply> GetGamesList() const;

			wxString ConstructNXM(const NetworkModInfo& modInfo, const GameID& id = {}, const NexusNXMLinkData& linkData = {}) const;
			bool ParseNXM(const wxString& link, GameID& gameID, NetworkModInfo& modInfo, NexusNXMLinkData& linkData) const;
	
		public:
			void OnToolBarMenu(KxMenu& menu) override;
	};
}
