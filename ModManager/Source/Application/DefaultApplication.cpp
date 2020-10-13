#include "pch.hpp"
#include "DefaultApplication.h"
#include "SystemApplication.h"
#include "Log.h"
#include <kxf/System/ShellOperations.h>

namespace Kortex::Application
{
	void DefaultApplication::OnCreate()
	{
		m_BroadcastReciever = std::make_unique<BroadcastReciever>();

		// Setup paths
		m_DataDirectory = GetRootDirectory() / "Data";
		m_SettingsDirectory = kxf::Shell::GetKnownDirectory(kxf::KnownDirectoryID::ApplicationDataLocal) / GetID();
		m_SettingsFile = m_SettingsDirectory / "Settings.xml";
		m_LogsDirectory = m_SettingsDirectory / "Logs";
		m_DefaultInstancesDirectory = m_SettingsDirectory / "Instances";

		// Variables
		m_Variables.SetItem("App", "DataDirectory", m_DataDirectory);
		m_Variables.SetDynamicItem("App", "Revision", [this]()
		{
			return m_Variables.GetItem("App", "CommitHash").GetAs<kxf::String>().Left(7);
		});
	}
	void DefaultApplication::OnDestroy()
	{
		m_BroadcastReciever = nullptr;
	}

	bool DefaultApplication::OnInit()
	{
		const bool anotherInstanceRunning = IsAnotherInstanceRunning();

		return false;
	}
	int DefaultApplication::OnExit()
	{
		Log::Info("DefaultApplication::OnExit");

		return 0;
	}
	bool DefaultApplication::OnException()
	{
		Log::FatalError(ExamineCaughtException());
		return false;
	}
	
	void DefaultApplication::OnGlobalConfigChanged(AppOption& option)
	{
	}
	void DefaultApplication::OnInstanceConfigChanged(AppOption& option, IGameInstance& instance)
	{
	}
	void DefaultApplication::OnProfileConfigChanged(AppOption& option, IGameProfile& profile)
	{
	}

	kxf::String DefaultApplication::ExpandVariablesLocally(const kxf::String& variables) const
	{
		return m_Variables.Expand(variables);
	}
	kxf::String DefaultApplication::ExpandVariables(const kxf::String& variables) const
	{
		//if (IGameInstance* instance = IGameInstance::GetActive())
		//{
		//	return instance->ExpandVariables(variables);
		//}
		return ExpandVariablesLocally(variables);
	}

	bool DefaultApplication::OpenInstanceSelectionDialog()
	{
		return false;
	}
	bool DefaultApplication::Uninstall()
	{
		return false;
	}
}
