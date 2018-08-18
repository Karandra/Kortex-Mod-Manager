#pragma once
#include "stdafx.h"
#include "KPluginManager.h"
#include "KPMPluginEntry.h"
class KPluginManagerConfigSortingToolEntry;

class KPMPluginEntryBethesdaGeneric: public KPMPluginEntry
{
	private:
		bool m_HasDependentPlugins = false;

	private:
		void UpdateHasDependentPlugins();

	public:
		KPMPluginEntryBethesdaGeneric(const wxString& name, bool isActive, KPMPluginEntryType type);

	public:
		virtual void OnUpdate() override;

		virtual bool CanToggleEnabled() const override;
		virtual bool IsEnabled() const override;
};

class KPluginManagerBethesdaGeneric: public KPluginManager
{
	protected:
		wxString m_PluginsLocation;
		wxString m_ActiveListFile;
		wxString m_OrderListFile;
		wxString m_ActiveFileHeader;
		wxString m_OrderFileHeader;
		bool m_ChangeFileModificationDate;
		bool m_SortByFileModificationDate;
		KPMPluginEntryVector m_Entries;
		KWorkspace* m_Workspace = NULL;

	protected:
		void Clear();
		void SortByDate();

		virtual KPMPluginEntryType GetPluginTypeFromPath(const wxString& name) const;
		void SetParentModAndPluginFile(KPMPluginEntry* entry);

	protected:
		virtual void LoadNativeOrderBG();
		virtual void LoadNativeActiveBG();
		virtual void SaveNativeOrderBG() const;

		virtual wxString OnWriteToLoadOrder(const KPMPluginEntry* entry) const;
		virtual wxString OnWriteToActiveOrder(const KPMPluginEntry* entry) const;
		virtual KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override;

	public:
		KPluginManagerBethesdaGeneric(const wxString& interfaceName, const KxXMLNode& configNode, const KPluginManagerConfig* profilePluginConfig);
		virtual ~KPluginManagerBethesdaGeneric();

	public:
		virtual bool IsOK() const override;
		virtual KWorkspace* GetWorkspace() const override;
		virtual KPMPluginEntry* NewPluginEntry(const wxString& name, bool isActive, KPMPluginEntryType type) override
		{
			return new KPMPluginEntryBethesdaGeneric(name, isActive, type);
		}

		virtual const wxString& GetPluginsLocation() const override
		{
			return m_PluginsLocation;
		}
		virtual wxString GetPluginRootRelativePath(const wxString& fileName) const override
		{
			return m_PluginsLocation + '\\' + fileName;
		}
		virtual bool IsEntryTypeSupported(KPMPluginEntryType type) const override
		{
			const auto nMask = KPMPE_TYPE_NORMAL|KPMPE_TYPE_MASTER;
			return (~nMask & type) == 0;
		}

		virtual KPMPluginEntryVector& GetEntries() override
		{
			return m_Entries;
		}
		virtual const KPMPluginEntryVector& GetEntries() const override
		{
			return m_Entries;
		}

		virtual bool Save() override;
		virtual bool Load() override;
		virtual bool LoadNativeOrder() override;

		virtual bool ShouldChangeFileModificationDate() const
		{
			return m_ChangeFileModificationDate;
		}
		virtual bool ShouldSortByFileModificationDate() const
		{
			return m_SortByFileModificationDate;
		}

		virtual const KModEntry* GetParentMod(const KPMPluginEntry* pluginEntry) const override;
};
