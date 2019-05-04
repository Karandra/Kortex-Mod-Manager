#pragma once
#include "stdafx.h"
#include "Common.h"
#include "ModRepositoryReply.h"
#include "ModRepositoryRequest.h"
#include "GameInstance/GameID.h"
#include "Application/Resources/ImageResourceID.h"
#include <KxFramework/KxQueryInterface.h>
#include <KxFramework/KxComponentSystem.h>
#include <KxFramework/KxSecretStore.h>
#include <KxFramework/KxVersion.h>
class KxMenu;
class KxXMLNode;

namespace Kortex
{
	class IDownloadEntry;
	class IGameInstance;
	class IGameMod;
}

namespace Kortex
{
	class IModNetwork: public KxRTTI::IInterface<IModNetwork>, public KxComponentContainer
	{
		friend class INetworkManager;
		friend class NetworkModule;

		public:
			using Vector = std::vector<std::unique_ptr<IModNetwork>>;
			using RefVector = std::vector<IModNetwork*>;
			using ModsRefVector = std::vector<IGameMod*>;

		public:
			static ResourceID GetGenericIcon();

		private:
			void DoOnInit();
			void DoOnExit();

			virtual void OnInit() = 0;
			virtual void OnExit() = 0;

		protected:
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& networkNode) = 0;
			wxString GetIPBModPageURL(ModID modID, const wxString& modSignature = {}) const;

		public:
			bool IsDefault() const;
			wxString GetCacheFolder() const;
			
			virtual ResourceID GetIcon() const = 0;
			virtual wxString GetName() const = 0;
			
			virtual wxString TranslateGameIDToNetwork(const GameID& id = {}) const = 0;
			virtual GameID TranslateGameIDFromNetwork(const wxString& id) const = 0;
			virtual void ConvertDescriptionText(wxString& description) const
			{
			}
			
			virtual wxString GetModPageBaseURL(const GameID& id = {}) const = 0;
			virtual wxString GetModPageURL(const ModRepositoryRequest& request) = 0;

		public:
			virtual void OnToolBarMenu(KxMenu& menu)
			{
			}
			virtual void OnModsMenu(KxMenu& menu, const ModsRefVector& selectedMods, IGameMod* focusedMod)
			{
			}
	};
}
