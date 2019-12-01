#include "stdafx.h"
#include "SystemApplication.h"
#include "DefaultApplication.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Theme.hpp>
#include <Kortex/Git/Constants.h>
#include "Theme/Default.h"
#include "Theme/VisualStudio.h"
#include "MainWindow.h"
#include "Archive/KArchive.h"
#include "Utility/KAux.h"
#include "Utility/Log.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxRegistry.h>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxSystem.h>

namespace
{
	template<class... Args> auto FormatMessage(Args&&... arg)
	{
		return FormatMessageW(std::forward<Args>(arg)...);
	}
}

#pragma push_macro("NULL")
#undef NULL
#define NULL nullptr
#include <comdef.h>
#pragma pop_macro("NULL")

namespace
{
	void LogConfigChange(const Kortex::AppOption& option)
	{
		using namespace Kortex::Utility::Log;

		LogInfo(wxS("Path: \"%1\", Value: \"%2\""), option.GetXPath(), option.GetValue());
	}
}

namespace Kortex
{
	FILE* SystemApplicationTraits::CreateLogFile() const
	{
		wxString fileName = wxDateTime::Now().Format(wxS("%Y-%m-%d %H-%M-%S")) + wxS(".log");
		wxString filePath = m_SystemApp.m_Application->GetLogsFolder();
		wxFileName::Mkdir(filePath, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

		wxString fullPath = filePath + wxS('\\') + fileName;

		FILE* handle = nullptr;
		::_wfopen_s(&handle, fullPath.wc_str(), L"w+b");
		return handle;
	}

	SystemApplicationTraits::SystemApplicationTraits(SystemApplication& systemApp)
		:m_SystemApp(systemApp)
	{
	}

	wxLog* SystemApplicationTraits::CreateLogTarget()
	{
		m_LogTargetFILE = CreateLogFile();
		m_LogTarget = new wxLogStderr(m_LogTargetFILE);
		return m_LogTarget;
	}
}

namespace Kortex
{
	namespace SystemApplicationInfo
	{
		const constexpr wxChar ID[] = wxS("KortexModManager");
		const constexpr wxChar Name[] = wxS("Kortex Mod Manager");
		const constexpr wxChar ShortName[] = wxS("Kortex");
		const constexpr wxChar Version[] = wxS("2.0a8");
		const constexpr wxChar Developer[] = wxS("Kerber");
		const constexpr wxChar GUID[] = wxS("B5E8047C-9239-45C4-86F6-6C83A842063E");

		template<class T>
		wxString RemoveWhitespace(T&& value)
		{
			wxString data(value, std::size(value) - 1);
			return KxString::Trim(data, true, true);
		}
	}

	void SystemApplication::InitLogging()
	{
		m_BroadcastReciever.Bind(LogEvent::EvtInfo, &SystemApplication::OnError, this);
		m_BroadcastReciever.Bind(LogEvent::EvtError, &SystemApplication::OnError, this);
		m_BroadcastReciever.Bind(LogEvent::EvtWarning, &SystemApplication::OnError, this);
		m_BroadcastReciever.Bind(LogEvent::EvtCritical, &SystemApplication::OnError, this);
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
		m_NotificationCenter = std::make_unique<Notifications::DefaultNotificationCenter>();
	}
	void SystemApplication::UninitComponents()
	{
		Utility::Log::LogInfo("SystemApplication::UninitComponents");

		KArchive::UnInit();
	}
	void SystemApplication::SetPostCreateVariables()
	{
		IVariableTable& variables = m_Application->GetVariables();

		variables.SetVariable(Variables::KVAR_APP_SETTINGS_DIR, m_Application->GetUserSettingsFolder());
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
	}
	void SystemApplication::TerminateActiveInstance()
	{
		Utility::Log::LogInfo("SystemApplication::TerminateActiveInstance");
		
		IGameInstance* instance = IGameInstance::GetActive();
		IConfigurableGameInstance* configurableInstance = nullptr;
		if (instance && instance->QueryInterface(configurableInstance))
		{
			Utility::Log::LogInfo("Calling IConfigurableGameInstance::OnExit");
			configurableInstance->OnExit();
		}

		Utility::Log::LogInfo("SystemApplication::TerminateActiveInstance -> Destroying active instance");
		m_ActiveGameInstance.reset();
		m_ShallowGameInstances.clear();
		m_GameInstanceTemplates.clear();
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

		// Configure command line and parse it
		m_Application->OnConfigureCommandLine();
		ParseCommandLine();

		SetPostCreateVariables();

		// Initialize logging
		InitLogging();
		Utility::Log::LogInfo("SystemApplication::OnInit");

		// Log system info
		KxSystem::VersionInfo versionInfo = KxSystem::GetVersionInfo();
		Utility::Log::LogInfo("System: %1 %2 %3. Kernel version: %4.%5", KxSystem::GetName(), m_Application->IsSystem64Bit() ? "x64" : "x86", versionInfo.ServicePack, versionInfo.Kernel.Major, versionInfo.Kernel.Minor);
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
		SaveGlobalConfig();
		TerminateActiveInstance();

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
		Utility::Log::LogInfo("An error occurred:\r\n\r\n%1", event.GetString());

		m_Application->OnError(event);
	}
	
