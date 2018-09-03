#include "stdafx.h"
#include "KApp.h"
#include "KVariablesDatabase.h"
#include "KAux.h"
#include "KThemeManager.h"
#include "Themes/KThemeDefault.h"
#include "Themes/KThemeVisualStudio.h"
#include "Profile/KProfile.h"
#include "Events/KLogEvent.h"
#include "UI/KMainWindow.h"
#include "UI/KWorkspace.h"
#include "UI/KProfileSelectionDialog.h"
#include "SettingsWindow/KSettingsWindowManager.h"
#include "ConfigManager/KConfigManager.h"
#include "ConfigManager/KCMConfigEntry.h"
#include "VFS/KVirtualFileSystemService.h"
#include "ProgramManager/KProgramManager.h"
#include "ModManager/KModManager.h"
#include "ModManager/KModManagerWorkspace.h"
#include "DownloadManager/KDownloadManager.h"
#include "Archive/KArchive.h"
#include "IPC/KIPCClient.h"
#include "KINetFSHandler.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxProgressDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxTextBoxDialog.h>
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxRegistry.h>
#include <KxFramework/KxSplashWindow.h>
#include <KxFramework/KxTaskScheduler.h>

wxString KApp::ExpandVariablesUsing(const wxString& variables, const KVariablesTable& variablesTable)
{
	return variablesTable.Expand(variables);
}
wxString KApp::ExpandVariables(const wxString& variables)
{
	if (Get().GetCurrentProfile())
	{
		return ExpandVariablesUsing(Get().ExpandVariablesLocally(variables), Get().GetCurrentProfile()->GetVariables());
	}
	return Get().ExpandVariablesLocally(variables);
}

KApp::KApp()
	:m_ImageList(16, 16, false, KIMG_COUNT),
	m_GeneralOptions(KPGC_ID_CURRENT_PROFILE, "KApp", "General"),
	m_GeneralOptions_AppWide(KPGC_ID_APP, "KApp", "General"),
	m_ProfileOptions_AppWide(KPGC_ID_APP, "KApp", "Profiles")
{
	// Set app info
	SetAppName("KortexModManager");
	SetAppDisplayName("Kortex Mod Manager");
	SetAppVersion("1.4");
	SetVendorName("Kerber");

	m_Variables.SetVariable("AppName", GetAppDisplayName());
	m_Variables.SetVariable("AppVersion", GetAppVersion());
	m_Variables.SetVariable("AppVendor", GetVendorDisplayName());
	m_Variables.SetVariable("AppGUID", "{B5E8047C-9239-45C4-86F6-6C83A842063E}");
	m_Variables.SetVariable("AppModProjectProgID", "KMM.ModProject.1");
	m_Variables.SetVariable("AppModPackageProgID", "KMM.ModPackage.1");
	m_Variables.SetVariable("AppData", GetDataFolder());
	m_Variables.SetVariable("SystemArchitecture", KAux::ArchitectureToNumber(KxSystem::Is64Bit()));
	m_Variables.SetVariable("SystemArchitectureName", KAux::ArchitectureToString(KxSystem::Is64Bit()));

	// Configure command line parsing options
	wxCmdLineParser& cmdLineParser = GetCmdLineParser();
	cmdLineParser.SetSwitchChars("-");
	cmdLineParser.AddOption("ProfileID", wxEmptyString, "Profile ID");
	cmdLineParser.AddOption("ConfigID", wxEmptyString, "Profile configuration ID");
	cmdLineParser.AddOption("GlobalConfigPath", wxEmptyString, "Folder path for app-wide config");

	cmdLineParser.AddOption("NXM", wxEmptyString, "Nexus link");
}
KApp::~KApp()
{
}

const wxString& KApp::GetRootFolder() const
{
	static const wxString ms_RootFolder = KxLibrary(NULL).GetFileName().BeforeLast('\\');
	return ms_RootFolder;
}
const wxString& KApp::GetDataFolder() const
{
	static const wxString ms_DataFolder = GetRootFolder() + "\\Data";
	return ms_DataFolder;
}
const wxString& KApp::GetUserSettingsFolder()
{
	if (m_UserSettingsFolder.IsEmpty())
	{
		m_UserSettingsFolder = KxShell::GetFolder(KxSHF_APPLICATIONDATA_LOCAL) + '\\' + GetAppName();
	}
	return m_UserSettingsFolder;
}
const wxString& KApp::GetUserSettingsFile()
{
	static const wxString ms_UserSettingsFile = GetUserSettingsFolder() + "\\Settings.ini";
	return ms_UserSettingsFile;
}

