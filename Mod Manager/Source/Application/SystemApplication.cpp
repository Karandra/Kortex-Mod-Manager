#include "pch.hpp"
#include "SystemApplication.h"
#include "IApplication.h"
#include "Log.h"
#include "kxf/Application/ApplicationInitializer.h"
#include "kxf/FileSystem/NativeFileSystem.h"
#include "kxf/FileSystem/FSActionEvent.h"
#include "kxf/System/Win32Error.h"
#include "kxf/System/NtStatus.h"
#include "kxf/System/HResult.h"
#include <wx/msgdlg.h>

#include "kxf/System/Private/BeginIncludeCOM.h"
namespace
{
	Kx_MakeWinUnicodeCallWrapper(FormatMessage);
}
#include <comdef.h>
#include "kxf/System/Private/EndIncludeCOM.h"

namespace Kortex::SystemApplicationInfo
{
	const constexpr kxf::XChar ID[] = wxS("Kortex.ModManager");
	const constexpr kxf::XChar Name[] = wxS("Kortex Mod Manager");
	const constexpr kxf::XChar ShortName[] = wxS("Kortex");
	const constexpr kxf::XChar Version[] = wxS("2.0");
	const constexpr kxf::XChar Developer[] = wxS("Kerber");
	const constexpr kxf::XChar GUID[] = wxS("B5E8047C-9239-45C4-86F6-6C83A842063E");
}

namespace Kortex
{
	// SystemApplication
	kxf::String SystemApplication::ExamineCaughtException() const
	{
		Log::Trace("Trying to extract message form current exception");

		kxf::String message = "Unknown";
		kxf::String type = "Unknown";
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
		catch (const kxf::Win32Error& e)
		{
			type = "kxf::Win32Error";
			message = e.GetMessage();
		}
		catch (const kxf::NtStatus& e)
		{
			type = "kxf::NtStatus";
			message = e.GetMessage();
		}
		catch (const kxf::HResult& e)
		{
			type = "kxf::HResult";
			message = e.GetMessage();
		}
		catch (const kxf::IErrorCode& e)
		{
			type = "kxf::IErrorCode";
			message = e.GetMessage();
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

		kxf::String value = kxf::String::Format("Unexpected exception has occurred: %1.\r\n\r\nThe program will terminate.\r\n\r\nException type: %2", message, type);
		Log::FatalError(value);
		return value;
	}
	bool SystemApplication::OnException()
	{
		Log::Info("SystemApplication::OnException");
		if (wxIsDebuggerRunning())
		{
			Log::Info("Debugger is running, rethrowing exception");

			throw;
			return false;
		}

		if (!m_Application->OnException())
		{
			Log::Info("Exception info: %1", ExamineCaughtException());
			return false;
		}
		return true;
	}

	bool SystemApplication::OnCreate()
	{
		// Set basic info
		SetName(SystemApplicationInfo::ID);
		SetDisplayName(SystemApplicationInfo::Name);
		SetVersion(SystemApplicationInfo::Version);
		SetVendorName(SystemApplicationInfo::Developer);

		// Initialize root folder
		m_RootDirectory = kxf::NativeFileSystem::GetExecutableDirectory();

		// Create default application
		//m_Application = std::make_unique<Application::DefaultApplication>();

		// Initialize main variables
		kxf::IVariablesCollection& variables = m_Application->GetVariables();
		variables.SetItem("AppID", SystemApplicationInfo::ID);
		variables.SetItem("AppName", SystemApplicationInfo::Name);
		variables.SetItem("AppShortName", SystemApplicationInfo::ShortName);
		variables.SetItem("AppVersion", SystemApplicationInfo::Version);
		variables.SetItem("AppDeveloper", SystemApplicationInfo::Developer);
		variables.SetItem("AppGUID", SystemApplicationInfo::GUID);
		//variables.SetItem("AppCommitHash", SystemApplicationInfo::RemoveWhitespace(SystemApplicationInfo::GitCommitHash));
		variables.SetItem("AppModProjectProgID", kxf::String::Format("%1.Project.1", SystemApplicationInfo::ID));
		variables.SetItem("AppModPackageProgID", kxf::String::Format("%1.Package.1", SystemApplicationInfo::ID));
		variables.SetItem("AppData", m_Application->GetDataFolder());
		//variables.SetItem("SystemArchitecture", Utility::ArchitectureToNumber(m_Application->IsSystem64Bit()));
		//variables.SetItem("SystemArchitectureName", Utility::ArchitectureToString(m_Application->IsSystem64Bit()));

		return true;
	}
	bool SystemApplication::OnInit()
	{
		// Call creation function
		m_Application->OnCreate();

		// Configure command line and parse it
		m_Application->OnConfigureCommandLine();

		return true;
	}

	void SystemApplication::OnFatalException()
	{
		Log::Info("SystemApplication::OnFatalException");

		OnException();
	}
	bool SystemApplication::OnMainLoopException()
	{
		Log::Info("SystemApplication::OnMainLoopException");

		// Exit the main loop and terminate the program if 'OnException' return false.
		return OnException();
	}
	void SystemApplication::OnUnhandledException()
	{
		Log::Info("SystemApplication::OnUnhandledException");

		if (!OnException())
		{
			int exitCode = GetExitCode().value_or(std::numeric_limits<int>::min());
			Log::Info("Terminating with code: %1", exitCode);
			Exit(exitCode);
		}
	}
	void SystemApplication::OnAssertFailure(kxf::String file, int line, kxf::String function, kxf::String condition, kxf::String message)
	{
		wxMessageBox("OnAssertFailure", file);
	}

	kxf::String SystemApplication::GetShortName() const
	{
		return SystemApplicationInfo::ShortName;
	}
}

int main(int argc, char** argv)
{
	Kortex::SystemApplication app;
	return kxf::ApplicationInitializer(app, argc, argv).Run();
}
