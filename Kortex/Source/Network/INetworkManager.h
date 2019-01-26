#pragma once
#include "stdafx.h"
#include "Application/IManager.h"
#include "Common.h"
#include "INetworkProvider.h"
#include <KxFramework/KxSingleton.h>
class KMainWindow;
class KxAuiToolBarItem;
class KxAuiToolBarEvent;

namespace Kortex
{
	namespace NetworkManager
	{
		class Config;
	}
	namespace NetworkManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	};

	class INetworkManager:
		public ManagerWithTypeInfo<IManager, NetworkManager::Internal::TypeInfo>,
		public KxSingletonPtr<INetworkManager>
	{
		friend class KMainWindow;

		private:
			void CallOnToolBarButton(KxAuiToolBarEvent& event)
			{
				OnToolBarButton(event);
			}

		protected:
			virtual void OnSetToolBarButton(KxAuiToolBarItem* button) = 0;
			virtual void OnToolBarButton(KxAuiToolBarEvent& event) = 0;

			template<class T> T& AddProvider()
			{
				INetworkProvider::Vector& items = GetProviders();
				return static_cast<T&>(*items.emplace_back(INetworkProvider::Create<T>(items.size())));
			}

		public:
			INetworkManager();

		public:
			virtual const NetworkManager::Config& GetConfig() const = 0;
			virtual wxString GetCacheFolder() const = 0;
		
			virtual const INetworkProvider::Vector& GetProviders() const = 0;
			virtual INetworkProvider::Vector& GetProviders() = 0;

			virtual INetworkProvider* GetDefaultProvider() const = 0;
			bool IsDefaultProviderAvailable() const;
			NetworkProviderID GetDefaultProviderID() const;
		
			virtual INetworkProvider* FindProvider(const wxString& name) const = 0;
			virtual INetworkProvider* GetProvider(NetworkProviderID providerID) const = 0;
			bool HasProvider(NetworkProviderID providerID) const
			{
				return GetProvider(providerID) != nullptr;
			}
	
			virtual void OnAuthStateChanged() = 0;
	};
}