KProfile* KApp::GetCurrentProfile() const
{
	return m_CurrentProfile;
}
KProfile* KApp::SetCurrentProfile(KProfile* profile)
{
	delete m_CurrentProfile;
	m_CurrentProfile = profile;

	return profile;
}

bool KApp::OnInit()
{
	// Set CWD
	KxFile::SetCWD(GetRootFolder());

	// Init global settings folder
	const bool anotherInstanceRunning = m_SingleInstanceChecker.IsAnotherRunning();
	ParseCommandLine();
	GetCmdLineParser().Found("GlobalConfigPath", &m_UserSettingsFolder);
	m_Variables.SetVariable("AppSettings", GetUserSettingsFolder());

	// Setup logging
	m_LogTargetFILE = OpenLogFile();
	m_LogTarget = new wxLogStderr(m_LogTargetFILE);
	wxLog::SetActiveTarget(m_LogTarget);
	wxLog::SetVerbose(true);
	Bind(KEVT_LOG, &KApp::OnErrorOccurred, this);

	// Check download
	wxString downloadLink;
	if (!KDownloadManager::CheckCmdLineArgs(GetCmdLineParser(), downloadLink))
	{
		downloadLink.clear();
	}

	// Show loading window
	LoadImages();
	KThemeManager::Set(new KThemeDefault());
	KxSplashWindow splashWindow(NULL, m_ImageSet.GetBitmap("application-logo"));
	
	// Don't show loading screen if it's a download link request
	if (downloadLink.IsEmpty() || !anotherInstanceRunning)
	{
		splashWindow.Show();
	}
	m_InitProgressDialog = &splashWindow;

	// Log system info
	wxLogInfo("%s %s: Log opened", GetAppDisplayName(), GetAppVersion());
	KxSystem::VersionInfo tVI = KxSystem::GetVersionInfo();
	wxLogInfo("System: %s %s %s. Kernel version: %d.%d", KxSystem::GetName(), KxSystem::Is64Bit() ? "x64" : "x86", tVI.ServicePack, tVI.MajorVersion, tVI.MinorVersion);

	wxFileSystem::AddHandler(new KINetFSHandler());

	// Create user settings folder if needed and load app config
	wxLogInfo("Settings directory: %s", GetUserSettingsFolder());
	KxFile(GetUserSettingsFolder()).CreateFolder();
	m_SettingsManager = new KSettingsWindowManager(NULL);
	m_SettingsManager->InitAppConfig();

	// Translation loader depends on settings manager, so load it right after it
	LoadTranslation();

	if (!anotherInstanceRunning)
	{
		wxSystemOptions::SetOption("KxDataViewCtrl::DefaultRowHeight", m_InitProgressDialog->FromDIP(19));

		// Init systems
		wxLogInfo("Begin initializing core systems");
		InitSettings();
		InitVFS();
		InitProgramManager();
		KArchive::Init();
		wxLogInfo("Core systems initialized");

		// All required managers initialized, can create main window now
		wxLogInfo("Creating main window");
		m_MainWindow = new KMainWindow();
		SetTopWindow(m_MainWindow);
		wxWindowUpdateLocker lockMainWindow(m_MainWindow);

		// Only after main window is created settings manager can load its controller data
		m_SettingsManager->InitControllerData();

		// Show main window and selected workspace
		wxLogInfo("Main window created. Showing workspace.");
		ShowWorkspace();
		m_MainWindow->Show();
		return true;
	}
	else
	{
		// Send download
		if (!downloadLink.IsEmpty())
		{
			AddDownloadToAlreadyRunningInstance(downloadLink);
			return false;
		}

		KLogEvent(T("Init.AnotherInstanceRunning"), KLOG_ERROR);
		return false;
	}
	m_InitProgressDialog = NULL;
}
int KApp::OnExit()
{
	wxLog::FlushActive();
	wxLogInfo("Exiting app");

	// VFS should be uninitialized first
	UnInitVFS();

	// Destroy other managers
	delete m_ProgramManager;
	delete m_CurrentProfile;
	KThemeManager::Cleanup();
	KArchive::UnInit();

	// Save all settings and destroy settings manager
	if (m_AllowSaveSettinsgAtExit)
	{
		SaveSettings();
	}
	delete m_SettingsManager;

	wxLogInfo("Log closed");
	CleanupLogs();
	return KxApp::OnExit();
}
bool KApp::OnExceptionInMainLoop()
{
	if (wxIsDebuggerRunning())
	{
		throw;
		return false;
	}

	wxString message = "Unknown";
	wxString origin = "Unknown";
	try
	{
		throw;
	}
	catch (const std::exception& e)
	{
		origin = "C++ Standard Library";
		message = e.what();
	}
	catch (const std::string& e)
	{
		origin = "C++ std::string";
		message = e;
	}
	catch (const std::wstring& e)
	{
		origin = "C++ std::wstring";
		message = e;
	}
	catch (const char* e)
	{
		origin = "C Narrow String";
		message = e;
	}
	catch (const wchar_t* e)
	{
		origin = "C Wide String";
		message = e;
	}
	catch (...)
	{
		message = "unknown error.";
	}
	KLogEvent(wxString::Format("Unexpected exception has occurred: %s.\r\n\r\nThe program will terminate.\r\n\r\nException origin: %s", message, origin), KLOG_ERROR);

	// Exit the main loop and terminate the program.
	return false;
}
void KApp::OnUnhandledException()
{
	OnExceptionInMainLoop();
	ExitApp(std::numeric_limits<int>::min());
}

