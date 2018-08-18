#pragma once
#include "stdafx.h"
#include "KPluggableManager.h"
#include "KPMPluginEntry.h"
#include "ModManager/KModManager.h"
#include "ModManager/KModManagerModList.h"
#include "KProgramOptions.h"
#include <KxFramework/KxSingleton.h>
class KPluginManagerWorkspace;
class KPluginManagerConfig;
class KPluginManagerConfigSortingToolEntry;
class KModEntry;
class KxXMLNode;

class KPluginManager: public KPluggableManager, public KxSingletonPtr<KPluginManager>
{
	friend class KPluginManagerConfig;

	public:
		enum SyncListMode
		{
			EnableAll = 0,
			DisableAll = 1,
			DoNotChange = 2,
		};

		static KPluginManager* QueryInterface(const wxString& name, const KxXMLNode& configNode = KxXMLNode(), const KPluginManagerConfig* profilePluginConfig = NULL);

	private:
		KProgramOptionUI m_GeneralOptions;
		KProgramOptionUI m_SortingToolsOptions;

	private:
		KPMPluginEntryRefVector CollectDependentPlugins(const KPMPluginEntry* pluginEntry, bool bFirstOnly) const;

	public:
		KPluginManager(const wxString& interfaceName, const KxXMLNode& configNode, const KPluginManagerConfig* profilePluginConfig);
		virtual ~KPluginManager();

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual wxString GetVersion() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_PLUG_DISCONNECT;
		}

	public:
		virtual bool IsOK() const = 0;
		KProgramOption& GetGeneralOptions()
		{
			return m_GeneralOptions;
		}
		KProgramOption& GetSortingToolsOptions()
		{
			return m_SortingToolsOptions;
		}

		virtual KPMPluginEntry* NewPluginEntry(const wxString& name, bool isActive, KPMPluginEntryType type);
		KPMPluginEntry* EmplacePluginEntry(const wxString& name, bool isActive, KPMPluginEntryType type)
		{
			return GetEntries().emplace_back(NewPluginEntry(name, isActive, type)).get();
		}

		virtual const wxString& GetPluginsLocation() const = 0;
		virtual wxString GetPluginRootRelativePath(const wxString& fileName) const = 0;
		virtual wxString GetPluginTypeName(KPMPluginEntryType type) const;
		virtual bool IsEntryTypeSupported(KPMPluginEntryType type) const = 0;

		virtual KPMPluginEntryVector& GetEntries() = 0;
		virtual const KPMPluginEntryVector& GetEntries() const = 0;
		bool IsPluginsDataLoaded() const
		{
			return !GetEntries().empty();
		}

		virtual bool IsActiveVFSNeeded() const override
		{
			return false;
		}
		virtual bool Save() = 0;
		virtual bool Load() = 0;
		virtual bool LoadNativeOrder() = 0;
		bool LoadIfNeeded()
		{
			return !IsPluginsDataLoaded() ? Load() : true;
		}

		int GetPluginIndex(const KPMPluginEntry* modEntry) const;
		bool IsValidModIndex(int nModIndex) const;;
		bool MovePluginsIntoThis(const KPMPluginEntryRefVector& entriesToMove, const KPMPluginEntry* pAnchor);
		void SetAllPluginsEnabled(bool isEnabled);

		virtual bool HasDependentPlugins(const KPMPluginEntry* pluginEntry) const;
		virtual KPMPluginEntryRefVector GetDependentPlugins(const KPMPluginEntry* pluginEntry) const;
		virtual const KModEntry* GetParentMod(const KPMPluginEntry* pluginEntry) const;
		virtual bool IsPluginActive(const wxString& sPluginName) const;
		virtual void SyncWithPluginsList(const KxStringVector& tPluginNamesList, SyncListMode mode = EnableAll);
		virtual KxStringVector GetPluginsList(bool bActiveOnly = false) const;
		KPMPluginEntry* FindPluginByName(const wxString& name) const;
		void UpdateAllPlugins();

		virtual bool CheckSortingTool(const KPluginManagerConfigSortingToolEntry& entry);
		virtual void RunSortingTool(const KPluginManagerConfigSortingToolEntry& entry);
};
