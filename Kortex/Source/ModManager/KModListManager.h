#pragma once
#include "stdafx.h"
#include "KProgramOptions.h"
#include "KEvents.h"
#include <KxFramework/KxSingleton.h>
class KModEntry;
class KModManager;
class KPluginEntry;

class KModListMod
{
	public:
		KModEntry* m_ModEntry = NULL;
		bool m_IsEnabled = false;

	public:
		KModListMod(KModEntry* modEntry, bool enabled)
			:m_ModEntry(modEntry), m_IsEnabled(enabled)
		{
		}
		KModListMod(const wxString& signature, bool bEnabled);

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

class KModListPlugin
{
	public:
		wxString m_PluginName;
		bool m_IsEnabled = false;

	public:
		KModListPlugin(KPluginEntry* pluginEntry, bool enabled);
		KModListPlugin(const wxString& name, bool enabled);

	public:
		bool IsOK() const
		{
			return !m_PluginName.IsEmpty();
		}
		
		const wxString& GetPluginName() const
		{
			return m_PluginName;
		}
		KPluginEntry* GetPluginEntry() const;
		
		bool IsEnabled() const
		{
			return m_IsEnabled;
		}
};

//////////////////////////////////////////////////////////////////////////
class KModList
{
	friend class KModListManager;

	private:
		struct LocalFolderNames
		{
			constexpr static const auto Overwrites = wxS("LocalOverwrites");
			constexpr static const auto Saves = wxS("LocalSaves");
			constexpr static const auto Config = wxS("LocalConfig");
		};
		struct GlobalFolderNames
		{
			constexpr static const auto Saves = wxS("GlobalSaves");
			constexpr static const auto Config = wxS("GlobalConfig");
		};

	public:
		using ModEntryVector = std::vector<KModListMod>;
		using PluginEntryVector = std::vector<KModListPlugin>;

	private:
		wxString m_ID;
		bool m_LocalSavesEnabled = false;
		bool m_LocalConfigEnabled = false;

		ModEntryVector m_Mods;
		PluginEntryVector m_Plugins;

	private:
		static wxString CreateSignature(const wxString& listID);

		static wxString GetGlobalFolderPath(const wxString& folderName);
		static wxString GetLocalRootPath(const wxString& listID);
		static wxString GetLocalFolderPath(const wxString& listID, const wxString& folderName);

		static bool RemoveLocalRoot(const wxString& listID);
		static bool CreateLocalRoot(const wxString& listID);
		static bool CreateLocalFolder(const wxString& listID, const wxString& folderName);
		static bool RemoveLocalFolder(const wxString& listID, const wxString& folderName);
		static bool RenameLocalRoot(const wxString& oldID, const wxString& newID, wxString* newPathOut = NULL);
		
		wxString GetLocalRootPath() const
		{
			return GetLocalRootPath(m_ID);
		}
		wxString GetLocalFolderPath(const wxString& folderName) const
		{
			return GetLocalFolderPath(m_ID, folderName);
		}
		bool RemoveLocalRoot()
		{
			return RemoveLocalRoot(m_ID);
		}
		void OnRemove();

	public:
		KModList(const wxString& id)
			:m_ID(id)
		{
		}

	public:
		bool IsCurrentList() const;

		wxString GetSignature() const
		{
			return CreateSignature(m_ID);
		}
		const wxString& GetID() const
		{
			return m_ID;
		}
		bool SetID(const wxString& id);

		bool IsLocalSavesEnabled() const
		{
			return m_LocalSavesEnabled;
		}
		void SetLocalSavesEnabled(bool value);

		bool IsLocalConfigEnabled() const
		{
			return m_LocalConfigEnabled;
		}
		void SetLocalConfigEnabled(bool value);

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

class KModListManager: public KxSingletonPtr<KModListManager>
{
	friend class KModManager;
	friend class KModList;

	using LocalFolderNames = KModList::LocalFolderNames;
	using GlobalFolderNames = KModList::GlobalFolderNames;

	public:
		using ListVector = std::vector<KModList>;

	private:
		KProgramOptionUI m_Options;

		ListVector m_Lists;
		wxString m_CurrentListID;

	private:
		template<class T> static auto FindModListIterator(T& lists, const wxString& id)
		{
			const wxString uid = KModList::CreateSignature(id);
			return std::find_if(lists.begin(), lists.end(), [&uid](const KModList& modList)
			{
				return KModList::CreateSignature(modList.GetID()) == uid;
			});
		}

		void DoChangeCurrentListID(const wxString& id);
		void ClearLists()
		{
			m_Lists.clear();
		}
		wxString CreateListName(size_t pos) const;

		void SetupGlobalFolders();
		void OnModListSelected(KModListEvent& event);
		void OnInit();

	public:
		KModListManager();
		virtual ~KModListManager();

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
			return const_cast<KModListManager*>(this)->GetCurrentList();
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
		bool HasList(const wxString& id) const
		{
			return FindModList(id) != NULL;
		}

		const KModList* FindModList(const wxString& id) const
		{
			auto it = FindModListIterator(m_Lists, id);
			return it != m_Lists.end() ? &*it : NULL;
		}
		KModList* FindModList(const wxString& id)
		{
			auto it = FindModListIterator(m_Lists, id);
			return it != m_Lists.end() ? &*it : NULL;
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

		void LoadLists();
		void SaveLists();

		KModList& CreateNewList(const wxString& id);
		KModList& CreateListCopy(const KModList& list, const wxString& newID);
		KModList* RenameList(const wxString& oldID, const wxString& newID);
		
		bool RemoveList(const wxString& id);
		bool RemoveList(const KModList& list)
		{
			return RemoveList(list.GetID());
		}
};
