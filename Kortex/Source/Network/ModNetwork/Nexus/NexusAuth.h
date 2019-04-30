#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/IModNetwork.h"
#include "Network/ModNetworkAuth.h"
#include "NexusModInfo.h"
#include <KxFramework/KxUUID.h>

namespace KxWebSocket
{
	class IClient;
}

namespace Kortex::NetworkManager
{
	class NexusModNetwork;
	class NexusUtility;

	class NexusAuth: public ModNetworkAuth
	{
		friend class NexusModNetwork;

		private:
			NexusModNetwork& m_Nexus;
			NexusUtility& m_Utility;

			KxSecretDefaultStoreService m_CredentialsStore;
			std::unique_ptr<KxWebSocket::IClient> m_WebSocketClient;

			mutable std::optional<NexusValidationReply> m_LastValidationReply;
			wxString m_UserToken;
			KxUUID m_SessionGUID;
			bool m_IsAuthenticated = false;

		protected:
			wxWindow* GetInvokingWindow() const override;
			KxStandardID OnAuthSuccess() override;
			KxStandardID OnAuthFail() override;
			KxSecretDefaultStoreService& GetSecretStore() override
			{
				return m_CredentialsStore;
			}

		private:
			void OnMenu(KxMenu& menu);
			void RequestUserPicture(const NexusValidationReply& info);
			std::optional<NexusValidationReply> DoGetValidationInfo(const wxString& apiKey = {}, bool noErrorReport = false) const;

		public:
			NexusAuth(NexusModNetwork& nexus, NexusUtility& utility);

		public:
			bool IsAuthenticated() const override;
			bool Authenticate() override;
			bool ValidateAuth() override;
			bool SignOut() override;

			std::optional<NexusValidationReply> GetValidationInfo() const;
	};
}
