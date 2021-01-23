#include "pch.hpp"
#include "DefaultApplication.h"
#include "SystemApplication.h"
#include "Options/CmdLineDatabase.h"
#include "Log.h"

#include "GameInstance/IGameProfile.h"
#include "GameInstance/IGameInstance.h"

#include <kxf/System/ShellOperations.h>
#include <kxf/FileSystem/NativeFileSystem.h>
#include <wx/cmdline.h>

namespace Kortex::Application
{
	// IApplication
	bool DefaultApplication::OnCreate()
	{
		m_BroadcastReciever = std::make_unique<BroadcastReciever>();

		// Setup paths
		m_AppRootFS = SystemApplication::GetInstance().GetRootDirectory();
		m_AppResourcesFS = m_AppRootFS.GetCurrentDirectory() / wxS("Data");
		m_GlobalConfigFS = m_GlobalConfigOverride ? m_GlobalConfigOverride : kxf::Shell::GetKnownDirectory(kxf::KnownDirectoryID::ApplicationDataLocal) / GetID();
		m_AppLogsFS = m_GlobalConfigFS.GetCurrentDirectory() / wxS("Logs");
		m_GameDefinitionsFS = m_AppResourcesFS.GetCurrentDirectory() / wxS("GameDefinitions");
		m_GameDefinitionsUserFS = m_GlobalConfigFS.GetCurrentDirectory() / wxS("GameDefinitions");
		m_GameInstancesFS = m_GlobalConfigFS.GetCurrentDirectory() / wxS("Instances");

		// Variables
		m_Variables.SetItem("App", "AppResourcesDirectory", m_AppResourcesFS.GetCurrentDirectory());
		m_Variables.SetDynamicItem("App", "Revision", [this]()
		{
			return m_Variables.GetItem("App", "CommitHash").GetAs<kxf::String>().Left(7);
		});

		return true;
	}
	void DefaultApplication::OnDestroy()
	{
		m_BroadcastReciever = nullptr;
	}

	bool DefaultApplication::OnInit()
	{
		// Load global config
		const kxf::FSPath globalConfigPath = m_GlobalConfigFS.ResolvePath(wxS("Settings.xml"));
		Log::Info("Loading global config from: '%1'", globalConfigPath.GetFullPath());

		if (auto stream = m_GlobalConfigFS.OpenToRead(globalConfigPath); !stream || !m_GlobalConfig.Load(*stream))
		{
			throw std::runtime_error("Couldn't load global config");
		}

		// Do initialization procedure
		const bool anotherInstanceRunning = IsAnotherInstanceRunning();
		if (!anotherInstanceRunning)
		{
			return true;
		}
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

	DefaultApplication::DefaultApplication() = default;
	DefaultApplication::~DefaultApplication() = default;

	kxf::IFileSystem& DefaultApplication::GetFileSystem(FileSystemOrigin fsOrigin)
	{
		const SystemApplication& systemApp = SystemApplication::GetInstance();
		auto GetUserConfigBaseDirectory = [this]()
		{
			return kxf::Shell::GetKnownDirectory(kxf::KnownDirectoryID::ApplicationDataLocal) / GetID();
		};

		switch (fsOrigin)
		{
			case FileSystemOrigin::AppRoot:
			{
				return m_AppRootFS;
			}
			case FileSystemOrigin::AppResources:
			{
				return m_AppResourcesFS;
			}
			case FileSystemOrigin::GlobalConfig:
			{
				return m_GlobalConfigFS;
			}
			case FileSystemOrigin::AppLogs:
			{
				return m_AppLogsFS;
			}
			case FileSystemOrigin::GameInstances:
			{
				return m_GameInstancesFS;
			}
			case FileSystemOrigin::GameDefinitions:
			{
				return m_GameDefinitionsFS;
			}
			case FileSystemOrigin::GameDefinitionsUser:
			{
				return m_GameDefinitionsUserFS;
			}
		};
		return kxf::FileSystem::GetNullFileSystem();
	}

	kxf::String DefaultApplication::ExpandVariables(const kxf::String& variables) const
	{
		if (m_ActiveGameInstance)
		{
			return m_ActiveGameInstance->ExpandVariables(variables);
		}
		return ExpandVariablesLocally(variables);
	}
	kxf::String DefaultApplication::ExpandVariablesLocally(const kxf::String& variables) const
	{
		return m_Variables.Expand(variables);
	}

	bool DefaultApplication::OpenInstanceSelectionDialog()
	{
		return false;
	}
	bool DefaultApplication::Uninstall()
	{
		return false;
	}

	// kxf::Application::ICommandLine
	size_t DefaultApplication::EnumCommandLineArgs(std::function<bool(kxf::String)> func) const
	{
		if (m_CommandLineParser)
		{
			size_t count = 0;
			for (size_t i = 0; i < m_CommandLineParser->GetParamCount(); i++)
			{
				count++;
				if (!std::invoke(func, m_CommandLineParser->GetParam(i)))
				{
					break;
				}
			}
			return count;
		}
		return 0;
	}
	void DefaultApplication::OnCommandLineInit(wxCmdLineParser& parser)
	{
		m_CommandLineParser = &parser;

		parser.SetSwitchChars("-");
		parser.AddOption(CmdLineName::GameInstance, {}, "Game instance ID");
		parser.AddOption(CmdLineName::AddDownload, {}, "Download URI");
		parser.AddOption(CmdLineName::SettingsDirectory, {}, "Directory path for application-wide config");
	}
	bool DefaultApplication::OnCommandLineParsed(wxCmdLineParser& parser)
	{
		if (kxf::String path; parser.Found(CmdLineName::SettingsDirectory, &path.GetWxString()))
		{
			m_GlobalConfigOverride = std::move(path);
		}

		return true;
	}
	bool DefaultApplication::OnCommandLineError(wxCmdLineParser& parser)
	{
		if (m_CommandLineParser)
		{
			m_CommandLineParser->Usage();
		}
		return false;
	}
	bool DefaultApplication::OnCommandLineHelp(wxCmdLineParser& parser)
	{
		if (m_CommandLineParser)
		{
			m_CommandLineParser->Usage();
		}
		return false;
	}
}
