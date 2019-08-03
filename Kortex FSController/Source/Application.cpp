#include "stdafx.h"
#include "Application.h"
#include "RecievingWindow.h"
#include "MainApplicationLink.h"
#include "VirtualFileSystem/FSControllerService.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxFile.h>

namespace
{
	template<class T> bool GetCmdArgIntValue(const wxCmdLineParser& parser, const wxString& name, T& value)
	{
		wxString stringValue;
		unsigned long long intValue = 0;
		if (parser.Found(name, &stringValue) && stringValue.ToULongLong(&intValue))
		{
			if constexpr(std::is_pointer_v<T>)
			{
				value = reinterpret_cast<T>(intValue);
			}
			else
			{
				value = static_cast<T>(intValue);
			}
			return true;
		}
		return false;
	}

	HWND GetMainAppWindow(const wxCmdLineParser& parser)
	{
		HWND hWnd = nullptr;
		GetCmdArgIntValue(parser, "HWND", hWnd);
		return hWnd;
	}
	DWORD GetMainAppProcessID(const wxCmdLineParser& parser)
	{
		DWORD pid = 0;
		GetCmdArgIntValue(parser, "PID", pid);
		return pid;
	}
}

namespace Kortex::FSController
{
	void Application::OnMainAppTerminates(wxProcessEvent& event)
	{
		m_RecievingWindow->Destroy();
		ExitApp();
	}

	Application::Application()
	{
		wxLog::EnableLogging(false);
		SetAppName(wxS("Kortex FSController"));

		// Setup paths
		m_RootFolder = KxLibrary(nullptr).GetFileName().BeforeLast('\\');
		KxFile::SetCWD(m_RootFolder);
		m_DataFolder = m_RootFolder + "\\Data";
		m_LogFolder = KxShell::GetFolder(KxSHF_APPLICATIONDATA_LOCAL) + wxS("\\KortexModManager\\Logs");

		// Configure command line
		wxCmdLineParser& parser = GetCmdLineParser();
		parser.SetSwitchChars("-");
		parser.AddOption("HWND");
		parser.AddOption("PID");
	}
	Application::~Application()
	{
	}

	bool Application::OnInit()
	{
		ParseCommandLine();
		HWND windowHandle = GetMainAppWindow(GetCmdLineParser());
		DWORD pid = GetMainAppProcessID(GetCmdLineParser());

		if (windowHandle != nullptr && pid != 0)
		{
			if (m_MainApp = std::make_unique<MainApplicationLink>(windowHandle); m_MainApp->IsOK())
			{
				m_MainProcess = std::make_unique<KxProcess>(pid);
				m_MainProcess->Bind(KxEVT_PROCESS_END, &Application::OnMainAppTerminates, this);
				m_MainProcess->SetOptionEnabled(KxPROCESS_WAIT_END);
				m_MainProcess->Attach(KxPROCESS_RUN_ASYNC);

				m_Service = std::make_unique<VirtualFileSystem::FSControllerService>();
				m_RecievingWindow = new RecievingWindow(*m_Service);
				SetTopWindow(m_RecievingWindow);

				m_MainApp->SetService(*m_Service);
				m_MainApp->SetRecievingWindow(m_RecievingWindow);
				m_Service->SetRecievingWindow(m_RecievingWindow);
				return true;
			}
		}
		return false;
	}
	int Application::OnExit()
	{
		if (m_MainApp->IsOK())
		{
			m_MainApp->SendExit();
		}
		return KxApp::OnExit();
	}
	
	bool Application::OnExceptionInMainLoop()
	{
		wxString message = "Unknown";
		try
		{
			throw;
		}
		catch (const std::exception& e)
		{
			message = e.what();
		}
		catch (const std::string& e)
		{
			message = e;
		}
		catch (const std::wstring& e)
		{
			message = e;
		}
		catch (const char* e)
		{
			message = e;
		}
		catch (const wchar_t* e)
		{
			message = e;
		}
		catch (...)
		{
			message = "unknown error.";
		}
		message = KxString::Format(wxS("Unexpected exception has occurred: %1.\r\n\r\nThe program will terminate."), message);

		if (m_MainApp && m_MainApp->IsOK())
		{
			m_MainApp->Send(IPC::RequestID::UnhandledException, message);
		}
		else
		{
			wxMessageBox(message);
		}

		// Exit the main loop and terminate the program.
		return false;
	}
	void Application::OnUnhandledException()
	{
		OnExceptionInMainLoop();
		ExitApp(std::numeric_limits<int>::min());
	}
}

wxIMPLEMENT_APP(Kortex::FSController::Application);