void KApp::LoadTranslation()
{
	m_AvailableTranslations = KxTranslation::FindTranslationsInDirectory(GetDataFolder() + "\\Lang");
	auto LoadLang = [this](const wxString& locale) -> bool
	{
		auto it = m_AvailableTranslations.find(locale);
		if (it != m_AvailableTranslations.end())
		{
			if (m_Translation.LoadFromFile(it->second))
			{
				KxTranslation::SetCurrent(m_Translation);
				m_LoadedTranslationLocale = locale;
				m_GeneralOptions_AppWide.SetAttribute("Language", locale);
				return true;
			}
		}
		return false;
	};
	
	if (!m_AvailableTranslations.empty())
	{
		// Try load saved translation
		wxString selectedLang = m_GeneralOptions_AppWide.GetAttribute("Language");
		if (LoadLang(selectedLang))
		{
			return;
		}

		// Try default locales
		if (LoadLang(KxTranslation::GetUserDefaultLocale()) ||
			LoadLang(KxTranslation::GetSystemPreferredLocale()) ||
			LoadLang(KxTranslation::GetSystemDefaultLocale()) || 
			LoadLang("en-US"))
		{
			return;
		}

		// Try first available
		const auto& first = *m_AvailableTranslations.begin();
		if (LoadLang(first.first))
		{
			return;
		}
		KLogEvent(wxString::Format("Can't load translation from file \"%s\".\r\n\r\nTerminating.", first.second), KLOG_CRITICAL).Send();
	}
	else
	{
		KLogEvent("No translations found. Terminating.", KLOG_CRITICAL).Send();
	}
}
void KApp::LoadImages()
{
	KImageProvider::KLoadImages(m_ImageList, m_ImageSet);
}
void KApp::ShowWorkspace()
{
	wxString startPage = m_GeneralOptions.GetAttribute("Workspace", KModManagerWorkspace::GetInstance()->GetID());
	wxLogInfo("Start page is: %s", startPage);

	KWorkspace* workspace = m_MainWindow->GetWorkspace(startPage);
	if (!workspace || !workspace->CanBeStartPage() || !workspace->SwitchHere())
	{
		wxLogInfo("Can't show workspace %s. Trying first available", startPage);

		bool isSuccess = false;
		for (const auto& v: m_MainWindow->GetWorkspacesList())
		{
			workspace = v.second;
			if (workspace->CanBeStartPage())
			{
				wxLogInfo("Trying to load %s workspace", workspace->GetID());
				isSuccess = workspace->SwitchHere();
				break;
			}
		}

		if (isSuccess)
		{
			wxLogInfo("Successfully showed %s workspace", startPage);
		}
		else
		{
			wxLogInfo("No workspaces available. Terminating");
			KLogEvent(T("Init.Error3"), KLOG_CRITICAL, m_MainWindow).Send();
		}
	}
	else
	{
		wxLogInfo("Successfully showed %s workspace", startPage);
	}
}

