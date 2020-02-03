#include "stdafx.h"
#include <Kortex/PluginManager.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>
#include "Utility/OperationWithProgress.h"
#include "Utility/Log.h"
#include <KxFramework/KxShell.h>
#include <KxFramework/KxSystem.h>
#include "loot/api.h"
#include "loot/enum/game_type.h"
#include "loot/loot_version.h"

#if _WIN64
	#pragma comment(lib, "LibLoot/x64/loot.lib")
#else
	#pragma comment(lib, "LibLoot/x86/loot.lib")
#endif

namespace
{
	KxEVENT_DEFINE_LOCAL(KxFileOperationEvent, LibLoot);

	std::string ToLootString(const wxString& value)
	{
		auto utf8 = value.ToUTF8();
		return std::string(utf8.data(), utf8.length());
	}
	wxString FromLootString(const std::string& value)
	{
		return wxString::FromUTF8(value.data(), value.length());
	}

	std::optional<loot::GameType> GetLootGameType(const Kortex::GameID& gameID)
	{
		using namespace Kortex;
		using namespace loot;

		if (IPluginManager::GetInstance() && IGameInstance::GetActive())
		{
			// TES
			if (gameID == GameIDs::Morrowind)
			{
				return GameType::tes3;
			}
			else if (gameID == GameIDs::Oblivion)
			{
				return GameType::tes4;
			}
			else if (gameID == GameIDs::Skyrim)
			{
				return GameType::tes5;
			}
			else if (gameID == GameIDs::SkyrimSE)
			{
				return GameType::tes5se;
			}
			else if (gameID == GameIDs::SkyrimVR)
			{
				return GameType::tes5vr;
			}

			// Fallout
			else if (gameID == GameIDs::Fallout3)
			{
				return GameType::fo3;
			}
			else if (gameID == GameIDs::FalloutNV)
			{
				return GameType::fonv;
			}
			else if (gameID == GameIDs::Fallout4)
			{
				return GameType::fo4;
			}
			else if (gameID == GameIDs::Fallout4VR)
			{
				return GameType::fo4vr;
			}
		}
		return std::nullopt;
	}
	void LoggerCallback(loot::LogLevel level, const char* message,  Kortex::Utility::OperationWithProgressDialogBase* context = nullptr)
	{
		using namespace Kortex;
		using namespace loot;

		switch (level)
		{
			case LogLevel::trace:
			{
				Utility::Log::LogTrace("[LOOT API] %1", message);
				break;
			}
			case LogLevel::debug:
			{
				Utility::Log::LogDebug("[LOOT API] %1", message);
				break;
			}
			case LogLevel::info:
			{
				Utility::Log::LogInfo("[LOOT API] %1", message);

				if (context)
				{
					KxFileOperationEvent event(EvtLibLoot);
					event.SetEventObject(context->GetEventHandler());
					event.SetCurrent(message);
					context->ProcessEvent(event);
				}
				break;
			}
			case LogLevel::warning:
			{
				Utility::Log::LogWarning("[LOOT API] %1", message);
				break;
			}
			case LogLevel::error:
			{
				Utility::Log::LogError("[LOOT API] %1", message);
				break;
			}
			case LogLevel::fatal:
			{
				Utility::Log::LogMessage("Fatal Error: [LOOT API] %1", message);
				break;
			}
		};
	}
	void ThrowError(const wxString& message)
	{
		throw std::runtime_error(ToLootString(message));
	};
}

namespace Kortex::PluginManager
{
	wxString LibLoot::GetLibraryName()
	{
		return wxS("LibLoot");
	}
	KxVersion LibLoot::GetLibraryVersion()
	{
		return FromLootString(loot::LootVersion().GetVersionString());
	}

	wxString LibLoot::GetDataPath() const
	{
		BethesdaPluginManager* pluginManager = nullptr;
		if (IPluginManager::GetInstance()->QueryInterface(pluginManager))
		{
			KxFile folder(KxShell::GetFolder(KxSHF_APPLICATIONDATA_LOCAL) + wxS("\\LOOT\\") + pluginManager->GetLibLootConfig().GetFolderName());
			folder.CreateFolder();
			return folder.GetFullPath();
		}
		return wxEmptyString;
	}
	wxString LibLoot::GetMasterListPath() const
	{
		return GetDataPath() + wxS("\\masterlist.yaml");
	}
	wxString LibLoot::GetUserListPath() const
	{
		return GetDataPath() + wxS("\\userlist.yaml");
	}

	bool LibLoot::CanSortNow() const
	{
		return IModManager::GetInstance()->GetFileSystem().IsEnabled();
	}
	bool LibLoot::SortPlugins(KxStringVector& sortedList, Utility::OperationWithProgressDialogBase* context)
	{
		try
		{
			if (context)
			{
				context->GetEventHandler()->Bind(EvtLibLoot, &Utility::OperationWithProgressDialogBase::OnFileOperation, context);
			}
			loot::SetLoggingCallback([context](loot::LogLevel level, const char* s)
			{
				LoggerCallback(level, s, context);
			});

			BethesdaPluginManager* pluginManager = IPluginManager::GetInstance()->QueryInterface<BethesdaPluginManager>();
			const Config& managerConfig = pluginManager->GetConfig();
			const LootAPIConfig& lootConfig = pluginManager->GetLibLootConfig();

			if (auto gameType = GetLootGameType(IGameInstance::GetActive()->GetGameID()))
			{
				const std::string virtualGameDir = ToLootString(IGameInstance::GetActive()->GetVirtualGameDir());
				if (auto gameInterface = loot::CreateGameHandle(*gameType, virtualGameDir, ToLootString(lootConfig.GetLocalGamePath())))
				{
					if (auto dataBase = gameInterface->GetDatabase())
					{
						const std::string masterListPath = ToLootString(GetMasterListPath());
						const std::string userListPath = ToLootString(GetUserListPath());
						const std::string repositoryBranch = ToLootString(lootConfig.GetBranch());
						const std::string repositoryURL = ToLootString(lootConfig.GetRepository());

						// Update masterlist
						const bool isMasterListUpdated = dataBase->UpdateMasterlist(masterListPath, repositoryURL, repositoryBranch);
						if (isMasterListUpdated && !dataBase->IsLatestMasterlist(masterListPath, repositoryBranch))
						{
							ThrowError(KTr("PluginManager.LibLoot.CanNotUpdateMasterlist"));
						}

						// Load list
						dataBase->LoadLists(masterListPath, KxFile(userListPath).IsFileExist() ? userListPath : "");

						// Main ESM
						if (managerConfig.HasMainStdContentID())
						{
							gameInterface->IdentifyMainMasterFile(ToLootString(managerConfig.GetMainStdContentID()));
						}

						std::vector<std::string> pluginList;
						for (const auto& plugin: pluginManager->GetPlugins())
						{
							std::string name = ToLootString(plugin->GetName());
							if (gameInterface->IsValidPlugin(name))
							{
								pluginList.emplace_back(std::move(name));
							}
						}

						pluginManager->Save();
						gameInterface->LoadCurrentLoadOrderState();
						pluginList = gameInterface->SortPlugins(pluginList);

						// Return sorted list
						sortedList.clear();
						sortedList.reserve(pluginList.size());
						for (const std::string& s: pluginList)
						{
							sortedList.emplace_back(FromLootString(s));
						}
						return !sortedList.empty();
					}
				}
			}
		}
		catch (const std::exception& e)
		{
			LoggerCallback(loot::LogLevel::fatal, e.what(), context);
			sortedList.clear();

			BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtError, FromLootString(e.what()));
		}
		return false;
	}
}
