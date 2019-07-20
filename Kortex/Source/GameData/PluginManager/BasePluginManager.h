#pragma once
#include "stdafx.h"
#include "GameData/IPluginManager.h"

namespace Kortex::PluginManager
{
	class BasePluginManager: public IPluginManager
	{
		friend class IGamePlugin;

		public:
			Config m_Config;
			IGamePlugin::Vector m_Plugins;

		protected:
			intptr_t OnGetPluginPriority(const IGamePlugin& plugin) const override;
			intptr_t OnGetPluginDisplayPriority(const IGamePlugin& plugin) const override;

			virtual void OnInit() override;
			virtual void OnExit() override;
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;

		public:
			const PluginManager::Config& GetConfig() const override
			{
				return m_Config;
			}

			const IGamePlugin::Vector& GetPlugins() const override
			{
				return m_Plugins;
			}
			IGamePlugin::Vector& GetPlugins() override
			{
				return m_Plugins;
			}
			
			void Invalidate() override;
			bool MovePlugins(const IGamePlugin::RefVector& entriesToMove, const IGamePlugin& anchor, MoveMode moveMode = MoveMode::After) override;
			void SyncWithPluginsList(const KxStringVector& pluginNamesList, SyncListMode mode = SyncListMode::ActivateAll) override;
			KxStringVector GetPluginsList(bool activeOnly = false) const override;
			IGamePlugin* FindPluginByName(const wxString& name) const override;

			bool CheckSortingTool(const PluginManager::SortingToolEntry& entry) override;
			void RunSortingTool(const PluginManager::SortingToolEntry& entry) override;
	};
}
