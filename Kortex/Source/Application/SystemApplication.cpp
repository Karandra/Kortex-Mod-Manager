#include "stdafx.h"
#include "SystemApplication.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Theme.hpp>
#include <Kortex/Events.hpp>
#include "Archive/KArchive.h"
#include "UI/KMainWindow.h"
#include "KAux.h"
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxRegistry.h>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxSystem.h>
#include <wx/apptrait.h>

namespace
{
	using namespace Kortex;

	void LogConfigChnage(const IAppOption& option)
	{
		wxLogInfo("Path: \"%s\", Value: \"%s\"", option.GetXPath(), option.GetValue());
	}
}

namespace Kortex
{
	class SystemApplicationTraits: public wxGUIAppTraits
	{
		friend class SystemApplication;

		private:
			SystemApplication& m_SysApp;
			FILE* m_LogTargetFILE = nullptr;
			wxLogStderr* m_LogTarget = nullptr;

		private:
			FILE* CreateLogFile() const
			{
				wxString fileName = wxDateTime::Now().Format("%Y-%m-%d %H-%M-%S") + ".log";
				wxString filePath = m_SysApp.m_Application->GetLogsFolder();
				wxFileName::Mkdir(filePath, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

				wxString fullPath = filePath + wxS('\\') + fileName;

				FILE* handle = nullptr;
				::_wfopen_s(&handle, fullPath.wc_str(), L"w+b");
				return handle;
			}

		public:
			SystemApplicationTraits(SystemApplication& sysApp)
				:m_SysApp(sysApp)
			{
			}

		public:
			wxLog* CreateLogTarget() override
			{
				m_LogTargetFILE = CreateLogFile();
				m_LogTarget = new wxLogStderr(m_LogTargetFILE);
				return m_LogTarget;
			}
	};
}

namespace Kortex
{
	namespace SystemApplicationInfo
	{
		const constexpr wxChar ID[] = wxS("KortexModManager");
		const constexpr wxChar Name[] = wxS("Kortex Mod Manager");
		const constexpr wxChar Version[] = wxS("2.0");
		const constexpr wxChar Developer[] = wxS("Kerber");

		const constexpr wxChar GUID[] = wxS("B5E8047C-9239-45C4-86F6-6C83A842063E");
		const constexpr wxChar CommitHash[] = wxS("ee1f8acb5a9528de3ff0c118595b1d757ccc2bae");
	}

	void SystemApplication::InitLogging()
	{
		Bind(Events::Log, &SystemApplication::OnError, this);
		wxLog::SetVerbose(true);

		wxLogInfo(KxString::Format("%1 v%2: Log opened", SystemApplicationInfo::Name, SystemApplicationInfo::Version));
	}
	void SystemApplication::UninitLogging()
	{
		wxLogInfo("Log closed");
		wxLog::FlushActive();
		CleanupLogs();
	}
	
	void SystemApplication::InitComponents()
	{
		wxLogInfo("SystemApplication::InitComponents");

		KArchive::Init();
		m_ThemeManager = std::make_unique<Theme::Default>();
		m_NotificationCenter = std::make_unique<Notification::DefaultNotificationCenter>();
	}
	void SystemApplication::UninitComponents()
	{
		wxLogInfo("SystemApplication::UninitComponents");

		KArchive::UnInit();
	}

