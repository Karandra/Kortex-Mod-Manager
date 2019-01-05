#include "stdafx.h"
#include "Application.h"
#include "RecievingWindow.h"
#include "MainApplicationLink.h"
#include "VirtualFileSystem/FSControllerService.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxFile.h>

namespace
{
	HWND GetMainAppWIndow(const wxCmdLineParser& parser)
	{
		wxString value;
		unsigned long long intValue = 0;
		if (parser.Found("HWND", &value) && value.ToULongLong(&intValue))
		{
			return reinterpret_cast<HWND>(intValue);
		}
		return nullptr;
	}
}

namespace Kortex::FSController
{
	Application::Application()
	{
		wxLog::EnableLogging(false);
		SetAppName(wxS("Kortex FSController"));

		// Setup paths
		m_RootFolder = KxLibrary(nullptr).GetFileName().BeforeLast('\\');
		KxFile::SetCWD(m_RootFolder);
		m_DataFolder = m_RootFolder + "\\Data";

		// Configure command line
		wxCmdLineParser& parser = GetCmdLineParser();
		parser.SetSwitchChars("-");
		parser.AddOption("HWND");
	}
	Application::~Application()
	{
	}

	bool Application::OnInit()
	{
		ParseCommandLine();

		HWND windowHandle = GetMainAppWIndow(GetCmdLineParser());
		m_MainApp = std::make_unique<MainApplicationLink>(windowHandle);
		if (m_MainApp->IsOK())
		{
			m_Service = std::make_unique<VirtualFileSystem::FSControllerService>();
			m_RecievingWindow = new RecievingWindow(*m_Service);
			SetTopWindow(m_RecievingWindow);

			m_MainApp->SetService(*m_Service);
			m_MainApp->SetRecievingWindow(m_RecievingWindow);
			m_Service->SetRecievingWindow(m_RecievingWindow);
			return true;
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
		wxMessageBox(KxString::Format(wxS("Unexpected exception has occurred: %1.\r\n\r\nThe program will terminate."), message));

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
