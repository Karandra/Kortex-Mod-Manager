#pragma once
#include "stdafx.h"
#include "KManager.h"
#include "KWithBitmap.h"
#include "KProgramOptions.h"
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxSingleton.h>
class KProgramManagerConfig;
class KProgramManagerModel;
class KVFSEvent;
class KxMenu;
class KxMenuItem;
class KxProgressDialog;
class KxProcess;

class KProgramManagerEntry: public KWithBitmap
{
	private:
		wxString m_Name;
		wxString m_IconPath;
		wxString m_Executable;
		wxString m_Arguments;
		wxString m_WorkingDirectory;
		bool m_RequiresVFS = true;

	public:
		KProgramManagerEntry();
		KProgramManagerEntry(const KxXMLNode& node);

	public:
		bool IsOK() const;

		const wxString& GetName() const
		{
			return m_Name;
		}
		void SetName(const wxString& value)
		{
			m_Name = value;
		}
		
		const wxString& GetIconPath() const
		{
			return m_IconPath;
		}
		void SetIconPath(const wxString& value)
		{
			m_IconPath = value;
		}
		
		const wxString& GetExecutable() const
		{
			return m_Executable;
		}
		void SetExecutable(const wxString& value)
		{
			m_Executable = value;
		}
		
		const wxString& GetArguments() const
		{
			return m_Arguments;
		}
		void SetArguments(const wxString& value)
		{
			m_Arguments = value;
		}

		const wxString& GetWorkingDirectory() const
		{
			return m_WorkingDirectory;
		}
		void SetWorkingDirectory(const wxString& value)
		{
			m_WorkingDirectory = value;
		}

		bool CalcRequiresVFS() const;
		bool IsRequiresVFS() const
		{
			return m_RequiresVFS;
		}
		void SetRequiresVFS(bool value)
		{
			m_RequiresVFS = value;
		}
};
typedef std::vector<KProgramManagerEntry> KRMProgramEntryArray;

//////////////////////////////////////////////////////////////////////////
class KProgramManager: public KManager, public KxSingletonPtr<KProgramManager>
{
	friend class KProgramManagerModel;

	public:
		static wxString GetProgramsListFile(const wxString& templateID, const wxString& configID);

	public:
		static const int ms_DefaultPreMainInterval = 100;
		static const int ms_DefaultPreMainTimeout = 500;

	public:
		static const KProgramManagerConfig* GetRunConfig();
		static wxString GetKExecutePath();
		static void InitKExecute(KxProcess& process, const wxString& executable, const wxString& arguments = wxEmptyString, const wxString& workingDirectory = wxEmptyString);
		static void InitKExecute(KxProcess& process, const KProgramManagerEntry& runEntry);

	private:
		KProgramOptionUI m_RunMainOptions;

		const KProgramManagerConfig* m_RunConfig = NULL;
		KRMProgramEntryArray m_ProgramList;

		KxMenu* m_Menu = NULL;
		std::vector<KxMenuItem*> m_MenuItems;
		bool m_IconsExtracted = false;

	private:
		virtual wxBitmap OnQueryItemImage(const KProgramManagerEntry& runEntry) const;
		void OnVFSToggled(KVFSEvent& event);
		void OnMenuOpen(KxMenuEvent& event);

		// If 'processOut' not NULL, then created process object will not be run in this function
		// and will be returned in provided pointer.
		void DoRunEntry(const KProgramManagerEntry& runEntry, KxProgressDialog* dialog, KxProcess** processOut = NULL);
		bool CheckEntry(const KProgramManagerEntry& runEntry);
		
		KxProgressDialog* BeginRunProcess();
		void EndRunProcess(KxProgressDialog* dialog, KxProcess* process);
		int GetPreMainInterval() const;
		int GetPreMainTimeout() const;
		KxStringVector CheckPreMain();
		void RunPreMain(KxProgressDialog* dialog);
		void RunMain(KxProgressDialog* dialog, const KProgramManagerEntry& runEntry);

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
			return !m_ProgramList.empty();
		}
		const KRMProgramEntryArray& GetProgramList() const
		{
			return m_ProgramList;
		}
		KRMProgramEntryArray& GetProgramList()
		{
			return m_ProgramList;
		}

		wxString GetProgramsListFile() const;
		void UpdateProgramListImages();

	public:
		virtual void OnAddMenuItems(KxMenu* menu);
		virtual void OnRunEntry(KxMenuItem* menuItem, const KProgramManagerEntry& runEntry);

	public:
		KProgramOption& GetOptions()
		{
			return m_RunMainOptions;
		}

		void LoadProgramList();
		void SaveProgramList() const;

		// Never destroy provided dialog yourself!
		bool RunEntry(const KProgramManagerEntry& runEntry, KxProgressDialog* dialog = NULL);
		
		// Returned process can be run with either 'KxPROCESS_WAIT_SYNC' or 'KxPROCESS_WAIT_ASYNC' flag.
		// If you bind 'wxEVT_END_PROCESS' handler to the process make sure you wxEvent::Skip() it!
		KxProcess* RunEntryDelayed(const KProgramManagerEntry& runEntry, KxProgressDialog* dialog = NULL);
};
