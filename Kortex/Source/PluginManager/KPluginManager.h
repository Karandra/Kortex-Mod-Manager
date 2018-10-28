#pragma once
#include "stdafx.h"
#include "KPluggableManager.h"
#include "KPluginEntry.h"
#include "ModManager/KModManager.h"
#include "KProgramOptions.h"
#include "KEventsFwd.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxXML.h>
class KPluginManagerWorkspace;
class KPluginManagerConfig;
class KPluginManagerConfigSortingToolEntry;
class KModEntry;
class KPluginViewModel;
class KxXMLNode;

#define KPLUGIN_IMANAGER_BETHESDA wxS("Bethesda")
#define KPLUGIN_IMANAGER_BETHESDA2 wxS("Bethesda2")
#define KPLUGIN_IMANAGER_BETHESDAMW wxS("BethesdaMW")

#define KPLUGIN_IFILE_BETHESDA_MORROWIND wxS("BethesdaMorrowind")
#define KPLUGIN_IFILE_BETHESDA_OBLIVION wxS("BethesdaOblivion")
#define KPLUGIN_IFILE_BETHESDA_SKYRIM wxS("BethesdaSkyrim")

class KPluginManager: public KPluggableManager, public KxSingletonPtr<KPluginManager>
{
	friend class KPluginManagerConfig;

	public:
		enum class SyncListMode
		{
			EnableAll,
			DisableAll,
			DoNotChange,
		};
		enum class MoveMode
		{
			Before,
			After,
		};

		static std::unique_ptr<KPluginManager> QueryInterface(const wxString& name, const KxXMLNode& configNode = KxXMLNode());
		static std::unique_ptr<KPluginReader> QueryPluginReader(const wxString& formatName);

	private:
		KProgramOptionUI m_GeneralOptions;
		KProgramOptionUI m_SortingToolsOptions;

		KPluginEntry::Vector m_Entries;

	protected:
		virtual void OnInit() override;
		void Clear()
		{
			m_Entries.clear();
		}
		void ReadPluginsData();

	private:
		void OnVirtualTreeInvalidated(KEvent& event);

	public:
		KPluginManager(const wxString& interfaceName, const KxXMLNode& configNode);
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
		KProgramOption& GetGeneralOptions()
		{
			return m_GeneralOptions;
		}
		KProgramOption& GetSortingToolsOptions()
		{
			return m_SortingToolsOptions;
		}

		virtual KPluginViewModel* GetViewModel() const = 0;
		virtual wxString GetPluginsLocation() const = 0;
		virtual wxString GetPluginTypeName(const KPluginEntry& pluginEntry) const = 0;

		bool HasEntries() const
		{
			return !m_Entries.empty();
		}
		const KPluginEntry::Vector& GetEntries() const
		{
			return m_Entries;
		}
		KPluginEntry::Vector& GetEntries()
		{
			return m_Entries;
		}

		virtual void Save() const = 0;
		virtual void Load() = 0;
		virtual void LoadNativeOrder() = 0;
		void Invalidate();

		virtual bool HasDependentPlugins(const KPluginEntry& pluginEntry) const = 0;
		virtual KPluginEntry::RefVector GetDependentPlugins(const KPluginEntry& pluginEntry) const = 0;
		virtual const KModEntry* FindParentMod(const KPluginEntry& pluginEntry) const = 0;

		bool IsValidModIndex(intptr_t modIndex) const;
		intptr_t GetPluginOrderIndex(const KPluginEntry& modEntry) const;
		virtual intptr_t GetPluginPriority(const KPluginEntry& modEntry) const = 0;
		virtual intptr_t GetPluginDisplayPriority(const KPluginEntry& modEntry) const
		{
			return GetPluginPriority(modEntry);
		}
		virtual wxString FormatPriority(const KPluginEntry& modEntry, intptr_t value) const
		{
			return modEntry.IsEnabled() ? wxString::Format("0x%02X (%d)", (int)value, (int)value) : wxEmptyString;
		}
		wxString FormatPriority(const KPluginEntry& modEntry) const
		{
			return FormatPriority(modEntry, GetPluginDisplayPriority(modEntry));
		}

		bool MovePluginsIntoThis(const KPluginEntry::RefVector& entriesToMove, const KPluginEntry& anchor, MoveMode moveMode = MoveMode::After);
		void SetAllPluginsEnabled(bool isEnabled);

		virtual bool IsPluginActive(const wxString& pluginName) const;
		virtual void SyncWithPluginsList(const KxStringVector& pluginNamesList, SyncListMode mode = SyncListMode::EnableAll);
		virtual KxStringVector GetPluginsList(bool activeOnly = false) const;
		KPluginEntry* FindPluginByName(const wxString& name) const;

		virtual bool CheckSortingTool(const KPluginManagerConfigSortingToolEntry& entry);
		virtual void RunSortingTool(const KPluginManagerConfigSortingToolEntry& entry);
};