void KApp::InitSettings()
{
	wxLogInfo("Initializing app settings");

	// Init some application-wide variables
	m_Variables.SetVariable(KVAR_PROFILES_ROOT, m_ProfileOptions_AppWide.GetAttribute("Location", GetUserSettingsFolder()));

	// Show first time config dialog if needed and save new 'ProfilesFolder'
	if (IsPreStartConfigNeeded())
	{
		wxLogInfo("Pre start config needed");
		if (!ShowFirstTimeConfigDialog(m_InitProgressDialog))
		{
			KLogEvent(T("Init.Error1"), KLOG_CRITICAL).Send();
			return;
		}
	}

	m_ProfileOptions_AppWide.SetAttribute("Location", ExpandVariables(KVAR(KVAR_PROFILES_ROOT)));
	wxLogInfo("Profiles folder: %s", m_ProfileOptions_AppWide.GetAttribute("Location"));
	SaveSettings();

	// Init all profiles and load current one if specified (or ask user to choose it)
	wxLogInfo("Settings initialized. Begin loading profile.");
	m_InitProgressDialog->SetLabel(T("Init.Status2"));
	LoadCurrentTemplateAndConfig();
	InitProfilesData(m_InitProgressDialog);

	// All done, close the dialog now
	m_InitProgressDialog->SetLabel(T("Init.StatusDone"));
	m_InitProgressDialog->Close();

	ConfigureInternetExplorer(true);
}
bool KApp::IsPreStartConfigNeeded()
{
	wxString profilesDataPath = m_ProfileOptions_AppWide.GetAttribute("Location");
	return profilesDataPath.IsEmpty() || !KxFile(profilesDataPath).IsFolderExist();
}
bool KApp::ShowFirstTimeConfigDialog(wxWindow* parent)
{
	wxString defaultPath = GetUserSettingsFolder();
	wxString message = wxString::Format("%s\r\n\r\n%s: %s", T("Init.ProfilesPath2"), T("Generic.DefaultValue"), defaultPath);
	KxTaskDialog dialog(parent, KxID_NONE, T("Init.ProfilesPath1"), message, KxBTN_NONE);
	dialog.AddButton(KxID_YES, T("Generic.UseDefaultValue"));
	dialog.AddButton(KxID_NO, T("Generic.SelectFolder"));

	if (dialog.ShowModal() == KxID_YES)
	{
		m_Variables.SetVariable(KVAR_PROFILES_ROOT, defaultPath);
		return true;
	}
	else
	{
		KxFileBrowseDialog folderDialog(&dialog, KxID_NONE, KxFBD_OPEN_FOLDER);
		folderDialog.SetFolder(defaultPath);
		if (folderDialog.ShowModal() == KxID_OK)
		{
			m_Variables.SetVariable(KVAR_PROFILES_ROOT, folderDialog.GetResult());
			return true;
		}
		return false;
	}
}
void KApp::InitProfilesData(wxWindow* parent)
{
	if (!LoadProfile())
	{
		wxLogInfo("Unable to load saved profile. Asking user to choose one.");

		parent->Hide();
		KProfileSelectionDialog dialog(parent);
		wxWindowID ret = dialog.ShowModal();
		if (ret == KxID_OK)
		{
			SetCurrentTemplateID(dialog.GetNewTemplate());
			SetCurrentConfigID(dialog.GetNewConfig());
			SaveSettings();
			wxLogInfo("New TemplateID: %s, New ProfileID: %s", m_CurrentTemplateID, m_CurrentConfigID);

			if (dialog.IsNewGameRootSet())
			{
				wxLogInfo("New game root: %s", m_CurrentTemplateID, dialog.GetNewGameRoot());
				KProfile::SetGameRootPath(dialog.GetNewTemplate(), dialog.GetNewConfig(), dialog.GetNewGameRoot());
			}

			if (!LoadProfile())
			{
				KLogEvent(T("Init.Error1"), KLOG_CRITICAL).Send();
			}
		}
		else if (ret == KxID_CANCEL)
		{
			wxLogInfo("Profile loading canceled. Exiting.");
			ExitApp();
		}
		else
		{
			parent->Show();
		}
	}
}
bool KApp::LoadProfile()
{
	wxString templateID = GetCurrentTemplateID();
	wxString configID = GetCurrentConfigID();
	wxLogInfo("Try load profile. TemplateID: %s, ProfileID: %s", templateID, configID);

	if (!templateID.IsEmpty() && !configID.IsEmpty())
	{
		const KProfile* profileTemplate = KProfile::GetProfileTemplate(templateID);
		if (profileTemplate && profileTemplate->HasConfig(configID))
		{
			KProfile* currentProfile = SetCurrentProfile(new KProfile());
			currentProfile->Create(profileTemplate->GetTemplateFile(), configID);
			
			if (!KxFile(currentProfile->ExpandVariables(KVAR(KVAR_GAME_ROOT))).IsFolderExist())
			{
				SetCurrentProfile(NULL);
				return false;
			}
			m_Variables.SetVariable(KVAR_CURRENT_PROFILE_CONFIG, configID);

			wxLogInfo("Profile loaded successfully");
			return true;
		}
	}
	return false;
}
void KApp::LoadCurrentTemplateAndConfig()
{
	wxCmdLineParser& parser = GetCmdLineParser();
	if (!parser.Found("ProfileID", &m_CurrentTemplateID))
	{
		m_CurrentTemplateID = m_ProfileOptions_AppWide.GetAttribute("TemplateID");
	}
	if (!parser.Found("ConfigID", &m_CurrentConfigID))
	{
		m_CurrentConfigID = m_ProfileOptions_AppWide.GetAttribute("ProfileID");
	}
	wxLogInfo("TemplateID: %s, ProfileID: %s", m_CurrentTemplateID, m_CurrentConfigID);
}

