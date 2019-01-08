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
	class LibLoot;
	class BethesdaDisplayModel;

	class BethesdaPluginManager: public RTTI::IMultiInterface<BethesdaPluginManager, BasePluginManager>
	{
		protected:
			LootAPIConfig m_LibLootConfig;
			std::unique_ptr<LibLoot> m_LootAPI;

			wxString m_PluginsLocation;
			wxString m_ActiveListFile;
			wxString m_OrderListFile;
			wxString m_ActiveFileHeader;
			wxString m_OrderFileHeader;

			bool m_ShouldChangeFileModificationDate;
			bool m_ShouldSortByFileModificationDate;

		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;

			void SortByDate();
			virtual bool CheckExtension(const wxString& name) const;

			IGamePlugin::RefVector CollectDependentPlugins(const IGamePlugin& plugin, bool firstOnly) const;
			bool HasDependentPlugins(const IGamePlugin& plugin) const override;
			IGamePlugin::RefVector GetDependentPlugins(const IGamePlugin& plugin) const override;

		protected:
			virtual void LoadNativeOrderBG();
			virtual void LoadNativeActiveBG();
			virtual void SaveNativeOrderBG() const;

			virtual wxString OnWriteToLoadOrder(const IGamePlugin& plugin) const;
			virtual wxString OnWriteToActiveOrder(const IGamePlugin& plugin) const;
			KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override;

		public:
			BethesdaPluginManager();
			virtual ~BethesdaPluginManager();

		public:
			const LootAPIConfig& GetLibLootConfig() const
			{
				return m_LibLootConfig;
			}
			KWorkspace* GetWorkspace() const override;

			std::unique_ptr<PluginManager::IDisplayModel> CreateDisplayModel() override;
			std::unique_ptr<IPluginReader> CreatePluginReader() override;
			virtual std::unique_ptr<IGamePlugin> CreatePlugin(const wxString& fullPath, bool isActive);

			wxString GetPluginsLocation() const override
			{
				return m_PluginsLocation;
			}
			wxString GetPluginTypeName(const IGamePlugin& plugin) const override;
			wxString GetPluginTypeName(bool isMaster, bool isLight) const;
			virtual wxString GetPluginRootRelativePath(const wxString& fileName) const
			{
				return m_PluginsLocation + wxS('\\') + fileName;
			}
			const IGameMod* FindOwningMod(const IGamePlugin& plugin) const override;

			void Save() const override;
			void Load() override;
			void LoadNativeOrder() override;

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
