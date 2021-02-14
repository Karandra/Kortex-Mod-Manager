#include "pch.hpp"
#include "IApplication.h"
#include "SystemApplication.h"
#include "IModule.h"
#include "IMainWindow.h"
#include "GameDefinition/IGameInstance.h"
#include "GameDefinition/IGameDefinition.h"
#include "Options/Macros.h"
#include "Options/CmdLineDatabase.h"
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

	kxf::String IApplication::ExamineCaughtException() const
	{
		return SystemApplication::GetInstance().ExamineCaughtException();
	}

	IModule* IApplication::GetModuleByID(const kxf::IID& iid)
	{
		for (IModule& module: EnumModules())
		{
			if (module.QueryInterface<kxf::RTTI::ClassInfo>()->GetIID() == iid)
			{
				return &module;
			}
		}
		return nullptr;
	}

	IGameDefinition* IApplication::GetGameDefinitionByName(const kxf::String& name)
	{
		for (IGameDefinition& definition: EnumGameDefinitions())
		{
			if (definition.GetName() == name)
			{
				return &definition;
			}
		}
		return nullptr;
	}
	IGameInstance* IApplication::GetGameInstanceByName(const kxf::String& name)
	{
		for (IGameInstance& instance: EnumGameInstances())
		{
			if (instance.GetName() == name)
			{
				return &instance;
			}
		}
		return nullptr;
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

	kxf::String IApplication::FormatCommandLine(const std::unordered_map<kxf::String, kxf::String>& parameters)
	{
		kxf::String commandLine;
		auto AddCommand = [&commandLine](const kxf::String& command)
		{
			if (!command.IsEmpty())
			{
				if (!commandLine.IsEmpty())
				{
					commandLine += wxS(' ');
				}
				commandLine += command;
			}
		};

		for (auto&& [name, value]: parameters)
		{
			if (!name.IsEmptyOrWhitespace() && !value.IsEmptyOrWhitespace())
			{
				AddCommand(kxf::String::Format(wxS("-%1 \"%2\""), name, value));
			}
		}
		return commandLine;
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
				delay = kxf::TimeSpan::Seconds(GetGlobalOption(OName::RestartDelay).GetValueInt(3));
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
			if (const IMainWindow* mainWindow = GetMainWindow())
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