void KApp::InitVFS()
{
	wxLogInfo("Begin initializing VFS");
	if (KIPCClient::RunServerAndConnect(&m_VFSServiceClient))
	{
		wxLogInfo("Server: Started. Initializing driver service.");
		if (!m_VFSServiceClient->InitVFSService())
		{
			wxLogInfo("Server: Driver service init failed.");
		}
		else
		{
			wxLogInfo("Server: Driver service init success.");
		}

		wxLogInfo("Client: Initializing driver service.");
		m_VFSService = new KVirtualFileSystemService();
		wxLogInfo("Client: Driver service init %s.", m_VFSService->IsOK() ? "success" : "failed");

		wxLogInfo("Client: Initializing KModManager.");
		m_ModManager = new KModManager(NULL);
	}
	else
	{
		wxLogInfo("Server: Not started.");
		KLogEvent(T("VFSService.InstallFailed"), KLOG_CRITICAL).Send();
	}
}
void KApp::UnInitVFS()
{
	wxLogInfo("Unmounting VFS");
	KModManager::Get().UnMountVFS();

	wxLogInfo("Uninitializing VFS services");
	delete m_VFSServiceClient;
	delete m_VFSService;
	delete m_ModManager;
}
void KApp::InitProgramManager()
{
	wxLogInfo("Initializing KProgramManager");
	m_ProgramManager = new KProgramManager();
}
void KApp::AddDownloadToAlreadyRunningInstance(const wxString& link)
{
	KxProcess process(KxLibrary(NULL).GetFileName());
	if (process.Find())
	{
		for (HWND hWnd: process.EnumWindows())
		{
			if (KxTLWInternal::GetWindowUserData(hWnd) == KMainWindow::GetUniqueID())
			{
				COPYDATASTRUCT data = {0};
				data.lpData = (void*)link.wc_str();
				data.cbData = link.Length() * sizeof(WCHAR) + sizeof(WCHAR);

				::SendMessageW(hWnd, WM_COPYDATA, 0, (LPARAM)&data);
				return;
			}
		}
	}
}

