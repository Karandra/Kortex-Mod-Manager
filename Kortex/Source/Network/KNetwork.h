#pragma once
#include "stdafx.h"
#include "KManager.h"
#include "KNetworkConstants.h"
#include "KNetworkProvider.h"
#include "KProgramOptions.h"
#include <KxFramework/KxSingleton.h>
class KDownloadManager;
class KMainWindow;
class KxAuiToolBarEvent;
class KxAuiToolBarItem;
class KxMenuEvent;
class KxMenu;

class KNetwork: public KManager, public KxSingletonPtr<KNetwork>
{
	friend class KNetworkProvider;
	friend class KMainWindow;

	public:
		using ProvidersArray = std::array<KNetworkProvider*, KNETWORK_PROVIDER_ID_MAX>;
		using ProvidersArrayUPtr = std::array<std::unique_ptr<KNetworkProvider>, KNETWORK_PROVIDER_ID_MAX>;

	private:
		ProvidersArrayUPtr m_Providers;
		std::unique_ptr<KDownloadManager> m_DownloadManager;

		KxAuiToolBarItem* m_LoginButton = NULL;
		KNetworkProviderID m_CurrentProvider = KNETWORK_PROVIDER_ID_INVALID;
		KxMenu* m_Menu = NULL;

		KProgramOptionUI m_Options;

	private:
		virtual void OnInit() override;

		template<class T> T& NewProvider()
		{
			const constexpr KNetworkProviderID typeID = T::GetTypeID();
			m_Providers[typeID] = std::make_unique<T>(typeID);
			m_Providers[typeID]->Init();
			return static_cast<T&>(*m_Providers[typeID]);
		}
		void ValidateAuth();
		bool SetCurrentProviderToFirstAvailableIfNone();

	private:
		void OnSetToolBarButton(KxAuiToolBarItem* button);
		bool GetProviderInfo(const KNetworkProvider& provider, wxString& label, wxBitmap& bitmap, bool name = true) const;
		void UpdateButton();
		void CreateMenu();
		void QueueUIUpdate();

		void OnSignInOut(KxMenuEvent& event);
		void OnSelectProvider(KxMenuEvent& event);
		void OnToolBarButton(KxAuiToolBarEvent& event);

	public:
		KNetwork();
		virtual ~KNetwork();

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual wxString GetVersion() const override;
		virtual KImageEnum GetImageID() const override;

	public:
		void OnAuthStateChanged();

		wxString GetUniqueID() const;
		wxString GetCacheFolder() const;
		
		bool IsCurrentProviderAvailable() const;
		KNetworkProvider* GetCurrentProvider() const;
		KNetworkProviderID GetCurrentProviderID() const
		{
			return m_CurrentProvider;
		}
		ProvidersArray GetProviders() const
		{
			ProvidersArray list;
			for (size_t i = 0; i < m_Providers.size(); i++)
			{
				list[i] = m_Providers[i].get();
			}
			return list;
		}
		
		bool IsValidProviderID(KNetworkProviderID providerID) const
		{
			return (size_t)providerID < m_Providers.size();
		}
		KNetworkProvider* FindProvider(const wxString& name) const;
		KNetworkProvider* GetProvider(intptr_t providerID) const
		{
			return GetProvider(static_cast<KNetworkProviderID>(providerID));
		}
		KNetworkProvider* GetProvider(KNetworkProviderID providerID) const
		{
			if (IsValidProviderID(providerID))
			{
				return m_Providers[providerID].get();
			}
			return NULL;
		}
};
