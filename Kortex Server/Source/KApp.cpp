#include "stdafx.h"
#include "KApp.h"
#include "IPC/KIPCServer.h"
#include "IPC/KIPCConnection.h"
#include "IPC/KIPCRequest.h"
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxLibrary.h>

KApp::KApp()
{
	SetAppName("Kortex Server");

	wxCmdLineParser& cmdLineParser = GetCmdLineParser();
	cmdLineParser.AddOption("NXM", wxEmptyString, "NXM link like: nxm://Skyrim/mods/100/files/42");
}
KApp::~KApp()
{
}

bool KApp::OnInit()
{
	ParseCommandLine();
	wxLog::EnableLogging(false);

	m_Server = new KIPCServer();
	return true;
}
int KApp::OnExit()
{
	delete m_Server;
	return KxApp::OnExit();
}
bool KApp::OnExceptionInMainLoop()
{
	wxString sMessage = "Unknown";
	wxString sOrigin = "Unknown";
	try
	{
		throw;
	}
	catch (const std::exception& e)
	{
		sOrigin = "C++ Standard Library";
		sMessage = e.what();
	}
	catch (const std::string& e)
	{
		sOrigin = "C++ std::string";
		sMessage = e;
	}
	catch (const std::wstring& e)
	{
		sOrigin = "C++ std::wstring";
		sMessage = e;
	}
	catch (const char* e)
	{
		sOrigin = "C Narrow String";
		sMessage = e;
	}
	catch (const wchar_t* e)
	{
		sOrigin = "C Wide String";
		sMessage = e;
	}
	catch (...)
	{
		sMessage = "unknown error.";
	}
	wxMessageBox(wxString::Format("Unexpected exception has occurred: %s.\r\n\r\nThe program will terminate.\r\n\r\nException origin: %s", sMessage, sOrigin));
	ExitApp(std::numeric_limits<int>::min());

	// Exit the main loop and terminate the program.
	return false;
}
void KApp::OnUnhandledException()
{
	OnExceptionInMainLoop();
	ExitApp(std::numeric_limits<int>::min());
}

const wxString& KApp::GetDataFolder()
{
	static const wxString ms_DataFolder("Data");
	return ms_DataFolder;
}

wxIMPLEMENT_APP(KApp);