KSettingsWindowManager* KApp::GetSettingsManager() const
{
	return m_SettingsManager;
}
void KApp::SaveSettings() const
{
	m_SettingsManager->Save();
}
bool KApp::ShowChageProfileDialog()
{
	KProfileSelectionDialog dialog(m_MainWindow);
	wxWindowID ret = dialog.ShowModal();
	if (ret == KxID_OK && (GetCurrentTemplateID() != dialog.GetNewTemplate() || GetCurrentConfigID() != dialog.GetNewConfig()))
	{
		KxTaskDialog confirmDialog(m_MainWindow, KxID_NONE, T("ProfileSelection.ChangeProfileDialog.Caption"), T("ProfileSelection.ChangeProfileDialog.Message"), KxBTN_NONE, KxICON_WARNING);
		confirmDialog.AddButton(KxID_YES, T("ProfileSelection.ChangeProfileDialog.Yes"));
		confirmDialog.AddButton(KxID_NO, T("ProfileSelection.ChangeProfileDialog.No"));
		confirmDialog.AddButton(KxID_CANCEL, T("ProfileSelection.ChangeProfileDialog.Cancel"));
		confirmDialog.SetDefaultButton(KxID_CANCEL);

		ret = confirmDialog.ShowModal();
		if (ret != KxID_CANCEL)
		{
			// Set new game root
			if (dialog.IsNewGameRootSet())
			{
				KProfile::SetGameRootPath(dialog.GetNewTemplate(), dialog.GetNewConfig(), dialog.GetNewGameRoot());
			}

			// Temporary change current profile, save settings, and restore IDs.
			wxString templateID = GetCurrentTemplateID();
			wxString configID = GetCurrentConfigID();

			SetCurrentTemplateID(dialog.GetNewTemplate());
			SetCurrentConfigID(dialog.GetNewConfig());
			SaveSettings();
			m_AllowSaveSettinsgAtExit = false;

			SetCurrentTemplateID(templateID);
			SetCurrentConfigID(configID);

			// Restart if user agreed
			if (ret == KxID_YES)
			{
				ScheduleRestart();
			}
			return true;
		}
	}
	return false;
}

KCMConfigEntryStd* KApp::GetSettingsEntry(KPGCFileID id, const wxString& section, const wxString& name) const
{
	KCMFileEntry* fileEntry = m_SettingsManager->GetEntry(id);
	if (fileEntry)
	{
		for (size_t i = 0; i < fileEntry->GetEntriesCount(); i++)
		{
			KCMConfigEntryBase* configEntry = fileEntry->GetEntryAt(i);
			if (KCMConfigEntryStd* stdEntry = configEntry->ToStdEntry())
			{
				if (stdEntry->GetPath() == section && stdEntry->GetName() == name)
				{
					return stdEntry;
				}
			}
		}
	}
	return NULL;
}
wxString KApp::GetSettingsValue(KPGCFileID id, const wxString& section, const wxString& name) const
{
	KCMDataProviderINI* dataProvider = m_SettingsManager->GetProvider(id);
	if (dataProvider)
	{
		return dataProvider->GetDocument().GetValue(section, name);
	}
	return wxEmptyString;
}
void KApp::SetSettingsValue(KPGCFileID id, const wxString& section, const wxString& name, const wxString& value)
{
	KCMDataProviderINI* dataProvider = m_SettingsManager->GetProvider(id);
	if (dataProvider)
	{
		dataProvider->GetDocument().SetValue(section, name, value);
	}
}
wxString KApp::GetCurrentTemplateID() const
{
	return m_CurrentTemplateID;
}
wxString KApp::GetCurrentConfigID() const
{
	return m_CurrentConfigID;
}
void KApp::SetCurrentTemplateID(const wxString& templateID)
{
	m_CurrentTemplateID = templateID;
	m_ProfileOptions_AppWide.SetAttribute("TemplateID", templateID);
}
void KApp::SetCurrentConfigID(const wxString& configID)
{
	m_CurrentConfigID = configID;
	m_ProfileOptions_AppWide.SetAttribute("ProfileID", configID);
}
void KApp::ConfigureInternetExplorer(bool init)
{
	wxString appName = KxLibrary(NULL).GetFileName().AfterLast('\\');

	#define IERegPath	"SOFTWARE\\Microsoft\\Internet Explorer\\MAIN\\FeatureControl\\"
	if (init)
	{
		KxRegistry::SetValue(KxREG_HKEY_CURRENT_USER, IERegPath"FEATURE_BEHAVIORS", appName, 1, KxREG_VALUE_DWORD);
		KxRegistry::SetValue(KxREG_HKEY_CURRENT_USER, IERegPath"FEATURE_BROWSER_EMULATION", appName, 10000, KxREG_VALUE_DWORD);
	}
	else
	{
		KxRegistry::RemoveValue(KxREG_HKEY_CURRENT_USER, IERegPath"FEATURE_BEHAVIORS", appName);
		KxRegistry::RemoveValue(KxREG_HKEY_CURRENT_USER, IERegPath"FEATURE_BROWSER_EMULATION", appName);
	}
	#undef IERegPath
}

