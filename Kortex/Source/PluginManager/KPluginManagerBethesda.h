#pragma once
#include "stdafx.h"
#include "KPluginManager.h"
#include "KPluginEntryBethesda.h"
class KPluginManagerConfigSortingToolEntry;
class KPluginViewModelBethesda;

class KPluginManagerBethesda: public KPluginManager
{
	protected:
		/* Config */
		wxString m_PluginsLocation;
		wxString m_ActiveListFile;
		wxString m_OrderListFile;
		wxString m_ActiveFileHeader;
		wxString m_OrderFileHeader;

		bool m_ShouldChangeFileModificationDate;
		bool m_ShouldSortByFileModificationDate;

		/* Data */
		std::unique_ptr<KPluginViewModelBethesda> m_ViewModel;

	protected:
		void SortByDate();

		KPluginEntry::RefVector CollectDependentPlugins(const KPluginEntry& pluginEntry, bool firstOnly) const;
		virtual bool CheckExtension(const wxString& name) const;

	protected:
		virtual void LoadNativeOrderBG();
		virtual void LoadNativeActiveBG();
		virtual void SaveNativeOrderBG() const;

		virtual wxString OnWriteToLoadOrder(const KPluginEntry& entry) const;
		virtual wxString OnWriteToActiveOrder(const KPluginEntry& entry) const;
		virtual KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override;

	public:
		KPluginManagerBethesda(const wxString& interfaceName, const KxXMLNode& configNode);
		virtual ~KPluginManagerBethesda();

	public:
		virtual KWorkspace* GetWorkspace() const override;
		virtual KPluginViewModel* GetViewModel() const override;

		virtual KPluginEntryBethesda* NewPluginEntry(const wxString& name, bool isActive) const;
		virtual wxString GetPluginsLocation() const override
		{
			return m_PluginsLocation;
		}
		virtual wxString GetPluginTypeName(const KPluginEntry& pluginEntry) const override;
		virtual wxString GetPluginTypeName(bool isMaster, bool isLight) const;
		virtual wxString GetPluginRootRelativePath(const wxString& fileName) const
		{
			return m_PluginsLocation + '\\' + fileName;
		}

		virtual void Save() const override;
		virtual void Load() override;
		virtual void LoadNativeOrder() override;

		virtual bool ShouldChangeFileModificationDate() const
		{
			return m_ShouldChangeFileModificationDate;
		}
		virtual bool ShouldSortByFileModificationDate() const
		{
			return m_ShouldSortByFileModificationDate;
		}

		virtual bool HasDependentPlugins(const KPluginEntry& pluginEntry) const override;
		virtual KPluginEntry::RefVector GetDependentPlugins(const KPluginEntry& pluginEntry) const override;
		virtual const KModEntry* FindParentMod(const KPluginEntry& pluginEntry) const override;

		virtual intptr_t GetPluginPriority(const KPluginEntry& modEntry) const override;
};
