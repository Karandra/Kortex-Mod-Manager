#include "pch.hpp"
#include "IApplication.h"
#include "SystemApplication.h"
#include <kxf/System/TaskScheduler.h>
#include <kxf/System/SystemInformation.h>
#include <wx/cmdline.h>

namespace Kortex::OName
{
	//KortexDefOption(RestartDelay);
}

namespace Kortex
{
	SystemApplication& IApplication::GetSystemApp() noexcept
	{
		return SystemApplication::GetInstance();
	}
	IApplication& IApplication::GetInstance() noexcept
	{
		return *GetSystemApp().GetApplication();
	}

	void IApplication::OnConfigureCommandLine()
	{
		wxCmdLineParser& parser = GetCommandLineParser();

		parser.SetSwitchChars("-");
		//cmdLineParser.AddOption(CmdLineName::InstanceID, {}, "Instance ID");
		//cmdLineParser.AddOption(CmdLineName::GlobalConfigPath, {}, "Folder path for app-wide config");
		//cmdLineParser.AddOption(CmdLineName::DownloadLink, {}, "Download link");
	}

	kxf::String IApplication::ExamineCaughtException() const
	{
		return GetSystemApp().ExamineCaughtException();
	}

	kxf::String IApplication::GetRootFolder() const
	{
		//return GetSystemApp().GetRootFolder();
	}
	kxf::String IApplication::GetExecutablePath() const
	{
		//return GetSystemApp().GetExecutablePath();
	}
	kxf::String IApplication::GetExecutableName() const
	{
		//return GetSystemApp().GetExecutableName();
	}

	bool IApplication::Is64Bit() const
	{
		#if defined _WIN64
		return true;
		#else
		return false;
		#endif
	}
	bool IApplication::IsSystem64Bit() const
	{
		return kxf::System::Is64Bit();
	}
	bool IApplication::IsAnotherInstanceRunning() const
	{
		return GetSystemApp().IsAnotherInstanceRunning();
	}

	bool IApplication::QueueDownloadToMainProcess(const kxf::String& link)
	{
		//return GetSystemApp().QueueDownloadToMainProcess(link);
	}
	std::optional<kxf::String> IApplication::GetLinkFromCommandLine() const
	{
		#if 0
		using namespace Application;

		kxf::String link;
		if (GetCommandLineParser().Found(CmdLineName::DownloadLink, &link) && !link.IsEmpty())
		{
			return link;
		}
		return {};
		#endif
	}

	wxCmdLineParser& IApplication::GetCommandLineParser() const
	{
		return GetSystemApp().GetCommandLineParser();
	}
	kxf::String IApplication::FormatCommandLine(const CmdLineParameters& parameters)
	{
	}
	bool IApplication::ScheduleRestart(const kxf::String& commandLine, std::optional<kxf::TimeSpan> timeout)
	{
		using namespace kxf::System;

		TaskScheduler taskSheduler;
		if (taskSheduler)
		{
			kxf::TimeSpan delay;
			if (timeout && timeout->IsPositive())
			{
				delay = *timeout;
			}
			else
			{
				//delay = wxTimeSpan::Seconds(GetGlobalOption(OName::RestartDelay).GetValueInt(3));
			}

			ScheduledTask task = taskSheduler.NewTask();
			task.SetExecutable(GetExecutablePath(), commandLine);
			task.SetRegistrationTrigger("Restart", delay, wxDateTime::Now() + delay * 2);
			task.DeleteExpiredTaskAfter(delay);

			const kxf::String taskName = wxS("Kortex.ScheduleRestart");
			taskSheduler.DeleteTask(taskName);
			return taskSheduler.SaveTask(task, taskName).IsSuccess();
		}
		return false;
	}
	void IApplication::Exit(int exitCode)
	{
		GetSystemApp().Exit(exitCode);
	}

	kxf::String IApplication::GetID() const
	{
		return GetSystemApp().GetName();
	}
	kxf::String IApplication::GetName() const
	{
		return GetSystemApp().GetDisplayName();
	}
	kxf::String IApplication::GetShortName() const
	{
		return GetSystemApp().GetShortName();
	}
	kxf::String IApplication::GetDeveloper() const
	{
		return GetSystemApp().GetVendorName();
	}
	kxf::Version IApplication::GetVersion() const
	{
		return GetSystemApp().GetVersion();
	}
	kxf::Version IApplication::GetWxWidgetsVersion() const
	{
		return wxGetLibraryVersionInfo();
	}
	kxf::XMLDocument& IApplication::GetGlobalConfig() const
	{
		//return GetSystemApp().GetGlobalConfig();
	}

	wxWindow* IApplication::GetActiveWindow() const
	{
		return ::wxGetActiveWindow();
	}
	wxWindow* IApplication::GetTopWindow() const
	{
		return GetSystemApp().GetTopWindow();
	}
	void IApplication::SetTopWindow(wxWindow* window)
	{
		return GetSystemApp().SetTopWindow(window);
	}
	bool IApplication::IsActive() const
	{
		return GetSystemApp().IsActive();
	}
	bool IApplication::IsMainWindowActive() const
	{
		if (IsActive())
		{
			#if 0
			if (const IMainWindow* mainWindow = IMainWindow::GetInstance())
			{
				return mainWindow->GetFrame().GetHandle() == ::GetForegroundWindow();
			}
			#endif
			return true;
		}
		return false;
	}

	wxLog& IApplication::GetLogger()
	{
		return GetSystemApp().GetLogger();
	}
	BroadcastProcessor& IApplication::GetBroadcastProcessor()
	{
		return GetSystemApp().GetBroadcastProcessor();
	}
}
