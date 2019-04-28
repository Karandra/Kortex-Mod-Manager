#pragma once
#include "stdafx.h"
#include "Common.h"
#include "ModRepositoryReply.h"
#include "ModRepositoryRequest.h"
#include "GameInstance/GameID.h"
#include "Utility/KImageProvider.h"
#include <KxFramework/KxQueryInterface.h>
#include <KxFramework/KxComponentSystem.h>
#include <KxFramework/KxSecretStore.h>
#include <KxFramework/KxVersion.h>

namespace Kortex
{
	class IDownloadEntry;
}

namespace Kortex
{
	class IModNetwork: public KxRTTI::IInterface<IModNetwork>, public KxComponentContainer
	{
		friend class INetworkManager;

		public:
			using Vector = std::vector<std::unique_ptr<IModNetwork>>;
			using RefVector = std::vector<IModNetwork*>;

		public:
			static KImageEnum GetGenericIcon();
			template<class TModNetwork, class... Args> static std::unique_ptr<TModNetwork> Create(Args&&... arg)
			{
				auto modNetwork = std::make_unique<TModNetwork>(std::forward<Args>(arg)...);

				IModNetwork& modNetworkBase = *modNetwork;
				modNetworkBase.Init();

				return modNetwork;
			}

		private:
			void Init();

		protected:
			wxString GetIPBModPageURL(ModID modID, const wxString& modSignature = {}) const;

		public:
			bool IsDefault() const;
			wxString GetCacheFolder() const;
			
			virtual KImageEnum GetIcon() const = 0;
			virtual wxString GetName() const = 0;
			
			virtual wxString TranslateGameIDToNetwork(const GameID& id = {}) const = 0;
			virtual GameID TranslateGameIDFromNetwork(const wxString& id) const = 0;
			virtual void ConvertDescriptionText(wxString& description) const
			{
			}
			
			virtual wxString GetModPageBaseURL(const GameID& id = {}) const = 0;
			virtual wxString GetModPageURL(const ModRepositoryRequest& request) = 0;
	};
}
