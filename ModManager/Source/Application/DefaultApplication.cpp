#include "pch.hpp"
#include "DefaultApplication.h"
#include "SystemApplication.h"
#include "Options/CmdLineDatabase.h"
#include "Log.h"

#include "GameDefinition/IGameProfile.h"
#include "GameDefinition/IGameInstance.h"
#include "GameDefinition/DefaultGameDefinition.h"
#include "GameDefinition/DefaultGameInstance.h"

#include <kxf/System/ShellOperations.h>
#include <kxf/FileSystem/NativeFileSystem.h>
#include <wx/cmdline.h>

namespace Kortex
{
	void DefaultApplication::LoadGlobalConfiguration()
	{
		const kxf::FSPath globalConfigPath = m_GlobalConfigFS.ResolvePath(wxS("Settings.xml"));
		Log::Info("Loading global config from: '%1'", globalConfigPath.GetFullPath());

		if (auto stream = m_GlobalConfigFS.OpenToRead(globalConfigPath); !stream || !m_GlobalConfig.Load(*stream))
		{
			throw std::runtime_error("Couldn't load global config");
		}
	}
	void DefaultApplication::LoadLocalizationPackages()
	{
		kxf::Locale activeLocale = this->GetGlobalOption("Language").GetAttribute("Locale");
		if (!activeLocale)
		{
			activeLocale = kxf::Locale::GetSystemPreferred();
		}

		kxf::Localization::SearchPackages(m_AppResourcesFS, "LocalizationPacks", [&](kxf::Locale locale, kxf::FileItem fileItem)
		{
			if (locale == activeLocale)
			{
				if (auto stream = m_AppResourcesFS.OpenToRead(fileItem.GetFullPath()))
				{
					auto package = std::make_unique<kxf::AndroidLocalizationPackage>();
					if (package->Load(*stream, std::move(locale)))
					{
						m_LocalizationPackages.Add(std::move(package));
					}
				}
				return true;
			}
			return true;
		});

		if (m_LocalizationPackages.IsEmpty())
		{
			throw std::runtime_error("No localization packages found");
		}
	}

	void DefaultApplication::LoadGameDefinitions()
	{
		auto DoLoad = [&](kxf::FileItem item)
		{
			if (item.IsNormalItem())
			{
				auto definition = std::make_unique<DefaultGameDefinition>();

				kxf::ScopedNativeFileSystem fs(item.GetFullPath());
				if (definition->LoadDefinitionData(fs))
				{
					// We allow user-defined definitions to replace the system ones so using 'insert_or_assign' here
					m_GameDefinitions.insert_or_assign(definition->GetName(), std::move(definition));
				}
			}
			return true;
		};

		m_GameDefinitionsFS.EnumItems({}, DoLoad, {}, kxf::FSActionFlag::LimitToDirectories);
		m_GameDefinitionsUserFS.EnumItems({}, DoLoad, {}, kxf::FSActionFlag::LimitToDirectories);
	}
	void DefaultApplication::LoadGameInstances()
	{
		m_GameInstancesFS.EnumItems({}, [&](kxf::FileItem item)
		{
			if (item.IsNormalItem())
			{
				auto instance = std::make_unique<DefaultGameInstance>();

				kxf::ScopedNativeFileSystem fs(item.GetFullPath());
				if (instance->LoadInstanceData(fs))
				{
					// We allow user-defined definitions to replace the system ones so using 'insert_or_assign' here
					m_GameInstances.insert_or_assign(instance->GetName(), std::move(instance));
				}
			}
			return true;
		}, {}, kxf::FSActionFlag::LimitToDirectories);
	}

	// IApplication
	bool DefaultApplication::OnCreate()
	{
		m_BroadcastReciever = std::make_unique<BroadcastReciever>();

		// Setup paths
		m_AppRootFS = SystemApplication::GetInstance().GetRootDirectory();
		m_AppResourcesFS = m_AppRootFS.GetLookupDirectory() / wxS("ModManager");
		m_GlobalConfigFS = m_GlobalConfigOverride ? m_GlobalConfigOverride : kxf::Shell::GetKnownDirectory(kxf::KnownDirectoryID::ApplicationDataLocal) / GetID();
		m_AppLogsFS = m_GlobalConfigFS.GetLookupDirectory() / wxS("Logs");
		m_GameDefinitionsFS = m_AppResourcesFS.GetLookupDirectory() / wxS("GameDefinitions");
		m_GameDefinitionsUserFS = m_GlobalConfigFS.GetLookupDirectory() / wxS("GameDefinitions");
		m_GameInstancesFS = m_GlobalConfigFS.GetLookupDirectory() / wxS("Instances");

		// Variables
		m_Variables.SetItem("App", "AppResourcesDirectory", m_AppResourcesFS.GetLookupDirectory());
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
		LoadGlobalConfiguration();
		LoadLocalizationPackages();

		// Do initialization procedure
		const bool anotherInstanceRunning = IsAnotherInstanceRunning();
		if (!anotherInstanceRunning)
		{
			LoadGameDefinitions();
			LoadGameInstances();

			return true;
		}
		else
		{
			throw std::runtime_error("Another instance is running");
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
			case FileSystemOrigin::Unscoped:
			{
				return m_UnscopedFS;
			}
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
			return m_Variables.Expand(m_ActiveGameInstance->ExpandVariables(variables));
		}
		return m_Variables.Expand(variables);
	}
	kxf::String DefaultApplication::ExpandVariablesLocally(const kxf::String& variables) const
	{
		return m_Variables.Expand(variables);
	}

	size_t DefaultApplication::EnumGameDefinitions(std::function<bool(IGameDefinition&)> func)
	{
		if (func)
		{
			size_t count = 0;
			for (auto&& [id, item]: m_GameDefinitions)
			{
				count++;
				if (!std::invoke(func, *item))
				{
					break;
				}
			}
			return count;
		}
		else
		{
			return m_GameDefinitions.size();
		}
	}
	size_t DefaultApplication::EnumGameInstances(std::function<bool(IGameInstance&)> func)
	{
		if (func)
		{
			size_t count = 0;
			for (auto&& [id, item]: m_GameInstances)
			{
				count++;
				if (!std::invoke(func, *item))
				{
					break;
				}
			}
			return count;
		}
		else
		{
			return m_GameInstances.size();
		}
	}
	IGameInstance* DefaultApplication::GetActiveGameInstance() const
	{
		return m_ActiveGameInstance.get();
	}

	IGameInstance* DefaultApplication::OpenInstanceSelectionDialog()
	{
		return nullptr;
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
