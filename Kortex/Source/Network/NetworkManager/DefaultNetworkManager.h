#pragma once
#include "stdafx.h"
#include "Network/INetworkManager.h"
class KMainWindow;
class KxAuiToolBarEvent;
class KxAuiToolBarItem;
class KxMenuEvent;
class KxMenu;

namespace Kortex
{
	class ModNetworkAuth;
	class ModNetworkRepository;
}

namespace Kortex::NetworkManager
{
	class DefaultNetworkManager: public INetworkManager
	{
		private:
			IModNetwork::Vector m_ModNetworks;
			IModNetwork* m_DefaultModNetwork = nullptr;

			KxAuiToolBarItem* m_LoginButton = nullptr;
			KxMenu* m_Menu = nullptr;

		private:
			virtual void OnInit() override;
			virtual void OnExit() override;
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;

			void ValidateAuth();
			bool AdjustDefaultModNetwork();

		private:
			void OnSetToolBarButton(KxAuiToolBarItem* button) override;
			void UpdateButton();
			void CreateMenu();
			void QueueUIUpdate();

			void OnSignInOut(KxMenuEvent& event);
			void OnSelectDefaultModSource(KxMenuEvent& event);
			void OnToolBarButton(KxAuiToolBarEvent& event) override;

		public:
			wxString GetCacheDirectory() const override;
			
			IModNetwork::Vector& GetModNetworks() override
			{
				return m_ModNetworks;
			}
			IModNetwork* GetDefaultModNetwork() const override;
			IModNetwork* GetModNetworkByName(const wxString& name) const override;
			
			void OnAuthStateChanged() override;
	};
}
