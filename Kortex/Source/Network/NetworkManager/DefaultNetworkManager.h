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
	class IAuthenticableModSource;
	class IModRepository;
}

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
			IModSource::Vector m_ModSources;
			IModSource* m_DefaultModSource = nullptr;
			NetworkManager::Config m_Config;

			KxAuiToolBarItem* m_LoginButton = nullptr;
			KxMenu* m_Menu = nullptr;

		private:
			virtual void OnInit() override;
			virtual void OnExit() override;
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;

			void ValidateAuth();
			bool AdjustDefaultModSource();

		private:
			void OnSetToolBarButton(KxAuiToolBarItem* button) override;
			void UpdateButton();
			void CreateMenu();
			void QueueUIUpdate();

			void OnSignInOut(KxMenuEvent& event);
			void OnSelectDefaultModSource(KxMenuEvent& event);
			void OnToolBarButton(KxAuiToolBarEvent& event) override;

		public:
			const NetworkManager::Config& GetConfig() const override
			{
				return m_Config;
			}
			wxString GetCacheFolder() const override;
			
			const IModSource::Vector& GetModSources() const override
			{
				return m_ModSources;
			}
			IModSource::Vector& GetModSources() override
			{
				return m_ModSources;
			}
			IModSource* GetDefaultModSource() const override;
			IModSource* GetModSource(const wxString& name) const override;
			
			void OnAuthStateChanged() override;
	};
}
