#pragma once
#include "stdafx.h"
#include "KManager.h"
#include "KModEntry.h"
#include "KFixedModEntry.h"
#include "KMandatoryModEntry.h"
#include "KModTagsManager.h"
#include "KDispatcher.h"
#include "KMirroredVirtualFolder.h"
#include "KNetworkConstants.h"
#include "KEvents.h"
#include "KProgramOptions.h"
#include <KxFramework/KxSingleton.h>
class KApp;
class KVFSService;
class KVFSConvergence;
class KVFSMirror;
class KVirtualFileSystemBase;
class KIPCClient;
class KxProgressDialog;

//////////////////////////////////////////////////////////////////////////
class KModManager: public KManager, public KxSingletonPtr<KModManager>
{
	friend class KIPCClient;
	friend class KApp;

	public:
		enum class MoveMode
		{
			Before,
			After,
		};

	private:
		using MandatotyModEntriesVector = std::vector<KMandatoryModEntry>;

	private:
		KProgramOptionUI m_Options;

		KModEntry::Vector m_ModEntries;
		MandatotyModEntriesVector m_ModEntry_Mandatory;
		KFixedModEntry m_ModEntry_BaseGame;
		KFixedModEntry m_ModEntry_WriteTarget;
		
		KModTagsManager m_TagManager;
		KDispatcher m_Dispatcher;
		bool m_IsMounted = false;
		KxProgressDialog* m_MountStatusDialog = NULL;

	private:
		void DoResortMods(const KProfile& profile);
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
		void OnInit() override;

		void OnModFilesChanged(KModEvent& event);
		void OnModInstalled(KModEvent& event);
		void OnModUninstalled(KModEvent& event);

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

		const KModEntry::Vector& GetEntries() const
		{
			return m_ModEntries;
		}
		KModEntry::Vector& GetEntries()
		{
			return m_ModEntries;
		}
		KModEntry::RefVector GetAllEntries(bool includeWriteTarget = false);

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

		void ResortMods(const KProfile& profile);
		void ResortMods();
		
		KModEntry* FindModByID(const wxString& modID, intptr_t* index = NULL) const;
		KModEntry* FindModByName(const wxString& modName, intptr_t* index = NULL) const;
		KModEntry* FindModBySignature(const wxString& signature, intptr_t* index = NULL) const;
		KModEntry* FindModByNetworkModID(KNetworkProviderID providerID, KNetworkModID id, intptr_t* index = NULL) const;
		
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

		intptr_t GetModOrderIndex(const KModEntry* modEntry) const;
		bool MoveModsIntoThis(const KModEntry::RefVector& entriesToMove, const KModEntry& anchor, MoveMode moveMode = MoveMode::After);

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

		void NotifyModInstalled(KModEntry& modEntry);
		void NotifyModUninstalled(const wxString& modID);
};
