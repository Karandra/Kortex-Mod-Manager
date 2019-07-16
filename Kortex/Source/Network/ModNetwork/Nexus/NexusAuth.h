#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/IModNetwork.h"
#include "Network/ModNetworkAuth.h"
#include "NexusNetworkReply.h"
#include <KxFramework/KxUUID.h>
class KxIWebSocketClient;

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
			std::unique_ptr<KxIWebSocketClient> m_WebSocketClient;

			std::optional<NexusValidationReply> m_LastValidationReply;
			wxString m_UserToken;
			KxUUID m_SessionGUID;

		protected:
			wxWindow* GetInvokingWindow() const override;
			void OnAuthSuccess() override;
			void OnAuthFail() override;
			void OnAuthReset() override;

			void ResetSessionInfo();
			KxSecretDefaultStoreService& GetCredentialsStore() override
			{
				return m_CredentialsStore;
			}

		private:
			void OnToolBarMenu(KxMenu& menu);
			void RequestUserPicture(const NexusValidationReply& info);
			
			const NexusValidationReply* GetLastValidationReply() const
			{
				return m_LastValidationReply ? &m_LastValidationReply.value() : nullptr;
			}
			std::optional<NexusValidationReply> DoGetValidationInfo(const wxString& apiKey = {}, bool silent = false);

		public:
			NexusAuth(NexusModNetwork& nexus, NexusUtility& utility);

		public:
			bool IsAuthenticated() const override;
			void Authenticate() override;
			void ValidateAuth() override;
			void SignOut() override;

		public:
			std::optional<NexusValidationReply> GetValidationInfo();
	};
}
