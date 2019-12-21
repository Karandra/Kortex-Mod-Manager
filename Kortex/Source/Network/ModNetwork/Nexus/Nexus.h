#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/IModNetwork.h"
#include "NexusUpdateChecker.h"
#include "NexusRepository.h"
#include "NexusUtility.h"
#include "NexusAuth.h"
#include "NexusNetworkReply.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxUUID.h>
class KxCURLEvent;
class KxCURLSession;
class KxCURLReplyBase;

namespace Kortex::NetworkManager
{
	class NexusModNetwork:
		public KxRTTI::ExtendInterface<NexusModNetwork, IModNetwork>,
		public KxSingletonPtr<NexusModNetwork>
	{
		KxDecalreIID(NexusModNetwork, {0x8da1cedb, 0x3c16, 0x4ca8, {0x8b, 0x4f, 0xf6, 0x4b, 0x19, 0xf, 0xcb, 0xe3}});

		friend class NexusAuth;
		friend class NexusRepository;
		friend class NexusUpdateChecker;

		private:
			NexusUtility m_Utility;
			NexusRepository m_Repository;
			NexusAuth m_Auth;
			NexusUpdateChecker m_UpdateChecker;

		private:
			void OnAuthenticated();

		protected:
			wxString GetAPIURL() const;
			wxString GetAPIKey() const;
			std::unique_ptr<KxCURLSession> NewCURLSession(const wxString& address, const wxString& apiKey = {}) const;

			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& networkNode) override;

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

			KxURI GetModPageBaseURI(const GameID& id = {}) const override;
			KxURI GetModPageURI(const ModRepositoryRequest& request) const override;

		public:
			std::optional<NexusGameReply> GetGameInfo(const GameID& id = {}) const;
			std::vector<NexusGameReply> GetGamesList() const;
			
		public:
			void OnToolBarMenu(KxMenu& menu) override;
	};
}