bool KApp::ScheduleRestart()
{
	int delaySec = m_GeneralOptions_AppWide.GetAttributeInt("RestartDelay", 5);
	const wxString taskName = "KortexRestart";

	KxTaskScheduler taskSheduler;

	KxTaskSchedulerTask task = taskSheduler.NewTask();
	task.SetExecutable(KxProcess(0).GetImageName());
	task.SetRegistrationTrigger("Restart", wxTimeSpan(0, 0, delaySec), wxDateTime::Now());
	task.DeleteExpiredTaskAfter(wxTimeSpan(0, 0, 5));

	taskSheduler.DeleteTask(taskName);
	return taskSheduler.SaveTask(task, taskName);
}
bool KApp::Uninstall()
{
	ConfigureInternetExplorer(false);
	m_ModManager->UnMountVFS();
	return m_VFSServiceClient->UninstallVFSService();
}

void KApp::OnErrorOccurred(KLogEvent& event)
{
	KxIconType iconType = KxICON_NONE;
	KImageEnum iconImageID = KIMG_NONE;
	KLogLevel logLevel = event.GetLevel();
	wxWindow* window = event.GetWindow();
	bool isCritical = event.IsCritical();

	switch (logLevel)
	{
		case KLOG_INFO:
		{
			iconType = KxICON_INFORMATION;
			iconImageID = KIMG_INFORMATION_FRAME;
			break;
		}
		case KLOG_WARNING:
		{
			iconType = KxICON_WARNING;
			iconImageID = KIMG_EXCLAMATION_CIRCLE_FRAME;
			break;
		}
		case KLOG_ERROR:
		case KLOG_CRITICAL:
		{
			iconType = KxICON_ERROR;
			iconImageID = KIMG_CROSS_CIRCLE_FRAME;
			break;
		}
	};

	wxString caption;
	wxString message;
	if (IsTranslationLoaded())
	{
		if (event.IsCritical())
		{
			caption = T("Generic.CriticalError");
		}
		else
		{
			caption = T(KxID_ERROR);
		}
		message = event.GetMessage();
	}
	else
	{
		caption = "Error";
		message = event.GetMessage();
	}

	KLogMessage("%s: %s", caption, message);
	auto ShowErrorMessageFunc = [this, caption, message, iconType, window, logLevel, isCritical]()
	{
		KxTaskDialog dialog(window ? window : GetTopWindow(), KxID_NONE, caption, message, KxBTN_OK, iconType);
		dialog.SetOptionEnabled(KxTD_HYPERLINKS_ENABLED);
		if (logLevel == KLOG_INFO)
		{
			dialog.Show();
		}
		else
		{
			dialog.ShowModal();
			if (isCritical)
			{
				ExitApp(KxID_ERROR);
			}
		}
	};
	if (wxThread::IsMain())
	{
		ShowErrorMessageFunc();
	}
	else
	{
		CallAfter(ShowErrorMessageFunc);
	}
}

wxString KApp::GetLogsFolder()
{
	return GetUserSettingsFolder() + "\\Logs";
}
FILE* KApp::OpenLogFile()
{
	wxString fileName = wxDateTime::Now().Format("%Y-%m-%d %H-%M-%S") + ".log";
	wxString filePath = GetLogsFolder();
	wxFileName::Mkdir(filePath, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

	wxString fullPath = filePath + '\\' + fileName;
	return ::_wfopen(fullPath, L"w+b");
}
void KApp::CleanupLogs()
{
	const size_t countToKeep = 10;

	KxFileFinder finder(GetLogsFolder(), "*.log");
	std::vector<KxFileFinderItem> items;
	items.reserve(countToKeep);

	KxFileFinderItem item = finder.FindNext();
	while (item.IsOK())
	{
		if (item.IsNormalItem() && item.IsFile())
		{
			items.push_back(item);
		}
		item = finder.FindNext();
	}

	// Sort by creation date. Most recent first.
	std::sort(items.begin(), items.end(), [](const KxFileFinderItem& v1, const KxFileFinderItem& v2)
	{
		return v1.GetCreationTime() > v2.GetCreationTime();
	});

	// Delete all old logs
	for (size_t i = countToKeep; i < items.size(); i++)
	{
		KxFile(items[i].GetFullPath()).RemoveFile();
	}
}

wxIMPLEMENT_APP(KApp);
