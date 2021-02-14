#include "pch.hpp"
#include "DefaultApplication.h"
#include "SystemApplication.h"
#include "Options/CmdLineDatabase.h"
#include "Log.h"

#include "GameDefinition/IGameProfile.h"
#include "GameDefinition/IGameInstance.h"
#include "GameDefinition/DefaultGameDefinition.h"
#include "GameDefinition/DefaultGameInstance.h"
#include "GameDefinition/UI/InstanceSelectionDialog.h"

#include <kxf/System/ShellOperations.h>
#include <kxf/FileSystem/NativeFileSystem.h>
#include <kxf/Utility/Enumerator.h>
#include <wx/cmdline.h>

namespace
{
	constexpr struct
	{
		static constexpr kxf::XChar Language[] = wxS("Language");
		static constexpr kxf::XChar Locale[] = wxS("Locale");
		static constexpr kxf::XChar Active[] = wxS("Active");

		static constexpr kxf::XChar GameInstances[] = wxS("GameInstances");
	} g_OptionNames;
}

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
		kxf::Locale activeLocale = this->GetGlobalOption(g_OptionNames.Language).GetAttribute(g_OptionNames.Locale);
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
		auto DoLoad = [&](const kxf::FileItem& item)
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

		for (const kxf::FileItem& item: m_GameDefinitionsFS.EnumItems({}, {}, kxf::FSActionFlag::LimitToDirectories))
		{
			DoLoad(item);
		}
		for (const kxf::FileItem& item: m_GameDefinitionsUserFS.EnumItems({}, {}, kxf::FSActionFlag::LimitToDirectories))
		{
			DoLoad(item);
		}
	}
	void DefaultApplication::LoadGameInstances()
	{
		auto option = GetGlobalOption(g_OptionNames.GameInstances);
		const kxf::String activeName = option.GetAttribute(g_OptionNames.Active);

		for (const kxf::FileItem& item: m_GameInstancesFS.EnumItems({}, {}, kxf::FSActionFlag::LimitToDirectories))
		{
			if (item.IsNormalItem())
			{
				auto instance = std::make_unique<DefaultGameInstance>();

				kxf::ScopedNativeFileSystem fs(item.GetFullPath());
				if (instance->LoadInstanceData(fs))
				{
					if (!m_ActiveGameInstance && instance->GetName() == activeName)
					{
						m_ActiveGameInstance = instance.get();
					}
					m_GameInstances.insert_or_assign(instance->GetName(), std::move(instance));
				}
			}
		}
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
		for (auto&& classInfo: kxf::RTTI::EnumClassInfo())
		{
			Log::Info(classInfo.GetFullyQualifiedName());
		}

		// Load global config
		LoadGlobalConfiguration();
		LoadLocalizationPackages();

		// Do initialization procedure
		const bool anotherInstanceRunning = IsAnotherInstanceRunning();
		if (!anotherInstanceRunning)
		{
			LoadGameDefinitions();
			LoadGameInstances();

			return OpenInstanceSelectionDialog() != nullptr;
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
		m_GlobalConfigChanged = true;
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
	kxf::Enumerator<IModule&> DefaultApplication::EnumModules()
	{
		return kxf::Utility::EnumerateIterableContainer<IModule&, kxf::Utility::ReferenceOf>(m_LoadedModules);
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

	kxf::Enumerator<IGameDefinition&> DefaultApplication::EnumGameDefinitions()
	{
		return kxf::Utility::EnumerateStandardMap<IGameDefinition&, kxf::Utility::ReferenceOf>(m_GameDefinitions);
	}
	kxf::Enumerator<IGameInstance&> DefaultApplication::EnumGameInstances()
	{
		return kxf::Utility::EnumerateStandardMap<IGameInstance&, kxf::Utility::ReferenceOf>(m_GameInstances);
	}
	IGameInstance* DefaultApplication::GetActiveGameInstance() const
	{
		return m_ActiveGameInstance;
	}

	IGameInstance* DefaultApplication::OpenInstanceSelectionDialog(wxWindow* parent)
	{
		GameDefinition::UI::InstanceSelectionDialog dialog(parent);
		if (dialog.ShowModalDialog() == kxf::StdID::OK)
		{
			return dialog.GetSelectedInstance();
		}
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
