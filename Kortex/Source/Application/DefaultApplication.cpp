#include "stdafx.h"
#include "DefaultApplication.h"
#include "SystemApplication.h"
#include "Options/CmdLineDatabase.h"
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
#include "Utility/KBitmapSize.h"
#include "Utility/KAux.h"
#include "Utility/Log.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxSplashWindow.h>
#include <KxFramework/KxCallAtScopeExit.h>

namespace Kortex::Application
{
	namespace OName
	{
		KortexDefOption(RestartDelay);
		KortexDefOption(Instances);
	}
}

namespace Kortex::Application
{
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
		auto downloadLink = GetLinkFromCommandLine();

		// Show loading window
		KxSplashWindow* splashWindow = new KxSplashWindow();
		m_InitProgressDialog = splashWindow;
		KxCallAtScopeExit atExit([this]()
		{
			m_InitProgressDialog->Destroy();
			m_InitProgressDialog = nullptr;
		});

		// Don't show loading screen if there's a download link or it's secondary process
		if (!downloadLink && !anotherInstanceRunning)
		{
			splashWindow->Create(nullptr, ImageProvider::GetBitmap("kortex-logo"));
			splashWindow->Show();
		}
		
		if (!anotherInstanceRunning)
		{
			// Set default table-tree-list like controls
			const int defaultRowHeight = KBitmapSize().FromSystemSmallIcon().GetHeight() + m_InitProgressDialog->FromDIP(4);
			wxSystemOptions::SetOption("KxDataViewCtrl::DefaultRowHeight", defaultRowHeight);
			wxSystemOptions::SetOption("KxDataView2::DefaultRowHeight", defaultRowHeight);

			// Init systems
			Utility::Log::LogInfo("Begin initializing core systems");
			
			// Initialize main window
			SetTopWindow(new KMainWindow());

			if (InitSettings(downloadLink.has_value()))
			{
				// Order is important
				m_NetworkModule = std::make_unique<NetworkModule>();
				m_PackagesModule = std::make_unique<ModPackagesModule>();
				m_ProgramModule = std::make_unique<KProgramModule>();
				m_GameModsModule = std::make_unique<GameModsModule>();
				Utility::Log::LogInfo("Core systems initialized");

				Utility::Log::LogInfo("Initializing instances");
				if (BeginLoadCurrentInstance(m_InitProgressDialog, downloadLink.has_value()))
				{
					// Hook-in processing of download link dispatching
					if (downloadLink)
					{
						Utility::Log::LogInfo("Download link found (%1), trying to dispatch it its target", *downloadLink);
						
						bool canContinue = false;
						if (!DispatchDownloadLink(*downloadLink, &canContinue))
						{
							BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtCritical, KTrf("Init.DownloadLinkDispatchFailed", *downloadLink));
						}
						if (!canContinue)
						{
							return false;
						}
					}
					return FinalizeInitialization();
				}
			}
		}
		else
		{
			// Send download
			if (downloadLink)
			{
				QueueDownloadToMainProcess(*downloadLink);
				return false;
			}
			BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtError, KTr("Init.AnotherInstanceRunning"));
		}
		return false;
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

		if (event.GetLevel() == LogEvent::EvtInfo)
		{
			iconType = KxICON_INFORMATION;
			iconImageID = ImageResourceID::InformationFrame;
		}
		else if (event.GetLevel() == LogEvent::EvtWarning)
		{
			iconType = KxICON_WARNING;
			iconImageID = ImageResourceID::ExclamationCircleFrame;
		}
		else if (event.GetLevel() == LogEvent::EvtError || event.GetLevel() == LogEvent::EvtCritical)
		{
			iconType = KxICON_ERROR;
			iconImageID = ImageResourceID::CrossCircleFrame;
		}

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
			message = event.GetString();
		}
		else
		{
			caption = "Error";
			message = event.GetString();
		}

		auto ShowMessage = [&, window = event.GetWindow()]()
		{
			KxTaskDialog dialog(window ? window : GetTopWindow(), KxID_NONE, caption, message, KxBTN_OK, iconType);
			dialog.SetOptionEnabled(KxTD_HYPERLINKS_ENABLED);
			if (event.GetLevel() == LogEvent::EvtInfo)
			{
				dialog.Show();
			}
			else
			{
				dialog.ShowModal();
				if (event.IsCritical())
				{
					ExitApp(KxID_ERROR);
				}
			}
		};

		if (wxThread::IsMain())
		{
			ShowMessage();
		}
		else
		{
			BroadcastProcessor::Get().CallAfter(std::move(ShowMessage));
		}
	}
	bool DefaultApplication::OnException()
	{
		BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtError, RethrowCatchAndGetExceptionInfo());
		return false;
	}
	
	void DefaultApplication::OnGlobalConfigChanged(AppOption& option)
	{
	}
	void DefaultApplication::OnInstanceConfigChanged(AppOption& option, IGameInstance& instance)
	{
	}
	void DefaultApplication::OnProfileConfigChanged(AppOption& option, IGameProfile& profile)
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
				BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtCritical, "Can't load translation from file");
				break;
			}
			case LoadTranslationStatus::NoTranslations:
			{
				BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtCritical, "No translations found. Terminating.");
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
		wxString startPage = option.GetValue(IModManager::GetInstance()->GetWorkspace()->GetID());
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
				BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtCritical, KTr("Init.Error3"), KMainWindow::GetInstance());
			}
		}
		else
		{
			Utility::Log::LogInfo("Successfully showed %1 workspace", startPage);
		}

		option.SetValue(startPage);
	}

	bool DefaultApplication::InitSettings(bool downloadLinkPresent)
	{
		Utility::Log::LogInfo("Initializing app settings");

		// Init global settings folder
		GetCmdLineParser().Found(CmdLineName::GlobalConfigPath, &m_UserSettingsFolder);
		m_Variables.SetVariable("AppSettings", m_UserSettingsFolder);
		KxFile(m_UserSettingsFolder).CreateFolder();

		// Enable IE10 emulation level for WebView
		EnableIE10Support();

		// Init some application-wide variables
		auto options = GetGlobalOption(OName::Instances, OName::Location);
		m_InstancesFolder = options.GetValue(m_InstancesFolder);

		// Show first time config dialog if needed and save new 'ProfilesFolder'
		if (IsPreStartConfigNeeded())
		{
			if (downloadLinkPresent)
			{
				return false;
			}

			Utility::Log::LogInfo("Pre start config needed");
			if (!ShowFirstTimeConfigDialog(m_InitProgressDialog))
			{
				BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtCritical, KTr("Init.Error1"));
				return false;
			}
		}

		m_Variables.SetVariable(Variables::KVAR_INSTANCES_DIR, m_InstancesFolder);
		options.SetValue(m_InstancesFolder);
		Utility::Log::LogInfo("Instances folder changed: %1", m_InstancesFolder);

		// Init all profiles and load current one if specified (or ask user to choose it)
		Utility::Log::LogInfo("Settings initialized. Begin loading profile.");

		return true;
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
	bool DefaultApplication::BeginLoadCurrentInstance(wxWindow* parent, bool downloadLinkPresent)
	{
		LoadStartupInstanceID();
		IGameInstance::LoadTemplates();
		IGameInstance::LoadInstances();

		if (!LoadInstance() && !downloadLinkPresent)
		{
			Utility::Log::LogInfo("Unable to load saved instance. Asking user to choose one.");

			if (parent)
			{
				parent->Hide();
			}

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
					BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtCritical, KTr("Init.Error1"));
					return false;
				}
				return true;
			}
			else if (ret == KxID_CANCEL)
			{
				Utility::Log::LogInfo("Instance loading canceled. Exiting.");
				ExitApp();

				return false;
			}
		}

		if (parent)
		{
			parent->Show();
		}
		return true;
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
	bool DefaultApplication::DispatchDownloadLink(const wxString& link, bool* canContinue)
	{
		// Enum all mod repositories and ask them to process the link
		for (ModNetworkRepository* repository: INetworkManager::GetInstance()->GetModRepositories())
		{
			Utility::Log::LogInfo("Trying repository '%1'", repository->GetContainer().GetName());

			wxAny target = repository->GetDownloadTarget(link);
			if (IGameInstance* instance = nullptr; target.GetAs(&instance) && instance)
			{
				if (instance->GetInstanceID() != m_StartupInstanceID)
				{
					Utility::Log::LogInfo("Restarting with queuing download to another instance: '%1'", instance->GetInstanceID());

					CmdLineParameters parameters;
					parameters.InstanceID = instance->GetInstanceID();
					parameters.DownloadLink = link;
					ScheduleRestart(FormatCommandLine(parameters), wxTimeSpan::Seconds(1));
				}
				else
				{
					Utility::Log::LogInfo("Current instance matches with target instance (%1), proceeding", instance->GetInstanceID());
					
					if (canContinue)
					{
						*canContinue = true;
					}
				}
				return true;
			}
			else if (CmdLine command; target.GetAs(&command))
			{
				Utility::Log::LogInfo("Starting external process '%1' with arguments '%2'", command.Executable, command.Arguments);

				if (command.Executable != GetExecutablePath())
				{
					KxProcess process(command.Executable, KxString::Format(command.Arguments, link));
					process.Run(KxPROCESS_RUN_SYNC);
					return true;
				}
				else
				{
					Utility::Log::LogInfo("Attempt to run itself as an external program");
					return false;
				}
			}
		}
		return false;
	}
	bool DefaultApplication::FinalizeInitialization()
	{
		IModule::InitModulesWithDisposition(IModule::Disposition::Global);
		IModule::InitModulesWithDisposition(IModule::Disposition::ActiveInstance);
		IModule::InitModulesWithDisposition(IModule::Disposition::Local);
		InitVFS();

		Utility::Log::LogInfo("Loading saved profile");
		IGameInstance::GetActive()->LoadSavedProfileOrDefault();

		// All required managers initialized, can create main window now
		Utility::Log::LogInfo("Creating main window");
		KMainWindow::GetInstance()->Create();

		// Show main window and selected workspace
		Utility::Log::LogInfo("Main window created, displaying initial workspace.");
		ShowWorkspace();
		KMainWindow::GetInstance()->Show();
		return true;
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
			BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtCritical, KTr("VFS.Service.InstallFailed"));
		}
	}
	void DefaultApplication::UnInitVFS()
	{
		Utility::Log::LogInfo("Unmounting VFS");
		IModManager::GetInstance()->GetFileSystem().Disable();
		m_VFSService->Stop();
	}
}
