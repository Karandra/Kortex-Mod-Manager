#include "stdafx.h"
#include "DefaultApplication.h"
#include "UI/KMainWindow.h"
#include "UI/KWorkspace.h"
#include "GameInstance/SelectionDialog.h"
#include "VirtualFileSystem/DefaultVFSService.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/ProgramManager.hpp>
#include <Kortex/PackageManager.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/DownloadManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Events.hpp>
#include "Utility/KBitmapSize.h"
#include "Utility/KAux.h"
#include "Utility/Log.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxSplashWindow.h>
#include <KxFramework/KxTaskScheduler.h>
#include <KxFramework/KxCallAtScopeExit.h>

namespace Kortex::Application
{
	namespace OName
	{
		KortexDefOption(RestartDelay);
		KortexDefOption(Instances);
	}

	DefaultApplication::DefaultApplication()
		:m_Translator(m_Translation)
	{
	}

	wxString DefaultApplication::ExpandVariablesLocally(const wxString& variables) const
	{
		return m_Variables.Expand(variables);
	}
	wxString DefaultApplication::ExpandVariables(const wxString& variables) const
	{
		if (IGameInstance* instance = IGameInstance::GetActive())
		{
			return instance->ExpandVariables(variables);
		}
		return ExpandVariablesLocally(variables);
	}

	void DefaultApplication::OnCreate()
	{
		// Revision variable
		m_Variables.SetDynamicVariable(wxS("AppRevision"), [this]()
		{
			return m_Variables.GetVariable(wxS("AppCommitHash")).AsString().Left(7);
		});

		// Setup paths
		const wxString rootFolder = GetRootFolder();

		m_DataFolder = rootFolder + "\\Data";
		m_UserSettingsFolder = KxShell::GetFolder(KxSHF_APPLICATIONDATA_LOCAL) + '\\' + GetID();
		m_UserSettingsFile = m_UserSettingsFolder + "\\Settings.xml";
		m_LogsFolder = m_UserSettingsFolder + "\\Logs";
		m_InstancesFolder = m_UserSettingsFolder + "\\Instances";

		// Configure command line parsing options
		wxCmdLineParser& cmdLineParser = GetCmdLineParser();
		cmdLineParser.SetSwitchChars("-");
		cmdLineParser.AddOption("GameID", wxEmptyString, "Game ID");
		cmdLineParser.AddOption("InstanceID", wxEmptyString, "Instance ID");
		cmdLineParser.AddOption("ProfileID", wxEmptyString, "Profile ID");
		cmdLineParser.AddOption("GlobalConfigPath", wxEmptyString, "Folder path for app-wide config");
		IDownloadManager::ConfigureCommandLine(cmdLineParser);

		// Init global settings folder
		ParseCommandLine();
		GetCmdLineParser().Found("GlobalConfigPath", &m_UserSettingsFolder);
		m_Variables.SetVariable("AppSettings", m_UserSettingsFolder);
		KxFile(m_UserSettingsFolder).CreateFolder();
	}
	void DefaultApplication::OnDestroy()
	{
	}

