#pragma once
#include "stdafx.h"
#include "Common.h"
#include "ModRepositoryReply.h"
#include "ModRepositoryRequest.h"
#include "GameInstance/GameID.h"
#include "Application/Resources/ImageResourceID.h"
#include <KxFramework/KxComponentSystem.h>
#include <KxFramework/KxSecretStore.h>
#include <KxFramework/KxVersion.h>
#include <KxFramework/KxURI.h>
#include <Kx/RTTI.hpp>
class KxMenu;
class KxXMLNode;

namespace Kortex
{
	class DownloadItem;
	class IGameInstance;
	class IGameMod;
}

namespace Kortex
{
	class IModNetwork: public KxRTTI::Interface<IModNetwork>, public KxComponentContainer
	{
		KxDecalreIID(IModNetwork, {0xc58037c8, 0x9e52, 0x45df, {0xac, 0xd5, 0xa5, 0xb7, 0x4e, 0x3f, 0x88, 0x5f}});

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
			KxURI GetIPBModPageURI(ModID modID, const wxString& modSignature = {}) const;

		public:
			bool IsDefault() const;
			wxString GetCacheDirectory() const;
			wxString GetLocationInCache(const wxString& relativePath) const;
			
			virtual ResourceID GetIcon() const = 0;
			virtual wxString GetName() const = 0;
			
			virtual wxString TranslateGameIDToNetwork(const GameID& id = {}) const = 0;
			virtual GameID TranslateGameIDFromNetwork(const wxString& id) const = 0;
			virtual void ConvertDescriptionText(wxString& description) const
			{
			}
			
			virtual KxURI GetModPageBaseURI(const GameID& id = {}) const = 0;
			virtual KxURI GetModPageURI(const ModRepositoryRequest& request) const = 0;
			KxURI GetModPageURI(const IGameMod& mod) const;
			KxURI GetModPageURI(const DownloadItem& download) const;

		public:
			virtual void OnToolBarMenu(KxMenu& menu)
			{
			}
			virtual void OnModListMenu(KxMenu& menu, const ModsRefVector& selectedMods, IGameMod* focusedMod)
			{
			}
	};
}
