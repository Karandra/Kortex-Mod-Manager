#include "stdafx.h"
#include "SystemApplication.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Theme.hpp>
#include <Kortex/Events.hpp>
#include <Kortex/Git/Constants.h>
#include "Archive/KArchive.h"
#include "UI/KMainWindow.h"
#include "Utility/KAux.h"
#include "Utility/Log.h"
#include <KxFramework/KxFile.h>
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
		Utility::Log::LogInfo(wxS("Path: \"%1\", Value: \"%2\""), option.GetXPath(), option.GetValue());
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
		const constexpr wxChar ShortName[] = wxS("Kortex");
		const constexpr wxChar Version[] = wxS("2.0");
		const constexpr wxChar Developer[] = wxS("Kerber");
		const constexpr wxChar GUID[] = wxS("B5E8047C-9239-45C4-86F6-6C83A842063E");

		template<class T> wxString RemoveWhitespace(T&& value)
		{
			wxString data(value, std::size(value) - 1);
			return KxString::Trim(data, true, true);
		}
	}

	void SystemApplication::InitLogging()
	{
		Bind(Events::Log, &SystemApplication::OnError, this);
		wxLog::SetVerbose(true);

		Utility::Log::LogInfo("%1 v%2: Log opened", SystemApplicationInfo::Name, SystemApplicationInfo::Version);
	}
	void SystemApplication::UninitLogging()
	{
		Utility::Log::LogInfo("Log closed");
		wxLog::FlushActive();
		CleanupLogs();
	}
	
	void SystemApplication::InitComponents()
	{
		Utility::Log::LogInfo("SystemApplication::InitComponents");

		KArchive::Init();
		m_ThemeManager = std::make_unique<Theme::Default>();
		m_NotificationCenter = std::make_unique<Notification::DefaultNotificationCenter>();
	}
	void SystemApplication::UninitComponents()
	{
		Utility::Log::LogInfo("SystemApplication::UninitComponents");

		KArchive::UnInit();
	}

	void SystemApplication::LoadGlobalConfig()
	{
		Utility::Log::LogInfo("SystemApplication::LoadGlobalConfig");

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

		Utility::Log::LogInfo("SystemApplication::LoadGlobalConfig -> %1", success ? "true" : "false");
	}
	void SystemApplication::SaveGlobalConfig()
	{
		Utility::Log::LogInfo("SystemApplication::SaveGlobalConfig");

		KxFileStream stream(m_Application->GetUserSettingsFile(), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
		const bool success = m_GlobalConfig.Save(stream);

		Utility::Log::LogInfo("SystemApplication::SaveGlobalConfig -> %1", success ? "true" : "false");

		Utility::Log::LogInfo("SystemApplication::SaveGlobalConfig -> Destroying active instance");
		IGameInstance::DestroyActive();
	}
	void SystemApplication::SaveActiveInstanceSettings()
	{
		Utility::Log::LogInfo("SystemApplication::SaveActiveInstanceSettings");
		
		IGameInstance* instance = IGameInstance::GetActive();
		IConfigurableGameInstance* configurableInstance = nullptr;
		if (instance && instance->QueryInterface(configurableInstance))
		{
			Utility::Log::LogInfo("Calling IConfigurableGameInstance::OnExit");
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

		// Initialize .exe path and root folder
		m_ExecutablePath = KxLibrary(nullptr).GetFileName();
		m_RootFolder = m_ExecutablePath.BeforeLast(wxS('\\'), &m_ExecutableName);
		KxFile::SetCWD(m_RootFolder);

		// Create default application
		m_Application = std::make_unique<Application::DefaultApplication>();

		// Initialize main variables
		IVariableTable& variables = m_Application->GetVariables();
		variables.SetVariable("AppID", SystemApplicationInfo::ID);
		variables.SetVariable("AppName", SystemApplicationInfo::Name);
		variables.SetVariable("AppShortName", SystemApplicationInfo::ShortName);
		variables.SetVariable("AppVersion", SystemApplicationInfo::Version);
		variables.SetVariable("AppDeveloper", SystemApplicationInfo::Developer);
		variables.SetVariable("AppGUID", SystemApplicationInfo::GUID);
		variables.SetVariable("AppCommitHash", SystemApplicationInfo::RemoveWhitespace(SystemApplicationInfo::GitCommitHash));
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
		// Call creation function
		m_Application->OnCreate();

		// Initialize logging
		InitLogging();
		Utility::Log::LogInfo("SystemApplication::OnInit");

		// Log system info
		KxSystem::VersionInfo versionInfo = KxSystem::GetVersionInfo();
		Utility::Log::LogInfo("System: %1 %2 %3. Kernel version: %4.%5", KxSystem::GetName(), m_Application->IsSystem64Bit() ? "x64" : "x86", versionInfo.ServicePack, versionInfo.MajorVersion, versionInfo.MinorVersion);
		Utility::Log::LogInfo("Another instance detected: %1", IsAnotherRunning() ? "true" : "false");

		// Log paths
		Utility::Log::LogInfo("Root folder: %1", m_Application->GetRootFolder());
		Utility::Log::LogInfo("Data folder: %1", m_Application->GetDataFolder());
		Utility::Log::LogInfo("Logs folder: %1", m_Application->GetLogsFolder());
		Utility::Log::LogInfo("User settings folder: %1", m_Application->GetUserSettingsFolder());
		Utility::Log::LogInfo("User settings file: %1", m_Application->GetUserSettingsFile());
		Utility::Log::LogInfo("Instances folder: %1", m_Application->GetInstancesFolder());

		// Run initialization
		LoadGlobalConfig();
		InitComponents();
		m_IsApplicationInitialized = m_Application->OnInit();

		Utility::Log::LogInfo("Initializing app: %1", m_IsApplicationInitialized ? "true" : "false");
		if (!m_IsApplicationInitialized)
		{
			UninitLogging();
		}
		return m_IsApplicationInitialized;
	}
	int SystemApplication::OnExit()
	{
		Utility::Log::LogInfo("SystemApplication::OnExit");

		if (m_IsApplicationInitialized)
		{
			m_ExitCode = m_Application->OnExit();
		}
		else
		{
			Utility::Log::LogInfo("Exiting before initialization is completed");
		}
		Utility::Log::LogInfo("Exit code: %1", m_ExitCode);
		
		UninitComponents();
		SaveActiveInstanceSettings();
		SaveGlobalConfig();

		// Close log now, it will be closed after call to 'KxApp::OnExit' anyway.
		UninitLogging();

		// Exit now
		if (m_IsApplicationInitialized)
		{
			KxApp::OnExit();
		}
		return HasExternalExitCode() ? GetExternalExitCode() : m_ExitCode;
	}
	void SystemApplication::OnError(LogEvent& event)
	{
		Utility::Log::LogInfo("SystemApplication::OnError");
		Utility::Log::LogInfo("An error occurred:\r\n\r\n%1", event.GetMessage());

		m_Application->OnError(event);
	}
	
	void SystemApplication::OnGlobalConfigChanged(IAppOption& option)
	{
		Utility::Log::LogInfo("SystemApplication::OnGlobalConfigChanged");
		LogConfigChnage(option);

		m_Application->OnGlobalConfigChanged(option);
	}
	void SystemApplication::OnInstanceConfigChanged(IAppOption& option, IGameInstance& instance)
	{
		Utility::Log::LogInfo("SystemApplication::OnInstanceConfigChanged");
		Utility::Log::LogInfo("InstanceID: %1", instance.GetInstanceID());
		LogConfigChnage(option);

		// If this function called, it's certainly 'IConfigurableGameInstance'
		instance.QueryInterface<IConfigurableGameInstance>()->OnConfigChanged(option);
		m_Application->OnInstanceConfigChanged(option, instance);
	}
	void SystemApplication::OnProfileConfigChanged(IAppOption& option, IGameProfile& profile)
	{
		Utility::Log::LogInfo("SystemApplication::OnProfileConfigChanged");
		Utility::Log::LogInfo("ProfileID: %1", profile.GetID());
		LogConfigChnage(option);

		profile.OnConfigChanged(option);
		m_Application->OnProfileConfigChanged(option, profile);
	}

	bool SystemApplication::OnException()
	{
		Utility::Log::LogInfo("SystemApplication::OnException");
		if (wxIsDebuggerRunning())
		{
			Utility::Log::LogInfo("Debugger is running, rethrowing exception");

			throw;
			return false;
		}

		if (!m_Application->OnException())
		{
			Utility::Log::LogInfo("Exception info: %1", RethrowCatchAndGetExceptionInfo());
			return false;
		}
		return true;
	}
	bool SystemApplication::OnExceptionInMainLoop()
	{
		Utility::Log::LogInfo("SystemApplication::OnExceptionInMainLoop");

		// Exit the main loop and terminate the program if 'OnException' return false.
		return OnException();
	}
	void SystemApplication::OnUnhandledException()
	{
		Utility::Log::LogInfo("SystemApplication::OnUnhandledException");

		if (!OnException())
		{
			constexpr int exitCode = std::numeric_limits<int>::min();
			Utility::Log::LogInfo("Terminating with code: %1", exitCode);
			ExitApp(exitCode);
		}
	}
	wxString SystemApplication::RethrowCatchAndGetExceptionInfo() const
	{
		Utility::Log::LogError("Trying to extract message form current exception");

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

		wxString value = KxString::Format("Unexpected exception has occurred: %1.\r\n\r\nThe program will terminate.\r\n\r\nException type: %2", message, type);
		Utility::Log::LogFatalError(value);
		return value;
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
		#define IERegPath	wxS("SOFTWARE\\Microsoft\\Internet Explorer\\MAIN\\FeatureControl\\")
		if (init)
		{
			KxRegistry::SetValue(KxREG_HKEY_CURRENT_USER, IERegPath wxS("FEATURE_BEHAVIORS"), m_ExecutableName, 1, KxREG_VALUE_DWORD);
			KxRegistry::SetValue(KxREG_HKEY_CURRENT_USER, IERegPath wxS("FEATURE_BROWSER_EMULATION"), m_ExecutableName, 10000, KxREG_VALUE_DWORD);
		}
		else
		{
			KxRegistry::RemoveValue(KxREG_HKEY_CURRENT_USER, IERegPath wxS("FEATURE_BEHAVIORS"), m_ExecutableName);
			KxRegistry::RemoveValue(KxREG_HKEY_CURRENT_USER, IERegPath wxS("FEATURE_BROWSER_EMULATION"), m_ExecutableName);
		}
		#undef IERegPath
	}
	bool SystemApplication::QueueDownloadToMainProcess(const wxString& link) const
	{
		KxProcess process(KxLibrary(nullptr).GetFileName());
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
					return true;
				}
			}
		}
		return false;
	}

	wxString SystemApplication::GetShortName() const
	{
		return SystemApplicationInfo::ShortName;
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