	void SystemApplication::OnGlobalConfigChanged(AppOption& option)
	{
		Utility::Log::LogInfo("SystemApplication::OnGlobalConfigChanged");
		LogConfigChange(option);

		m_Application->OnGlobalConfigChanged(option);
	}
	void SystemApplication::OnInstanceConfigChanged(AppOption& option, IGameInstance& instance)
	{
		Utility::Log::LogInfo("SystemApplication::OnInstanceConfigChanged");
		Utility::Log::LogInfo("InstanceID: %1", instance.GetInstanceID());
		LogConfigChange(option);

		// If this function called, it's certainly 'IConfigurableGameInstance'
		instance.QueryInterface<IConfigurableGameInstance>()->OnConfigChanged(option);
		m_Application->OnInstanceConfigChanged(option, instance);
	}
	void SystemApplication::OnProfileConfigChanged(AppOption& option, IGameProfile& profile)
	{
		Utility::Log::LogInfo("SystemApplication::OnProfileConfigChanged");
		Utility::Log::LogInfo("ProfileID: %1", profile.GetID());
		LogConfigChange(option);

		profile.OnConfigChanged(option);
		m_Application->OnProfileConfigChanged(option, profile);
	}

	IGameInstance* SystemApplication::GetActiveGameInstance()
	{
		return m_ActiveGameInstance.get();
	}
	void SystemApplication::AssignActiveGameInstance(std::unique_ptr<IGameInstance> instance)
	{
		m_ActiveGameInstance = std::move(instance);
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
	void SystemApplication::OnFatalException()
	{
		Utility::Log::LogInfo("SystemApplication::OnFatalException");
		
		OnException();
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
		catch (const _com_error& e)
		{
			type = "_com_error";
			message = e.ErrorMessage();
			if (!message.IsEmpty())
			{
				message += wxS("\r\n");
				message += e.Description().GetBSTR();
			}
			else
			{
				message == e.Description().GetBSTR();
			}
		}
		catch (NTSTATUS e)
		{
			type = "HRESULT/NTSTATUS";
			message = KxFormat("%1: 0x%2")(type)(e, 0, 16);
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
			// the 'wxExit' will call 'abort(-1)' anyway, but we need to exit with our own exit code.

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
		if (KxProcess process(m_Application->GetExecutableName()); process.Find())
		{
			for (HWND handle: process.EnumWindows())
			{
				if (KxTLWInternal::GetWindowUserData(handle) == Application::MainWindow::GetUniqueID())
				{
					COPYDATASTRUCT data = {0};
					data.lpData = const_cast<wchar_t*>(link.wc_str());
					data.dwData = link.length();
					data.cbData = link.length() * sizeof(wchar_t) + sizeof(wchar_t);

					::SendMessageW(handle, WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&data));
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
		constexpr size_t countToKeep = 10;

		KxFileFinder finder(m_Application->GetLogsFolder(), wxS("*.log"));
		std::vector<KxFileItem> items;
		items.reserve(countToKeep);

		for (KxFileItem item = finder.FindNext(); item.IsOK(); item = finder.FindNext())
		{
			if (item.IsNormalItem() && item.IsFile())
			{
				items.push_back(item);
			}
		}

		// Sort by creation date. Most recent first.
		std::sort(items.begin(), items.end(), [](const KxFileItem& left, const KxFileItem& right)
		{
			return left.GetCreationTime() > right.GetCreationTime();
		});

		// Delete all old logs
		for (size_t i = countToKeep; i < items.size(); i++)
		{
			KxFile(items[i].GetFullPath()).RemoveFile();
		}
	}
}

wxIMPLEMENT_APP(Kortex::SystemApplication);
