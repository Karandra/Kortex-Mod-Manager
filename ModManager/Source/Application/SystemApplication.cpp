#include "pch.hpp"
#include "SystemApplication.h"
#include "DefaultApplication.h"
#include "Log.h"
#include "kxf/Application/ApplicationInitializer.h"
#include "kxf/FileSystem/NativeFileSystem.h"
#include "kxf/FileSystem/FSActionEvent.h"
#include "kxf/System/Win32Error.h"
#include "kxf/System/NtStatus.h"
#include "kxf/System/HResult.h"
#include "kxf/IO/StreamReaderWriter.h"
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
	const constexpr kxf::XChar Developer[] = wxS("Kerber");
	const constexpr kxf::XChar Version[] = wxS("2.0");

	const constexpr kxf::XChar GitCommitHash[] =
		#include "../Include/LatestCommit.txt"
		;
}

namespace Kortex
{
	// SystemApplication
	kxf::String SystemApplication::ExamineCaughtException() const
	{
		Log::Trace("Trying to extract message form current exception");

		kxf::String message;
		kxf::String type;
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
				message += wxS('\n');
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
			message = "Unknown";
			type = "Unknown";
		}

		kxf::String value = kxf::String::Format("Unexpected exception has occurred: %1.\n\nThe program will terminate.\n\nException type: %2", message, type);
		Log::FatalError(value);
		return value;
	}
	bool SystemApplication::OnException()
	{
		Log::Info("SystemApplication::OnException");

		if (!m_Application->OnException())
		{
			Log::Info("Exception info: %1", ExamineCaughtException());
			return false;
		}
		return true;
	}

	void SystemApplication::InitializeLogging()
	{
		class LogTarget final: public wxLog
		{
			private:
				std::unique_ptr<kxf::IOutputStream> m_Stream;

			protected:
				void DoLogText(const wxString& message) override
				{
					kxf::IO::OutputStreamWriter writer(*m_Stream);
					writer.WriteStringUTF8(message);
					writer.WriteStringUTF8(wxS('\n'));

					m_Stream->Flush();
				}

			public:
				LogTarget(std::unique_ptr<kxf::IOutputStream> stream)
					:m_Stream(std::move(stream))
				{
				}

			public:
				void Flush() override
				{
					m_Stream->Flush();
				}
		};

		kxf::IFileSystem& fs = m_Application->GetFileSystem(FileSystemOrigin::AppLogs);

		// Make a vlaid log file name
		kxf::String name = kxf::String::Format(wxS("Log[%1].log"), kxf::DateTime::Now().FormatISOCombined(' '));
		for (kxf::XChar c: fs.GetForbiddenPathNameCharacters())
		{
			name.Replace(c, wxS('.'));
		}

		// Create the log target or use a default one
		if (auto stream = fs.OpenToWrite(std::move(name), kxf::IOStreamDisposition::CreateAlways, kxf::IOStreamShare::Read, kxf::FSActionFlag::CreateDirectoryTree))
		{
			kxf::Log::SetActiveTarget(std::make_unique<LogTarget>(std::move(stream)));
			Log::Info("Log opened");
		}
		else
		{
			Log::Error("Couldn't open log file in write mode");
		}
	}
	void SystemApplication::InitializeFramework()
	{
		kxf::Drawing::InitalizeImageHandlers();
	}

	SystemApplication::SystemApplication() = default;
	SystemApplication::~SystemApplication() = default;

	// ICoreApplication
	bool SystemApplication::OnCreate()
	{
		// Set basic info
		SetName(SystemApplicationInfo::ID);
		SetDisplayName(SystemApplicationInfo::Name);
		SetVersion(SystemApplicationInfo::Version);
		SetVendorName(SystemApplicationInfo::Developer);

		// Initialize the root directory
		m_RootDirectory = kxf::NativeFileSystem::GetExecutingModuleRootDirectory();

		// Create default application
		m_Application = std::make_unique<DefaultApplication>();

		// Initialize main variables
		kxf::IVariablesCollection& variables = m_Application->GetVariables();
		variables.SetItem("App", "ID", SystemApplicationInfo::ID);
		variables.SetItem("App", "Name", SystemApplicationInfo::Name);
		variables.SetItem("App", "ShortName", SystemApplicationInfo::ShortName);
		variables.SetItem("App", "Developer", SystemApplicationInfo::Developer);
		variables.SetItem("App", "Version", SystemApplicationInfo::Version);
		variables.SetItem("App", "UniqueID", kxf::UniversallyUniqueID(kxf::RTTI::GetInterfaceID<IApplication>().ToNativeUUID()).ToString());
		variables.SetItem("App", "CommitHash", kxf::String(SystemApplicationInfo::GitCommitHash).Trim().Trim(kxf::StringOpFlag::FromEnd));
		variables.SetItem("App", "RootDirectory", m_RootDirectory);

		variables.SetItem("App", "Platform", m_Application->Is64Bit() ? "Win64" : "Win32");
		variables.SetItem("App", "Architecture", m_Application->Is64Bit() ? "x64" : "x86");

		variables.SetItem("System", "Platform", m_Application->IsSystem64Bit() ? "Win64" : "Win32");
		variables.SetItem("System", "Architecture", m_Application->IsSystem64Bit() ? "x64" : "x86");

		return true;
	}
	void SystemApplication::OnDestroy()
	{
		Log::Info("Log closed");
		kxf::Log::SetActiveTarget(nullptr);
	}
	bool SystemApplication::OnInit()
	{
		// Call creation function
		if (m_Application->OnCreate())
		{
			InitializeLogging();
			InitializeFramework();

			return m_Application->OnInit();
		}
		return false;
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
		Log::Debug("<SystemApplication::OnAssertFailure> %1:%2; [Function=%3][Condition=%4][Message=%5]", file, line, function, condition, message);
	}

	const kxf::ILocalizationPackage& SystemApplication::GetLocalizationPackage() const
	{
		return m_Application ? m_Application->GetLocalizationPackage() : m_EmptyLocalizationPackage;
	}

	// ICoreApplication -> ICommandLine
	size_t SystemApplication::EnumCommandLineArgs(std::function<bool(kxf::String)> func) const
	{
		return GUIApplication::EnumCommandLineArgs(std::move(func));
	}
	void SystemApplication::OnCommandLineInit(wxCmdLineParser& parser)
	{
		if (auto commandLine = m_Application->QueryInterface<kxf::Application::ICommandLine>())
		{
			commandLine->OnCommandLineInit(parser);
		}

		GUIApplication::OnCommandLineInit(parser);
	}
	bool SystemApplication::OnCommandLineParsed(wxCmdLineParser& parser)
	{
		bool result = false;
		if (auto commandLine = m_Application->QueryInterface<kxf::Application::ICommandLine>())
		{
			result = commandLine->OnCommandLineParsed(parser);
		}
		return result || GUIApplication::OnCommandLineParsed(parser);
	}
	bool SystemApplication::OnCommandLineError(wxCmdLineParser& parser)
	{
		bool result = false;
		if (auto commandLine = m_Application->QueryInterface<kxf::Application::ICommandLine>())
		{
			result = commandLine->OnCommandLineError(parser);
		}
		return result || GUIApplication::OnCommandLineError(parser);
	}
	bool SystemApplication::OnCommandLineHelp(wxCmdLineParser& parser)
	{
		bool result = false;
		if (auto commandLine = m_Application->QueryInterface<kxf::Application::ICommandLine>())
		{
			result = commandLine->OnCommandLineHelp(parser);
		}
		return result || GUIApplication::OnCommandLineHelp(parser);
	}

	// SystemApplication
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
