#pragma once
#include "stdafx.h"
#include "Events/KBroadcastEvent.h"
#include "KVariablesTable.h"
#include "KDynamicVariablesTable.h"
#include "KImageProvider.h"
#include "KProgramOptions.h"
#include <KxFramework/KxApp.h>
#include <KxFramework/KxImageList.h>
#include <KxFramework/KxImageSet.h>
#include <KxFramework/KxINI.h>
#include <wx/snglinst.h>
class KProfile;
class KLogEvent;
class KMainWindow;
class KModManager;
class KRunManager;
class KSettingsWindowManager;
class KVirtualFileSystemService;
class KCMConfigEntryStd;
class KIPCClient;
enum KPGCFileID;

class KApp: public KxApp<wxApp, KApp>
{
	public:
		static wxString ExpandVariablesUsing(const wxString& variables, const KVariablesTable& variablesTable);
		static wxString ExpandVariables(const wxString& variables);

	private:
		wxSingleInstanceChecker m_SingleInstanceChecker;
		KxTranslationTable m_AvailableTranslations;
		wxString m_LoadedTranslation;
		wxString m_UserSettingsFolder;
		wxWindow* m_InitProgressDialog = NULL;

		FILE* m_LogTargetFILE = NULL;
		wxLogStderr* m_LogTarget = NULL;

		KxImageList m_ImageList;
		KxImageSet m_ImageSet;

		KDynamicVariablesTable m_Variables;
		KProfile* m_CurrentProfile = NULL;
		KMainWindow* m_MainWindow = NULL;
		KRunManager* m_RunManager = NULL;
		KModManager* m_ModManager = NULL;
		mutable KSettingsWindowManager* m_SettingsManager = NULL;
		bool m_AllowSaveSettinsgAtExit = true;
		
		KIPCClient* m_VFSServiceClient = NULL;
		KVirtualFileSystemService* m_VFSService = NULL;
		std::vector<std::pair<wxEvtHandler*, wxEventType>> m_BroadcastingRecievers;
		bool m_BroadcastingEnabled = true;

		wxString m_CurrentTemplateID;
		wxString m_CurrentConfigID;

		KProgramOption m_GeneralOptions;
		KProgramOption m_GeneralOptions_AppWide;
		KProgramOption m_ProfileOptions_AppWide;

	public:
		KApp();
		virtual ~KApp();

	public:
		const KxImageList* GetImageList() const
		{
			return &m_ImageList;
		}
		const KxImageSet* GetImageSet() const
		{
			return &m_ImageSet;
		}
		wxLogStderr* GetLogTarget() const
		{
			return m_LogTarget;
		}

		const wxString& GetRootFolder() const;
		const wxString& GetDataFolder() const;
		const wxString& GetUserSettingsFolder();
		const wxString& GetUserSettingsFile();

		wxString ExpandVariablesLocally(const wxString& variables) const
		{
			return ExpandVariablesUsing(variables, m_Variables);
		}
		KProfile* GetCurrentProfile() const;
		KProfile* SetCurrentProfile(KProfile* profile);
		bool IsProfileLoaded() const
		{
			return GetCurrentProfile() != NULL;
		}
		bool IsTranslationLoaded() const
		{
			return !m_LoadedTranslation.IsEmpty() && !m_AvailableTranslations.empty();
		}
		const KxTranslationTable& GetAvailableTranslations() const
		{
			return m_AvailableTranslations;
		}

		KSettingsWindowManager* GetSettingsManager() const;
		void SaveSettings() const;
		bool ShowChageProfileDialog();

		KProgramOption& GetOptionsGeneral()
		{
			return m_GeneralOptions_AppWide;
		}
		KProgramOption& GetOptionsProfile()
		{
			return m_ProfileOptions_AppWide;
		}

		KCMConfigEntryStd* GetSettingsEntry(KPGCFileID id, const wxString& section, const wxString& name) const;
		wxString GetSettingsValue(KPGCFileID id, const wxString& section, const wxString& name) const;
		void SetSettingsValue(KPGCFileID id, const wxString& section, const wxString& name, const wxString& value);
		wxString GetCurrentTemplateID() const;
		wxString GetCurrentConfigID() const;
		void SetCurrentTemplateID(const wxString& templateID);
		void SetCurrentConfigID(const wxString& configID);
		void ConfigureInternetExplorer(bool init);

		KMainWindow* GetMainWindow() const
		{
			return m_MainWindow;
		}
		KIPCClient* GetVFSServiceClient() const
		{
			return m_VFSServiceClient;
		}
		KVirtualFileSystemService* GetVFSService() const
		{
			return m_VFSService;
		}
		KRunManager* GetRunManager() const
		{
			return m_RunManager;
		}

		void BroadcastEvent(KBroadcastEvent& event);
		void SubscribeBroadcasting(wxEvtHandler* pHandler, wxEventType type);
		void SubscribeBroadcasting(wxWindow* window, wxEventType type);
		void SetBroadcastingEnabled(bool isEnabled)
		{
			m_BroadcastingEnabled = isEnabled;
		}

		bool Uninstall();

	public:
		virtual bool OnInit() override;
		virtual int OnExit() override;
		virtual bool OnExceptionInMainLoop() override;
		virtual void OnUnhandledException() override;

	private:
		void LoadTranslation();
		void LoadImages();
		void ShowWorkspace();

		void InitSettings();
		bool IsPreStartConfigNeeded();
		bool ShowFirstTimeConfigDialog(wxWindow* parent);
		void InitProfilesData(wxWindow* parent);
		bool LoadProfile();
		void LoadCurrentTemplateAndConfig();

		void InitVFS();
		void UnInitVFS();
		void InitRunManager();
		void AddDownloadToAlreadyRunningInstance(const wxString& link);

	private:
		void OnErrorOccurred(KLogEvent& event);

		wxString GetLogsFolder();
		FILE* OpenLogFile();
		void CleanupLogs();
};

//////////////////////////////////////////////////////////////////////////
namespace
{
	template<class Type> wxString T_Fallback(const Type& id)
	{
		if constexpr(std::is_integral<Type>::value || std::is_enum<Type>::value)
		{
			return wxString::Format("$T(%d)", id);
		}
		else
		{
			return wxString::Format("$T(%s)", id);
		}
	}
}

template<class Type> wxString V(const Type& source)
{
	return KApp::Get().ExpandVariables(source);
}
template<class Type> wxString V(KProfile* profile, const Type& source)
{
	return profile->ExpandVariables(source);
}

template<class Type> wxString T(const Type& id)
{
	bool isSuccess = false;
	wxString out = KxTranslation::GetString(id, &isSuccess);
	if (isSuccess)
	{
		return V(out);
	}
	else
	{
		return T_Fallback(id);
	}
}
template<class Type> wxString T(KProfile* profile, const Type& id)
{
	bool isSuccess = false;
	wxString out = KxTranslation::GetString(id, &isSuccess);
	if (isSuccess)
	{
		return V(profile, out);
	}
	else
	{
		return T_Fallback(id);
	}
}

template<class Type, class ...Args> wxString T(const Type& id, Args... args)
{
	return wxString::Format(T(id), std::forward<Args>(args)...);
}
template<class Type, class ...Args> wxString T(KProfile* profile, const Type& id, Args... args)
{
	return wxString::Format(T(profile, id), std::forward<Args>(args)...);
}
