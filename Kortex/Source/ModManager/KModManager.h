#pragma once
#include "stdafx.h"
#include "KManager.h"
#include "KModEntry.h"
#include "KFixedModEntry.h"
#include "KModTagsManager.h"
#include "KModManagerDispatcher.h"
#include "KModListManager.h"
#include "KMirroredVirtualFolder.h"
#include "KNetworkConstants.h"
#include "KEvents.h"
#include "KProgramOptions.h"
#include <KxFramework/KxSingleton.h>
class KVFSService;
class KVFSConvergence;
class KVFSMirror;
class KVirtualFileSystemBase;
class KIPCClient;
class KxProgressDialog;

enum KModManagerLocation
{
	KMM_LOCATION_MODS_ORDER,
	KMM_LOCATION_MODS_FOLDER,

	KMM_LOCATION_MOD_ROOT,
	KMM_LOCATION_MOD_INFO,
	KMM_LOCATION_MOD_FILES,
	KMM_LOCATION_MOD_FILES_DEFAULT,
	KMM_LOCATION_MOD_LOGO,
	KMM_LOCATION_MOD_DESCRIPTION,
};
enum KModManagerModsMoveType
{
	KMM_MOVEMOD_BEFORE,
	KMM_MOVEMOD_AFTER,
};

//////////////////////////////////////////////////////////////////////////
class KModManager: public KManager, public KxSingletonPtr<KModManager>
{
	friend class KIPCClient;

	public:
		static KModManager& Get()
		{
			return *GetInstance();
		}
		static KModTagsManager& GetTagManager()
		{
			return Get().m_TagManager;
		}
		static KModManagerDispatcher& GetDispatcher()
		{
			return Get().m_Dispatcher;
		}
		static KModListManager& GetListManager()
		{
			return Get().m_ModListManager;
		}

	public:
		/*
			This function works only for:
			- KMM_LOCATION_MODS_ORDER
			- KMM_LOCATION_MODS_FOLDER
			- KMM_LOCATION_MOD_ROOT

			To use any other constant use function KModEntry::GetLocation.
		*/
		static wxString GetLocation(KModManagerLocation nLocation, const wxString& signature = wxEmptyString);

	private:
		using MandatotyModEntriesVector = std::vector<KFixedModEntry>;

	private:
		KProgramOptionUI m_Options;

		KModEntryArray m_ModEntries;
		MandatotyModEntriesVector m_ModEntry_Mandatory;
		KFixedModEntry m_ModEntry_BaseGame;
		KFixedModEntry m_ModEntry_WriteTarget;
		
		KModTagsManager m_TagManager;
		KModManagerDispatcher m_Dispatcher;
		KModListManager m_ModListManager;
		bool m_IsMounted = false;
		KxProgressDialog* m_MountStatusDialog = NULL;

	private:
		void SortEntries();
		void SetMounted(bool value)
		{
			m_IsMounted = value;
		}
		void DoUninstallMod(KModEntry* modEntry, bool erase, wxWindow* window = NULL);

		bool InitMainVirtualFolder();
		bool InitMirroredLocations();

		void CreateMountStatusDialog();
		void DestroyMountStatusDialog();

		bool CheckMountPoint(const wxString& folderPath);
		void ReportNonEmptyMountPoint(const wxString& folderPath);

	private:
		void OnModFilesChanged(KModEvent& event);
		void OnModToggled(KModEvent& event);
		void OnModsReordered(KModEvent& event);
		
		void OnModInstalled(KModEvent& event);
		void OnModUninstalled(KModEvent& event);

		void OnModListSelected(KModListEvent& event);

	public:
		KModManager(KWorkspace* workspace);
		void Clear();
		virtual ~KModManager();

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual wxString GetVersion() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_PUZZLE;
		}
		virtual KWorkspace* GetWorkspace() const override;
		virtual void SetWorkspace(KWorkspace* workspace)
		{
		}

	public:
		KProgramOption& GetOptions()
		{
			return m_Options;
		}

		const KModEntryArray& GetEntries() const
		{
			return m_ModEntries;
		}
		KModEntryArray& GetEntries()
		{
			return m_ModEntries;
		}
		KModEntryArray GetAllEntries(bool includeWriteTarget = false);

		MandatotyModEntriesVector& GetModEntry_Mandatory()
		{
			return m_ModEntry_Mandatory;
		}
		KModEntry* GetModEntry_BaseGame()
		{
			return &m_ModEntry_BaseGame;
		}
		KModEntry* GetModEntry_WriteTarget()
		{
			return &m_ModEntry_WriteTarget;
		}

		virtual void Load() override;
		virtual void Save() const override;
		bool ChangeModListAndResort(const wxString& newModListID);
		
		KModEntry* FindModByID(const wxString& modID) const;
		KModEntry* FindModByName(const wxString& modName) const;
		KModEntry* FindModBySignature(const wxString& signature) const;
		KModEntry* FindModByNetworkModID(KNetworkProviderID providerID, KNetworkModID id) const;
		
		bool IsModActive(const wxString& modID) const;
		void UninstallMod(KModEntry* entry, wxWindow* window = NULL)
		{
			DoUninstallMod(entry, false, window);
		}
		void EraseMod(KModEntry* entry, wxWindow* window = NULL)
		{
			DoUninstallMod(entry, true, window);
		}
		bool ChangeModID(KModEntry* entry, const wxString& newID);

		intptr_t GetModIndex(const KModEntry* modEntry) const;
		bool MoveModsIntoThis(const KModEntryArray& entriesToMove, const KModEntry* anchor, KModManagerModsMoveType moveMode = KMM_MOVEMOD_AFTER);

		wxString GetVirtualGameRoot() const;
		KxProgressDialog* GetMountStatusDialog() const
		{
			return m_MountStatusDialog;
		}
		
		bool IsVFSMounted() const
		{
			return m_IsMounted;
		}
		void MountVFS();
		void UnMountVFS();

		void ExportModList(const wxString& outputFilePath) const;
};