	bool DefaultApplication::OnInit()
	{
		const bool anotherInstanceRunning = IsAnotherRunning();

		// Load translation and images
		LoadTranslation();
		LoadImages();

		// Check download
		auto downloadLink = IDownloadManager::GetLinkFromCommandLine(GetCmdLineParser());

		// Show loading window
		KxSplashWindow* splashWindow = new KxSplashWindow();
		m_InitProgressDialog = splashWindow;
		KxCallAtScopeExit atExit([this]()
		{
			m_InitProgressDialog->Destroy();
			m_InitProgressDialog = nullptr;
		});

		// Don't show loading screen if it's a download link request
		if (!downloadLink || !anotherInstanceRunning)
		{
			splashWindow->Create(nullptr, ImageProvider::GetBitmap("kortex-logo"));
			splashWindow->Show();
		}
		
		KMainWindow* mainWindow = nullptr;
		if (!anotherInstanceRunning)
		{
			// Set default table-tree-list like controls
			const int defaultRowHeight = KBitmapSize().FromSystemSmallIcon().GetHeight() + m_InitProgressDialog->FromDIP(4);
			wxSystemOptions::SetOption("KxDataViewCtrl::DefaultRowHeight", defaultRowHeight);
			wxSystemOptions::SetOption("KxDataView2::DefaultRowHeight", defaultRowHeight);

			// Init systems
			Utility::Log::LogInfo("Begin initializing core systems");
			
			// Initialize main window
			mainWindow = new KMainWindow();
			SetTopWindow(mainWindow);

			InitSettings();
			InitVFS();
			
			// Order is important
			m_NetworkModule = std::make_unique<NetworkModule>();
			m_PackagesModule = std::make_unique<ModPackagesModule>();
			m_ProgramModule = std::make_unique<KProgramModule>();
			m_GameModsModule = std::make_unique<GameModsModule>();
			Utility::Log::LogInfo("Core systems initialized");

			Utility::Log::LogInfo("Initializing instances");
			InitInstancesData(m_InitProgressDialog);
			IModule::InitModulesWithDisposition(IModule::Disposition::Global);
			IModule::InitModulesWithDisposition(IModule::Disposition::ActiveInstance);
			IModule::InitModulesWithDisposition(IModule::Disposition::Local);

			Utility::Log::LogInfo("Loading saved profile");
			IGameInstance::GetActive()->LoadSavedProfileOrDefault();

			// All required managers initialized, can create main window now
			Utility::Log::LogInfo("Creating main window");
			mainWindow->Create();

			// Show main window and selected workspace
			Utility::Log::LogInfo("Main window created. Showing workspace.");
			ShowWorkspace();
			mainWindow->Show();
			return true;
		}
		else
		{
			// Send download
			if (downloadLink)
			{
				QueueDownloadToMainProcess(*downloadLink);
				return false;
			}

			LogEvent(KTr("Init.AnotherInstanceRunning"), LogLevel::Error);
			return false;
		}
	}
	int DefaultApplication::OnExit()
	{
		Utility::Log::LogInfo("DefaultApplication::OnExit");

		// VFS should be uninitialized first
		UnInitVFS();

		// Destroy other managers
		IModule::UninitModulesWithDisposition(IModule::Disposition::Local);
		IModule::UninitModulesWithDisposition(IModule::Disposition::ActiveInstance);
		IModule::UninitModulesWithDisposition(IModule::Disposition::Global);

		m_GameModsModule.reset();
		m_PackagesModule.reset();
		m_ProgramModule.reset();
		m_NetworkModule.reset();

		return 0;
	}
	
	void DefaultApplication::OnError(LogEvent& event)
	{
		KxIconType iconType = KxICON_NONE;
		ImageResourceID iconImageID = ImageResourceID::None;
		LogLevel logLevel = event.GetLevel();
		wxWindow* window = event.GetWindow();
		bool isCritical = event.IsCritical();

		switch (logLevel)
		{
			case LogLevel::Info:
			{
				iconType = KxICON_INFORMATION;
				iconImageID = ImageResourceID::InformationFrame;
				break;
			}
			case LogLevel::Warning:
			{
				iconType = KxICON_WARNING;
				iconImageID = ImageResourceID::ExclamationCircleFrame;
				break;
			}
			case LogLevel::Error:
			case LogLevel::Critical:
			{
				iconType = KxICON_ERROR;
				iconImageID = ImageResourceID::CrossCircleFrame;
				break;
			}
		};

		wxString caption;
		wxString message;
		if (IsTranslationLoaded())
		{
			if (event.IsCritical())
			{
				caption = KTr("Generic.CriticalError");
			}
			else
			{
				caption = KTr(KxID_ERROR);
			}
			message = event.GetMessage();
		}
		else
		{
			caption = "Error";
			message = event.GetMessage();
		}

		Utility::Log::LogMessage("%1: %2", caption, message);
		auto ShowErrorMessageFunc = [this, caption, message, iconType, window, logLevel, isCritical]()
		{
			KxTaskDialog dialog(window ? window : GetTopWindow(), KxID_NONE, caption, message, KxBTN_OK, iconType);
			dialog.SetOptionEnabled(KxTD_HYPERLINKS_ENABLED);
			if (logLevel == LogLevel::Info)
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
			IEvent::CallAfter(ShowErrorMessageFunc);
		}
	}
	bool DefaultApplication::OnException()
	{
		LogEvent(RethrowCatchAndGetExceptionInfo(), LogLevel::Error);
		return false;
	}
	