	void SystemApplication::LoadGlobalConfig()
	{
		wxLogInfo("SystemApplication::LoadGlobalConfig");

		KxFileStream stream(m_Application->GetUserSettingsFile(), KxFileStream::Access::RW, KxFileStream::Disposition::OpenAlways, KxFileStream::Share::Read);
		const bool success = m_GlobalConfig.Load(stream);
		if (!success)
		{
			// Query some option to initialize file structure
			m_Application->GetGlobalOption();

			// Save
			stream.Rewind();
			stream.SetAllocationSize(0);
			m_GlobalConfig.Save(stream);
		}

		wxLogInfo("SystemApplication::LoadGlobalConfig -> %s", success ? "true" : "false");
	}
	void SystemApplication::SaveGlobalConfig()
	{
		wxLogInfo("SystemApplication::SaveGlobalConfig");

		KxFileStream stream(m_Application->GetUserSettingsFile(), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
		const bool success = m_GlobalConfig.Save(stream);

		wxLogInfo("SystemApplication::SaveGlobalConfig -> %s", success ? "true" : "false");
	}
	void SystemApplication::SaveActiveInstanceSettings()
	{
		wxLogInfo("SystemApplication::SaveActiveInstanceSettings");
		
		IGameInstance* instance = IGameInstance::GetActive();
		IConfigurableGameInstance* configurableInstance = nullptr;
		if (instance && instance->QueryInterface(configurableInstance))
		{
			wxLogInfo("Calling IConfigurableGameInstance::OnExit");
			configurableInstance->OnExit();
		}
	}

	SystemApplication::SystemApplication()
	{
		// Set basic info
		SetAppName(SystemApplicationInfo::ID);
		SetAppDisplayName(SystemApplicationInfo::Name);
		SetAppVersion(SystemApplicationInfo::Version);
		SetVendorName(SystemApplicationInfo::Developer);

		// Create default application
		m_Application = std::make_unique<Application::DefaultApplication>();
		m_Application->OnCreate();
		
		// Initialize main variables
		IVariableTable& variables = m_Application->GetVariables();
		variables.SetVariable("AppID", SystemApplicationInfo::ID);
		variables.SetVariable("AppName", SystemApplicationInfo::Name);
		variables.SetVariable("AppVersion", SystemApplicationInfo::Version);
		variables.SetVariable("AppDeveloper", SystemApplicationInfo::Developer);
		variables.SetVariable("AppGUID", SystemApplicationInfo::GUID);
		variables.SetVariable("AppCommitHash", SystemApplicationInfo::CommitHash);
		variables.SetVariable("AppModProjectProgID", "KMM.ModProject.1");
		variables.SetVariable("AppModPackageProgID", "KMM.ModPackage.1");
		variables.SetVariable("AppData", m_Application->GetDataFolder());
		variables.SetVariable("SystemArchitecture", KAux::ArchitectureToNumber(m_Application->IsSystem64Bit()));
		variables.SetVariable("SystemArchitectureName", KAux::ArchitectureToString(m_Application->IsSystem64Bit()));
	}
	SystemApplication::~SystemApplication()
	{
		if (m_Application)
		{
			m_Application->OnDestroy();
		}
	}

	bool SystemApplication::OnInit()
	{
		// Initialize logging
		InitLogging();
		wxLogInfo("SystemApplication::OnInit");

		// Log system info
		KxSystem::VersionInfo versionInfo = KxSystem::GetVersionInfo();
		wxLogInfo("System: %s %s %s. Kernel version: %d.%d", KxSystem::GetName(), m_Application->IsSystem64Bit() ? "x64" : "x86", versionInfo.ServicePack, versionInfo.MajorVersion, versionInfo.MinorVersion);
		wxLogInfo("Another instance detected: %s", IsAnotherRunning() ? "true" : "false");

		// Log paths
		wxLogInfo("Root folder: %s", m_Application->GetRootFolder());
		wxLogInfo("Data folder: %s", m_Application->GetDataFolder());
		wxLogInfo("Logs folder: %s", m_Application->GetLogsFolder());
		wxLogInfo("User settings folder: %s", m_Application->GetUserSettingsFolder());
		wxLogInfo("User settings file: %s", m_Application->GetUserSettingsFile());
		wxLogInfo("Instances folder: %s", m_Application->GetInstancesFolder());

		// Run initialization
		LoadGlobalConfig();
		InitComponents();
		m_IsApplicationInitialized = m_Application->OnInit();

		wxLogInfo("Initializing app: %s", m_IsApplicationInitialized ? "true" : "false");
		if (!m_IsApplicationInitialized)
		{
			UninitLogging();
		}
		return m_IsApplicationInitialized;
	}
	int SystemApplication::OnExit()
	{
		wxLogInfo("SystemApplication::OnExit");

		if (m_IsApplicationInitialized)
		{
			m_ExitCode = m_Application->OnExit();
		}
		else
		{
			wxLogInfo("Exiting before initialization is completed");
		}
		wxLogInfo("Exit code: %d", m_ExitCode);
		
		UninitComponents();
		SaveActiveInstanceSettings();
		SaveGlobalConfig();

		// Exit now
		UninitLogging();
		if (m_IsApplicationInitialized)
		{
			KxApp::OnExit();
		}
		return HasExternalExitCode() ? GetExternalExitCode() : m_ExitCode;
	}
	void SystemApplication::OnError(LogEvent& event)
	{
		wxLogInfo("SystemApplication::OnError");
		wxLogInfo("An error occurred:\r\n\r\n%s", event.GetMessage());

		m_Application->OnError(event);
	}
	
	bool SystemApplication::OnGlobalConfigChanged(IAppOption& option)
	{
		wxLogInfo("SystemApplication::OnGlobalConfigChanged");
		LogConfigChnage(option);

		return m_Application->OnGlobalConfigChanged(option);
	}
	bool SystemApplication::OnInstanceConfigChanged(IAppOption& option, IGameInstance& instance)
	{
		wxLogInfo("SystemApplication::OnInstanceConfigChanged");
		wxLogInfo("InstanceID: %s", instance.GetInstanceID());
		LogConfigChnage(option);

		instance.QueryInterface<IConfigurableGameInstance>()->OnConfigChanged(option);
		return m_Application->OnInstanceConfigChanged(option, instance);
	}
	bool SystemApplication::OnProfileConfigChanged(IAppOption& option, IGameProfile& profile)
	{
		wxLogInfo("SystemApplication::OnProfileConfigChanged");
		wxLogInfo("ProfileID: %s", profile.GetID());
		LogConfigChnage(option);

		profile.OnConfigChanged(option);
		return m_Application->OnProfileConfigChanged(option, profile);
	}

	bool SystemApplication::OnException()
	{
		wxLogError("SystemApplication::OnException");
		if (wxIsDebuggerRunning())
		{
			wxLogError("Debugger is running, rethrowing exception");

			throw;
			return false;
		}

		if (!m_Application->OnException())
		{
			wxLogError("Exception info: %s", RethrowCatchAndGetExceptionInfo());
			return false;
		}
		return true;
	}
	bool SystemApplication::OnExceptionInMainLoop()
	{
		wxLogError("SystemApplication::OnExceptionInMainLoop");

		// Exit the main loop and terminate the program if 'OnException' return false.
		return OnException();
	}
	void SystemApplication::OnUnhandledException()
	{
		wxLogError("SystemApplication::OnUnhandledException");

		if (!OnException())
		{
			constexpr int exitCode = std::numeric_limits<int>::min();
			wxLogError("Terminating with code: %d", exitCode);
			ExitApp(exitCode);
		}
	}
	wxString SystemApplication::RethrowCatchAndGetExceptionInfo() const
	{
		wxLogError("Trying to extract message form current exception");

		wxString message = "Unknown";
		wxString type = "Unknown";
		try
		{
			throw;
		}
		catch (const std::exception& e)
		{
			type = "std::exception";
			message = e.what();
		}
		catch (const std::string& e)
		{
			type = "std::string";
			message = e;
		}
		catch (const std::wstring& e)
		{
			type = "std::wstring";
			message = e;
		}
		catch (const char* e)
		{
			type = "const char*";
			message = e;
		}
		catch (const wchar_t* e)
		{
			type = "const wchar_t*";
			message = e;
		}
		catch (...)
		{
		}
		return KxString::Format("Unexpected exception has occurred: %1.\r\n\r\nThe program will terminate.\r\n\r\nException type: %2", message, type);
	}
	
	wxAppTraits* SystemApplication::CreateTraits()
	{
		m_AppTraits = new SystemApplicationTraits(*this);
		return m_AppTraits;
	}
	void SystemApplication::ExitApp(int exitCode)
	{
		if (!KxApp::GetMainLoop())
		{
			// If there are no main loop then initialization process is not completed.
			// In such case, we need to set exit code and call 'SystemApplication::OnExit' manually
			// and then terminate the program without invoking wxWidgets. Because without main loop
			// the 'wxExit' will call 'abort(-1)' anyway, but we need to exit with out own exit code.

			m_ExitCode = exitCode;
			std::exit(OnExit());
		}
		else
		{
			// Normal program termination process.
			KxApp::ExitApp(exitCode);
		}
	}

	bool SystemApplication::IsAnotherRunning() const
	{
		return m_SingleInstanceChecker.IsAnotherRunning();
	}
	void SystemApplication::ConfigureForInternetExplorer10(bool init) const
	{
		wxString appName = KxLibrary(nullptr).GetFileName().AfterLast('\\');

		#define IERegPath	wxS("SOFTWARE\\Microsoft\\Internet Explorer\\MAIN\\FeatureControl\\")
		if (init)
		{
			KxRegistry::SetValue(KxREG_HKEY_CURRENT_USER, IERegPath wxS("FEATURE_BEHAVIORS"), appName, 1, KxREG_VALUE_DWORD);
			KxRegistry::SetValue(KxREG_HKEY_CURRENT_USER, IERegPath wxS("FEATURE_BROWSER_EMULATION"), appName, 10000, KxREG_VALUE_DWORD);
		}
		else
		{
			KxRegistry::RemoveValue(KxREG_HKEY_CURRENT_USER, IERegPath wxS("FEATURE_BEHAVIORS"), appName);
			KxRegistry::RemoveValue(KxREG_HKEY_CURRENT_USER, IERegPath wxS("FEATURE_BROWSER_EMULATION"), appName);
		}
		#undef IERegPath
	}
	bool SystemApplication::QueueDownloadToMainProcess(const wxString& link) const
	{
		KxProcess process(KxLibrary(nullptr).GetFileName());
		if (process.Find())
		{
			for (HWND hWnd : process.EnumWindows())
			{
				if (KxTLWInternal::GetWindowUserData(hWnd) == KMainWindow::GetUniqueID())
				{
					COPYDATASTRUCT data = {0};
					data.lpData = (void*)link.wc_str();
					data.cbData = link.Length() * sizeof(WCHAR) + sizeof(WCHAR);

					::SendMessageW(hWnd, WM_COPYDATA, 0, (LPARAM)&data);
					return true;
				}
			}
		}
		return false;
	}

	wxLog* SystemApplication::GetLogger() const
	{
		return m_AppTraits->m_LogTarget;
	}
	void SystemApplication::CleanupLogs()
	{
		const size_t countToKeep = 10;

		KxFileFinder finder(m_Application->GetLogsFolder(), "*.log");
		std::vector<KxFileItem> items;
		items.reserve(countToKeep);

		KxFileItem item = finder.FindNext();
		while (item.IsOK())
		{
			if (item.IsNormalItem() && item.IsFile())
			{
				items.push_back(item);
			}
			item = finder.FindNext();
		}

		// Sort by creation date. Most recent first.
		std::sort(items.begin(), items.end(), [](const KxFileItem& v1, const KxFileItem& v2)
		{
			return v1.GetCreationTime() > v2.GetCreationTime();
		});

		// Delete all old logs
		for (size_t i = countToKeep; i < items.size(); i++)
		{
			KxFile(items[i].GetFullPath()).RemoveFile();
		}
	}
}

wxIMPLEMENT_APP(Kortex::SystemApplication);
