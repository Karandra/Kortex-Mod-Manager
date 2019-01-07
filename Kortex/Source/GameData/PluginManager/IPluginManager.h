#pragma once
#include "stdafx.h"
#include "Application/IPluggableManager.h"
#include "IGamePlugin.h"
#include "Common.h"
#include <Kortex/Events.hpp>
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxXML.h>
class IEvent;

namespace Kortex
{
	class IGameMod;
	class IGamePlugin;
}
namespace Kortex::PluginManager
{
	class IDisplayModel;
	class Config;

	enum class SyncListMode
	{
		ActivateAll,
		DeactivateAll,
		DoNotChange,
	};
	enum class MoveMode
	{
		Before,
		After,
	};

	namespace Internal
	{
		extern const SimpleManagerInfo TypeInfo;

		namespace ManagerImplementation
		{
			const constexpr wxChar Bethesda[] = wxS("Bethesda");
			const constexpr wxChar Bethesda2[] = wxS("Bethesda2");
			const constexpr wxChar BethesdaMorrowind[] = wxS("BethesdaMorrowind");
		}
		namespace PluginImplementation
		{
			const constexpr wxChar BethesdaMorrowind[] = wxS("BethesdaMorrowind");
			const constexpr wxChar BethesdaOblivion[] = wxS("BethesdaOblivion");
			const constexpr wxChar BethesdaSkyrim[] = wxS("BethesdaSkyrim");
		}
	}
}

namespace Kortex
{
	class IPluginManager:
		public ManagerWithTypeInfo<IPluggableManager, SimpleManagerInfo, PluginManager::Internal::TypeInfo>,
		public RTTI::IInterface<IPluginManager>,
		public KxSingletonPtr<IPluginManager>
	{
		friend class IGamePlugin;

		public:
			using MoveMode = PluginManager::MoveMode;
			using SyncListMode = PluginManager::SyncListMode;

		private:
			void OnVirtualTreeInvalidated(IEvent& event);
			intptr_t OnGetOrderIndex(const IGamePlugin& plugin) const;
			
		protected:
			virtual intptr_t OnGetPluginPriority(const IGamePlugin& plugin) const = 0;
			virtual intptr_t OnGetPluginDisplayPriority(const IGamePlugin& plugin) const = 0;

			virtual bool HasDependentPlugins(const IGamePlugin& pluginEntry) const = 0;
			virtual IGamePlugin::RefVector GetDependentPlugins(const IGamePlugin& pluginEntry) const = 0;

		public:
			IPluginManager();

		public:
			virtual const PluginManager::Config& GetConfig() const = 0;

			virtual std::unique_ptr<PluginManager::IPluginReader> CreatePluginReader() = 0;
			virtual std::unique_ptr<PluginManager::IDisplayModel> CreateDisplayModel() = 0;
			virtual wxString GetPluginsLocation() const = 0;
			virtual wxString GetPluginTypeName(const IGamePlugin& plugin) const = 0;
			virtual const IGameMod* FindOwningMod(const IGamePlugin& pluginEntry) const = 0;

			bool HasPlugins() const
			{
				return !GetPlugins().empty();
			}
			void ClearPlugins()
			{
				GetPlugins().clear();
			}
			virtual const IGamePlugin::Vector& GetPlugins() const = 0;
			virtual IGamePlugin::Vector& GetPlugins() = 0;

			virtual void Save() const = 0;
			virtual void Load() = 0;
			virtual void LoadNativeOrder() = 0;
			virtual void Invalidate() = 0;

			virtual bool MovePlugins(const IGamePlugin::RefVector& entriesToMove, const IGamePlugin& anchor, MoveMode moveMode = MoveMode::After) = 0;
			virtual void SyncWithPluginsList(const KxStringVector& pluginNamesList, SyncListMode mode = SyncListMode::ActivateAll) = 0;
			virtual KxStringVector GetPluginsList(bool activeOnly = false) const = 0;
			virtual IGamePlugin* FindPluginByName(const wxString& name) const = 0;
			bool IsPluginActive(const wxString& pluginName) const;
			void SetAllPluginsActive(bool isActive);

			virtual bool CheckSortingTool(const PluginManager::SortingToolEntry& entry) = 0;
			virtual void RunSortingTool(const PluginManager::SortingToolEntry& entry) = 0;
	};
}