	void DefaultApplication::OnGlobalConfigChanged(IAppOption& option)
	{
	}
	void DefaultApplication::OnInstanceConfigChanged(IAppOption& option, IGameInstance& instance)
	{
	}
	void DefaultApplication::OnProfileConfigChanged(IAppOption& option, IGameProfile& profile)
	{
	}

	bool DefaultApplication::OpenInstanceSelectionDialog()
	{
		GameInstance::SelectionDialog dialog(KMainWindow::GetInstance());
		wxWindowID ret = dialog.ShowModal();
		IGameInstance* selectedInstance = dialog.GetSelectedInstance();

		if (ret == KxID_OK && (selectedInstance && m_StartupInstanceID != selectedInstance->GetInstanceID()))
		{
			KxTaskDialog confirmDialog(KMainWindow::GetInstance(), KxID_NONE, KTr("InstanceSelection.ChangeInstanceDialog.Caption"), KTr("InstanceSelection.ChangeInstanceDialog.Message"), KxBTN_NONE, KxICON_WARNING);
			confirmDialog.AddButton(KxID_YES, KTr("InstanceSelection.ChangeInstanceDialog.Yes"));
			confirmDialog.AddButton(KxID_NO, KTr("InstanceSelection.ChangeInstanceDialog.No"));
			confirmDialog.AddButton(KxID_CANCEL, KTr("InstanceSelection.ChangeInstanceDialog.Cancel"));
			confirmDialog.SetDefaultButton(KxID_CANCEL);

			ret = confirmDialog.ShowModal();
			if (ret != KxID_CANCEL)
			{
				// Set new game root
				IConfigurableGameInstance* configurableInstance = nullptr;
				if (dialog.IsGameRootSelected() && selectedInstance->QueryInterface(configurableInstance))
				{
					selectedInstance->GetVariables().SetVariable(Variables::KVAR_ACTUAL_GAME_DIR, dialog.GetSelectedGameRoot());
					configurableInstance->SaveConfig();
				}
				GetGlobalOption(OName::Instances, OName::Active).SetValue(selectedInstance->GetInstanceID());

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
	bool DefaultApplication::ScheduleRestart()
	{
		int delaySec = GetGlobalOption(OName::RestartDelay).GetValueInt(5);
		const wxString taskName = "Kortex::ScheduleRestart";

		KxTaskScheduler taskSheduler;
		KxTaskSchedulerTask task = taskSheduler.NewTask();
		task.SetExecutable(KxProcess(0).GetImageName());
		task.SetRegistrationTrigger("Restart", wxTimeSpan(0, 0, delaySec), wxDateTime::Now());
		task.DeleteExpiredTaskAfter(wxTimeSpan(0, 0, 5));

		taskSheduler.DeleteTask(taskName);
		return taskSheduler.SaveTask(task, taskName);
	}
	bool DefaultApplication::Uninstall()
	{
		DisableIE10Support();
		IModManager::GetInstance()->GetFileSystem().Disable();
		return m_VFSService->Uninstall();
	}

	void DefaultApplication::LoadTranslation()
	{
		m_AvailableTranslations = KxTranslation::FindTranslationsInDirectory(m_DataFolder + "\\Translation");
		auto option = GetGlobalOption(OName::Language);

		switch (TryLoadTranslation(m_Translation, m_AvailableTranslations, "Application", option.GetAttribute(OName::Locale)))
		{
			case LoadTranslationStatus::Success:
			{
				KxTranslation::SetCurrent(m_Translation);
				option.SetAttribute(OName::Locale, m_Translation.GetLocale());
				break;
			}
			case LoadTranslationStatus::LoadingError:
			{
				LogEvent("Can't load translation from file", LogLevel::Critical).Send();
				break;
			}
			case LoadTranslationStatus::NoTranslations:
			{
				LogEvent("No translations found. Terminating.", LogLevel::Critical).Send();
				break;
			}
		};
	}
	void DefaultApplication::LoadImages()
	{
		m_ImageProvider.LoadImages();
	}
	void DefaultApplication::ShowWorkspace()
	{
		auto option = GetAInstanceOption(OName::Workspace);
		wxString startPage = option.GetValue(ModManager::Workspace::GetInstance()->GetID());
		Utility::Log::LogInfo("Start page is: %1", startPage);

		KWorkspace* workspace = KMainWindow::GetInstance()->GetWorkspace(startPage);
		if (!workspace || !workspace->CanBeStartPage() || !workspace->SwitchHere())
		{
			Utility::Log::LogInfo("Can't show workspace %1. Trying first available", startPage);

			bool isSuccess = false;
			for (const auto& v: KMainWindow::GetInstance()->GetWorkspacesList())
			{
				workspace = v.second;
				if (workspace->CanBeStartPage())
				{
					Utility::Log::LogInfo("Trying to load %1 workspace", workspace->GetID());
					isSuccess = workspace->SwitchHere();
					break;
				}
			}

			if (isSuccess)
			{
				startPage = workspace->GetID();
				Utility::Log::LogInfo("Successfully showed %1 workspace", startPage);
			}
			else
			{
				Utility::Log::LogInfo("No workspaces available. Terminating");
				LogEvent(KTr("Init.Error3"), LogLevel::Critical, KMainWindow::GetInstance()).Send();
			}
		}
		else
		{
			Utility::Log::LogInfo("Successfully showed %1 workspace", startPage);
		}

		option.SetValue(startPage);
	}

	void DefaultApplication::InitSettings()
	{
		EnableIE10Support();
		Utility::Log::LogInfo("Initializing app settings");

		// Init some application-wide variables
		auto options = GetGlobalOption(OName::Instances, OName::Location);
		m_InstancesFolder = options.GetValue(m_InstancesFolder);

		// Show first time config dialog if needed and save new 'ProfilesFolder'
		if (IsPreStartConfigNeeded())
		{
			Utility::Log::LogInfo("Pre start config needed");
			if (!ShowFirstTimeConfigDialog(m_InitProgressDialog))
			{
				LogEvent(KTr("Init.Error1"), LogLevel::Critical).Send();
				return;
			}
		}

		m_Variables.SetVariable(Variables::KVAR_INSTANCES_DIR, m_InstancesFolder);
		options.SetValue(m_InstancesFolder);
		Utility::Log::LogInfo("Instances folder changed: %1", m_InstancesFolder);

		// Init all profiles and load current one if specified (or ask user to choose it)
		Utility::Log::LogInfo("Settings initialized. Begin loading profile.");
	}
	bool DefaultApplication::IsPreStartConfigNeeded()
	{
		return m_InstancesFolder.IsEmpty() || !KxFile(m_InstancesFolder).IsFolderExist();
	}
	bool DefaultApplication::ShowFirstTimeConfigDialog(wxWindow* parent)
	{
		const wxString defaultPath = GetUserSettingsFolder() + wxS("\\Instances");

		wxString message = wxString::Format("%s\r\n\r\n%s: %s", KTr("Init.ProfilesPath2"), KTr("Generic.DefaultValue"), defaultPath);
		KxTaskDialog messageDialog(parent, KxID_NONE, KTr("Init.ProfilesPath1"), message, KxBTN_NONE);
		messageDialog.AddButton(KxID_YES, KTr("Generic.UseDefaultValue"));
		messageDialog.AddButton(KxID_NO, KTr("Generic.BrowseFolder"));

		if (messageDialog.ShowModal() == KxID_YES)
		{
			m_InstancesFolder = defaultPath;
			m_Variables.SetVariable(Variables::KVAR_INSTANCES_DIR, m_InstancesFolder);
			return true;
		}
		else
		{
			KxFileBrowseDialog folderDialog(m_InitProgressDialog, KxID_NONE, KxFBD_OPEN_FOLDER);
			folderDialog.SetFolder(defaultPath);
			if (folderDialog.ShowModal() == KxID_OK)
			{
				m_InstancesFolder = folderDialog.GetResult();
				m_Variables.SetVariable(Variables::KVAR_INSTANCES_DIR, m_InstancesFolder);
				return true;
			}
			return false;
		}
	}
	void DefaultApplication::InitInstancesData(wxWindow* parent)
	{
		LoadStartupInstanceID();
		IGameInstance::LoadTemplates();
		IGameInstance::LoadInstances();

		if (!LoadInstance())
		{
			Utility::Log::LogInfo("Unable to load saved instance. Asking user to choose one.");

			parent->Hide();
			GameInstance::SelectionDialog dialog(parent);
			wxWindowID ret = dialog.ShowModal();
			if (ret == KxID_OK)
			{
				// Instance ID
				m_StartupInstanceID = dialog.GetSelectedInstance()->GetInstanceID();
				if (!m_IsCmdStartupInstanceID)
				{
					GetGlobalOption(OName::Instances, OName::Active).SetValue(m_StartupInstanceID);
				}

				// Set new game root
				IGameInstance* activeInstnace = IGameInstance::GetActive();
				if (activeInstnace && dialog.IsGameRootSelected())
				{
					activeInstnace->GetVariables().SetVariable(Variables::KVAR_ACTUAL_GAME_DIR, dialog.GetSelectedGameRoot());
				}

				Utility::Log::LogInfo("New InstanceID: %1", m_StartupInstanceID);
				Utility::Log::LogInfo("Trying again");

				if (!LoadInstance())
				{
					LogEvent(KTr("Init.Error1"), LogLevel::Critical).Send();
				}
				return;
			}
			else if (ret == KxID_CANCEL)
			{
				Utility::Log::LogInfo("Instance loading canceled. Exiting.");
				ExitApp();
			}
		}
		parent->Show();
	}
	bool DefaultApplication::LoadInstance()
	{
		Utility::Log::LogInfo("Trying load instance. InstanceID: %1", m_StartupInstanceID);

		if (!m_StartupInstanceID.IsEmpty())
		{
			const IGameInstance* instance = IGameInstance::GetShallowInstance(m_StartupInstanceID);
			if (instance && instance->IsOK() && KxFile(instance->GetGameDir()).IsFolderExist())
			{
				m_Variables.SetVariable(Variables::KVAR_GAME_ID, instance->GetGameID().ToString());
				m_Variables.SetVariable(Variables::KVAR_INSTANCE_ID, m_StartupInstanceID);

				return IGameInstance::CreateActive(instance->GetTemplate(), m_StartupInstanceID);
			}
		}
		return false;
	}
	void DefaultApplication::LoadStartupInstanceID()
	{
		wxCmdLineParser& parser = GetCmdLineParser();
		if (parser.Found("InstanceID", &m_StartupInstanceID))
		{
			m_IsCmdStartupInstanceID = true;
		}
		else
		{
			m_StartupInstanceID = GetGlobalOption(OName::Instances, OName::Active).GetValue();
		}
		Utility::Log::LogInfo("Instance: %1", m_StartupInstanceID);
	}

	void DefaultApplication::InitVFS()
	{
		Utility::Log::LogInfo("Begin initializing VFS");
		m_VFSService = std::make_unique<VirtualFileSystem::DefaultVFSService>();

		if (m_VFSService && m_VFSService->IsOK())
		{
			if (!m_VFSService->IsInstalled())
			{
				m_VFSService->Install();
			}
			m_VFSService->Start();
		}
		else
		{
			Utility::Log::LogInfo("Server: Not started.");
			LogEvent(KTr("VFS.Service.InstallFailed"), LogLevel::Critical).Send();
		}
	}
	void DefaultApplication::UnInitVFS()
	{
		Utility::Log::LogInfo("Unmounting VFS");
		IModManager::GetInstance()->GetFileSystem().Disable();
		m_VFSService->Stop();
	}
}
