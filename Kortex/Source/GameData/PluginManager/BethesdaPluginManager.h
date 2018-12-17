#pragma once
#include "stdafx.h"
#include "IPluginManager.h"
#include "BasePluginManager.h"
#include "BethesdaPlugin.h"
#include <KxFramework/KxLibrary.h>

namespace Kortex::PluginManager
{
	class LootAPIConfig
	{
		private:
			KxLibrary m_Librray;
			wxString m_Branch;
			wxString m_Repository;
			wxString m_FolderName;
			wxString m_LocalGamePath;

		public:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& node);

		public:
			bool IsOK() const;

			wxString GetBranch() const;
			wxString GetRepository() const;
			wxString GetFolderName() const;
			wxString GetLocalGamePath() const;
	};
}

namespace Kortex::PluginManager
{
	class LootAPI;
	class BethesdaDisplayModel;

	class BethesdaPluginManager: public RTTI::IMultiInterface<BethesdaPluginManager, BasePluginManager>
	{
		protected:
			LootAPIConfig m_LootAPIConfig;
			std::unique_ptr<LootAPI> m_LootAPI;

			wxString m_PluginsLocation;
			wxString m_ActiveListFile;
			wxString m_OrderListFile;
			wxString m_ActiveFileHeader;
			wxString m_OrderFileHeader;

			bool m_ShouldChangeFileModificationDate;
			bool m_ShouldSortByFileModificationDate;

			std::unique_ptr<BethesdaDisplayModel> m_DisplayModel;

		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;

			void SortByDate();
			virtual bool CheckExtension(const wxString& name) const;
			wxString OnFormatPriority(const IGamePlugin& plugin, intptr_t value) const override;
			
			IGamePlugin::RefVector CollectDependentPlugins(const IGamePlugin& plugin, bool firstOnly) const;
			bool HasDependentPlugins(const IGamePlugin& plugin) const override;
			IGamePlugin::RefVector GetDependentPlugins(const IGamePlugin& plugin) const override;

		protected:
			virtual void LoadNativeOrderBG();
			virtual void LoadNativeActiveBG();
			virtual void SaveNativeOrderBG() const;

			virtual wxString OnWriteToLoadOrder(const IGamePlugin& plugin) const;
			virtual wxString OnWriteToActiveOrder(const IGamePlugin& plugin) const;
			virtual KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override;

		public:
			BethesdaPluginManager();
			virtual ~BethesdaPluginManager();

		public:
			const LootAPIConfig& GetLootAPIConfig() const
			{
				return m_LootAPIConfig;
			}
			KWorkspace* GetWorkspace() const override;
			IDisplayModel* GetDisplayModel() const override;

			virtual std::unique_ptr<IGamePlugin> CreatePlugin(const wxString& fullPath, bool isActive) const;
			std::unique_ptr<PluginManager::IPluginReader> CreatePluginReader() const override;

			virtual wxString GetPluginsLocation() const override
			{
				return m_PluginsLocation;
			}
			virtual wxString GetPluginTypeName(const IGamePlugin& plugin) const override;
			virtual wxString GetPluginTypeName(bool isMaster, bool isLight) const;
			virtual wxString GetPluginRootRelativePath(const wxString& fileName) const
			{
				return m_PluginsLocation + wxS('\\') + fileName;
			}
			virtual const IGameMod* FindOwningMod(const IGamePlugin& plugin) const override;

			virtual void Save() const override;
			virtual void Load() override;
			virtual void LoadNativeOrder() override;

			bool ShouldChangeFileModificationDate() const
			{
				return m_ShouldChangeFileModificationDate;
			}
			bool ShouldSortByFileModificationDate() const
			{
				return m_ShouldSortByFileModificationDate;
			}
	};
}
