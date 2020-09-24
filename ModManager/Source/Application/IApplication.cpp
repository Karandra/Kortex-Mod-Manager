#include "pch.hpp"
#include "IApplication.h"
#include "SystemApplication.h"
#include "IMainWindow.h"
#include <kxf/System/TaskScheduler.h>
#include <kxf/System/DynamicLibrary.h>
#include <kxf/System/SystemInformation.h>
#include <wx/cmdline.h>

namespace Kortex::OName
{
	Kortex_DefOption(RestartDelay);
}

namespace Kortex
{
	IApplication& IApplication::GetInstance() noexcept
	{
		return *SystemApplication::GetInstance().GetApplication();
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
		return SystemApplication::GetInstance().ExamineCaughtException();
	}
	kxf::FSPath IApplication::GetRootDirectory() const
	{
		return SystemApplication::GetInstance().GetRootDirectory();
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
		return SystemApplication::GetInstance().IsAnotherInstanceRunning();
	}

	bool IApplication::QueueDownloadToMainProcess(const kxf::String& link)
	{
		//return SystemApplication::GetInstance().QueueDownloadToMainProcess(link);
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
		return SystemApplication::GetInstance().GetCommandLineParser();
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
				//delay = kxf::TimeSpan::Seconds(GetGlobalOption(OName::RestartDelay).GetValueInt(3));
			}

			ScheduledTask task = taskSheduler.NewTask();
			task.SetExecutable(kxf::DynamicLibrary::GetExecutingModule().GetFilePath(), commandLine);
			task.SetRegistrationTrigger("Restart", delay, kxf::DateTime::Now() + delay * 2);
			task.DeleteExpiredTaskAfter(delay);

			const kxf::String taskName = wxS("Kortex.ScheduleRestart");
			taskSheduler.DeleteTask(taskName);
			return taskSheduler.SaveTask(task, taskName).IsSuccess();
		}
		return false;
	}
	void IApplication::Exit(int exitCode)
	{
		SystemApplication::GetInstance().Exit(exitCode);
	}

	kxf::String IApplication::GetID() const
	{
		return SystemApplication::GetInstance().GetName();
	}
	kxf::String IApplication::GetName() const
	{
		return SystemApplication::GetInstance().GetDisplayName();
	}
	kxf::String IApplication::GetShortName() const
	{
		return SystemApplication::GetInstance().GetShortName();
	}
	kxf::String IApplication::GetDeveloper() const
	{
		return SystemApplication::GetInstance().GetVendorName();
	}
	kxf::Version IApplication::GetVersion() const
	{
		return SystemApplication::GetInstance().GetVersion();
	}
	kxf::XMLDocument& IApplication::GetGlobalConfig() const
	{
		//return SystemApplication::GetInstance().GetGlobalConfig();
	}

	wxWindow* IApplication::GetActiveWindow() const
	{
		return ::wxGetActiveWindow();
	}
	wxWindow* IApplication::GetTopWindow() const
	{
		return SystemApplication::GetInstance().GetTopWindow();
	}
	void IApplication::SetTopWindow(wxWindow* window)
	{
		return SystemApplication::GetInstance().SetTopWindow(window);
	}
	bool IApplication::IsActive() const
	{
		return SystemApplication::GetInstance().IsActive();
	}
	bool IApplication::IsMainWindowActive() const
	{
		if (IsActive())
		{
			if (const IMainWindow* mainWindow = IMainWindow::GetInstance())
			{
				return mainWindow->GetFrame().GetHandle() == ::GetForegroundWindow();
			}
			return true;
		}
		return false;
	}

	wxLog& IApplication::GetLogger()
	{
		return SystemApplication::GetInstance().GetLogger();
	}
	BroadcastProcessor& IApplication::GetBroadcastProcessor()
	{
		return SystemApplication::GetInstance().GetBroadcastProcessor();
	}
}
