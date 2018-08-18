#pragma once
#include "stdafx.h"
#include "KProgramOptions.h"
class KModEntry;
class KModManager;
class KPMPluginEntry;

class KModListModEntry
{
	public:
		KModEntry* m_ModEntry = NULL;
		bool m_IsEnabled;

	public:
		KModListModEntry(KModEntry* modEntry, bool enabled)
			:m_ModEntry(modEntry), m_IsEnabled(enabled)
		{
		}
		KModListModEntry(const wxString& signature, bool bEnabled);

	public:
		bool IsOK() const
		{
			return m_ModEntry != NULL;
		}
		
		KModEntry* GetMod() const
		{
			return m_ModEntry;
		}
		bool IsEnabled() const
		{
			return m_IsEnabled;
		}
};

class KModListPluginEntry
{
	public:
		wxString m_PluginName;
		bool m_IsEnabled;

	public:
		KModListPluginEntry(KPMPluginEntry* pluginEntry, bool enabled);
		KModListPluginEntry(const wxString& name, bool enabled);

	public:
		bool IsOK() const
		{
			return !m_PluginName.IsEmpty();
		}
		
		const wxString& GetPluginName() const
		{
			return m_PluginName;
		}
		KPMPluginEntry* GetPluginEntry() const;
		
		bool IsEnabled() const
		{
			return m_IsEnabled;
		}
};

class KModList
{
	public:
		using ModEntryVector = std::vector<KModListModEntry>;
		using PluginEntryVector = std::vector<KModListPluginEntry>;

	private:
		wxString m_ID;
		ModEntryVector m_Mods;
		PluginEntryVector m_Plugins;

	public:
		KModList(const wxString& id)
			:m_ID(id)
		{
		}

	public:
		const wxString& GetID() const
		{
			return m_ID;
		}
		void SetID(const wxString& id);

		const ModEntryVector& GetMods() const
		{
			return m_Mods;
		}
		ModEntryVector& GetMods()
		{
			return m_Mods;
		}

		const PluginEntryVector& GetPlugins() const
		{
			return m_Plugins;
		}
		PluginEntryVector& GetPlugins()
		{
			return m_Plugins;
		}
};

class KModManagerModList
{
	friend class KModManager;
	friend class KModList;

	public:
		using ListVector = std::vector<KModList>;

	private:
		KProgramOptionUI m_Options;

		ListVector m_Lists;
		wxString m_CurrentListID;

	private:
		ListVector::iterator FindModListIterator(const wxString& id)
		{
			return std::find_if(m_Lists.begin(), m_Lists.end(), [&id](const KModList& v)
			{
				return v.GetID() == id;
			});
		}
		KModList* FindModList(const wxString& id)
		{
			auto it = FindModListIterator(id);
			return it != m_Lists.end() ? &*it : NULL;
		}
		
		void DoChangeCurrentListID(const wxString& id);
		void UpdateWriteTargetLocation();
		void DoRenameList(KModList& list, const wxString& newID);

		wxString CreateListName(size_t pos) const;

	public:
		KModManagerModList();
		virtual ~KModManagerModList();

	public:
		KProgramOption& GetOptions()
		{
			return m_Options;
		}
		wxString GetDefaultListID() const
		{
			return "Default";
		}
		
		const ListVector& GetLists() const
		{
			return m_Lists;
		}
		ListVector& GetLists()
		{
			return m_Lists;
		}
		
		const KModList& GetCurrentList() const
		{
			return const_cast<KModManagerModList*>(this)->GetCurrentList();
		}
		KModList& GetCurrentList()
		{
			KModList* pModList = FindModList(m_CurrentListID);

			// Default list must always exist
			return pModList ? *pModList : *FindModList(GetDefaultListID());
		}
		
		size_t GetListsCount() const
		{
			return m_Lists.size();
		}
		bool HasLists() const
		{
			return !m_Lists.empty();
		}
		bool HasList(const wxString& sID) const
		{
			return const_cast<KModManagerModList*>(this)->FindModList(sID) != NULL;
		}

		bool IsCurrentListID(const wxString& id) const
		{
			return m_CurrentListID == id;
		}
		const wxString& GetCurrentListID() const
		{
			return m_CurrentListID;
		}
		bool SetCurrentListID(const wxString& id);
		bool SetCurrentListID(const KModList& list)
		{
			DoChangeCurrentListID(list.GetID());
			return true;
		}
		
		bool SyncList(const wxString& id);
		bool SyncCurrentList()
		{
			return SyncList(m_CurrentListID);
		}

		void ClearLists()
		{
			m_Lists.clear();
		}
		void ReloadLists();
		void SaveLists();

		KModList& CreateNewList(const wxString& id);
		KModList& CreateListCopy(const KModList& list, const wxString& newID);
		KModList* RenameList(const wxString& oldID, const wxString& newID);
		bool RemoveList(const wxString& id);
		bool RemoveList(const KModList& list)
		{
			return RemoveList(list.GetID());
		}

		wxString GetWriteTargetName(const wxString& id) const;
		wxString GetWriteTargetFullPath(const wxString& id) const;
};
