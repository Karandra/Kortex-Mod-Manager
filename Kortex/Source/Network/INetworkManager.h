#pragma once
#include "stdafx.h"
#include "Application/IManager.h"
#include "Common.h"
#include "INetworkModSource.h"
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

			template<class T> T& AddModSource()
			{
				INetworkModSource::Vector& items = GetModSources();
				return static_cast<T&>(*items.emplace_back(INetworkModSource::Create<T>(items.size())));
			}

		public:
			INetworkManager();

		public:
			virtual const NetworkManager::Config& GetConfig() const = 0;
			virtual wxString GetCacheFolder() const = 0;
		
			virtual const INetworkModSource::Vector& GetModSources() const = 0;
			virtual INetworkModSource::Vector& GetModSources() = 0;

			virtual INetworkModSource* GetDefaultModSource() const = 0;
			bool IsDefaultProviderAvailable() const;
			ModSourceID GetDefaultProviderID() const;
			
			virtual INetworkModSource* FindModSource(const wxString& name) const = 0;
			virtual INetworkModSource* GetModSource(ModSourceID sourceID) const = 0;
			bool HasModSource(ModSourceID sourceID) const
			{
				return GetModSource(sourceID) != nullptr;
			}
	
			virtual void OnAuthStateChanged() = 0;
	};
}
