#pragma once
#include "stdafx.h"
#include "KManager.h"
#include "KProgramEntry.h"
#include "KProgramOptions.h"
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxSingleton.h>
class KxMenu;
class KxMenuItem;
class KxProcess;

class KProgramManager: public KManager, public KxSingletonPtr<KProgramManager>
{
	friend class KMainWindow;
	friend class KProgramManagerModel;
	friend class KProgramManagerConfig;

	private:
		KProgramEntry::Vector m_DefaultPrograms;
		KProgramEntry::Vector m_UserPrograms;
		KProgramOptionUI m_Options;

	private:
		virtual void OnInit() override;
		void OnLoadConfig(const KxXMLNode& configNode);
		void LoadUserPrograms();
		void SaveUserPrograms() const;

		void LoadEntryImages(KProgramEntry& entry) const;
		bool CheckEntryImages(const KProgramEntry& entry) const;
		wxBitmap LoadEntryImage(const KProgramEntry& entry, bool smallBitmap) const;
		void OnAddMainMenuItems(KxMenu& menu);

		KxProcess& DoCreateProcess(const KProgramEntry& entry) const;
		int DoRunProcess(KxProcess& process) const;
		bool DoCheckEntry(const KProgramEntry& entry) const;

	public:
		KProgramManager();
		virtual ~KProgramManager();

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual wxString GetVersion() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_APPLICATION_RUN;
		}

		bool HasPrograms() const
		{
			return !m_UserPrograms.empty();
		}
		const KProgramEntry::Vector& GetProgramList() const
		{
			return m_UserPrograms;
		}
		KProgramEntry::Vector& GetProgramList()
		{
			return m_UserPrograms;
		}

	public:
		KProgramOption& GetOptions()
		{
			return m_Options;
		}

		virtual void Save() const override;
		virtual void Load() override;
		void LoadDefaultPrograms();

		// Created process can be run with either 'KxPROCESS_WAIT_SYNC' or 'KxPROCESS_WAIT_ASYNC' flag.
		// Or use 'RunProcess' to run it with default parameters.
		// If you didn't run it, call 'DestroyProcess'.
		KxProcess& CreateProcess(const KProgramEntry& entry) const;
		void DestroyProcess(KxProcess& process);
		int RunProcess(KxProcess& process) const;

		// Check entry paths and perform 'CreateProcess -> RunProcess' sequence on it.
		int RunEntry(const KProgramEntry& entry) const;
};
