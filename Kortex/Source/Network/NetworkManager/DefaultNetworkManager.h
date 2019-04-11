#pragma once
#include "stdafx.h"
#include "Network/INetworkManager.h"
class KMainWindow;
class KxAuiToolBarEvent;
class KxAuiToolBarItem;
class KxMenuEvent;
class KxMenu;

namespace Kortex::NetworkManager
{
	class Config
	{
		private:
			wxString m_NexusID;
			int m_SteamID = -1;

		public:
			void OnLoadInstance(IGameInstance& profile, const KxXMLNode& node);

		public:
			bool HasNexusID() const
			{
				return !m_NexusID.IsEmpty();
			}
			wxString GetNexusID() const;

			bool HasSteamID() const
			{
				return m_SteamID > 0;
			}
			int GetSteamID() const
			{
				return m_SteamID;
			}
	};
}

namespace Kortex::NetworkManager
{
	class DefaultNetworkManager: public INetworkManager
	{
		private:
			INetworkModSource::Vector m_Providers;
			NetworkProviderID m_DefaultProvider = NetworkProviderIDs::Invalid;
			NetworkManager::Config m_Config;

			KxAuiToolBarItem* m_LoginButton = nullptr;
			KxMenu* m_Menu = nullptr;

		private:
			virtual void OnInit() override;
			virtual void OnExit() override;
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;

			void ValidateAuth();
			bool SetDefaultProviderToFirstAvailableIfNone();

		private:
			void OnSetToolBarButton(KxAuiToolBarItem* button) override;
			bool GetProviderInfo(const INetworkModSource& provider, wxString& label, wxBitmap& bitmap, bool name = true) const;
			void UpdateButton();
			void CreateMenu();
			void QueueUIUpdate();

			void OnSignInOut(KxMenuEvent& event);
			void OnSelectActiveProvider(KxMenuEvent& event);
			void OnToolBarButton(KxAuiToolBarEvent& event) override;

		public:
			const NetworkManager::Config& GetConfig() const override
			{
				return m_Config;
			}
			wxString GetCacheFolder() const override;
		
			const INetworkModSource::Vector& GetProviders() const override
			{
				return m_Providers;
			}
			INetworkModSource::Vector& GetProviders() override
			{
				return m_Providers;
			}
			INetworkModSource* GetDefaultProvider() const override;

			INetworkModSource* FindProvider(const wxString& name) const override;
			INetworkModSource* GetProvider(NetworkProviderID providerID) const override;
			
			void OnAuthStateChanged() override;
	};
}
