#include "stdafx.h"
#include <Kortex/PluginManager.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>
#include "Utility/KOperationWithProgress.h"
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
	std::string ToLootString(const wxString& s)
	{
		auto utf8 = s.ToUTF8();
		return std::string(utf8.data(), utf8.length());
	}
	wxString FromLootString(const std::string& s)
	{
		return wxString::FromUTF8(s.data(), s.length());
	}
}

namespace Kortex::Events
{
	wxDEFINE_EVENT(LibLoot, KxFileOperationEvent);
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

	int LibLoot::GetLootGameID() const
	{
		loot::GameType gameType = (loot::GameType)INVALID_GAME_ID;
		if (IPluginManager::GetInstance() && IGameInstance::GetActive())
		{
			const GameID gameID = IGameInstance::GetActive()->GetGameID();

			// TES
			if (gameID == GameIDs::Oblivion)
			{
				gameType = loot::GameType::tes4;
			}
			else if (gameID == GameIDs::Skyrim)
			{
				gameType = loot::GameType::tes5;
			}
			else if (gameID == GameIDs::SkyrimSE)
			{
				gameType = loot::GameType::tes5se;
			}
			else if (gameID == GameIDs::SkyrimVR)
			{
				gameType = loot::GameType::tes5vr;
			}

			// Fallout
			else if (gameID == GameIDs::Fallout3)
			{
				gameType = loot::GameType::fo3;
			}
			else if (gameID == GameIDs::FalloutNV)
			{
				gameType = loot::GameType::fonv;
			}
			else if (gameID == GameIDs::Fallout4)
			{
				gameType = loot::GameType::fo4;
			}
			else if (gameID == GameIDs::Fallout4VR)
			{
				gameType = loot::GameType::fo4vr;
			}
		}
		return (int)gameType;
	}
	void LibLoot::LoggerCallback(int level, const char* value, KOperationWithProgressDialogBase* context)
	{
		using LogLevel = loot::LogLevel;
		switch (level)
		{
			case (int)LogLevel::trace:
			{
				Utility::Log::LogTrace("[LOOT API] %1", value);
				break;
			}
			case (int)LogLevel::debug:
			{
				Utility::Log::LogDebug("[LOOT API] %1", value);
				break;
			}
			case (int)LogLevel::info:
			{
				Utility::Log::LogInfo("[LOOT API] %1", value);

				KxFileOperationEvent event(Events::LibLoot);
				event.SetEventObject(context->GetEventHandler());
				event.SetCurrent(value);
				context->ProcessEvent(event);
				break;
			}
			case (int)LogLevel::warning:
			{
				Utility::Log::LogWarning("[LOOT API] %1", value);
				break;
			}
			case (int)LogLevel::error:
			{
				Utility::Log::LogError("[LOOT API] %1", value);
				break;
			}
			case (int)LogLevel::fatal:
			{
				Utility::Log::LogMessage("Fatal Error: [LOOT API] %1", value);
				break;
			}
		};
	}

	LibLoot::LibLoot()
		:m_PluginManager(IPluginManager::GetInstance()->QueryInterface<BethesdaPluginManager>()),
		m_LootConfig(m_PluginManager->GetLibLootConfig()), m_ManagerConfig(m_PluginManager->GetConfig())
	{
	}

	wxString LibLoot::GetDataPath() const
	{
		KxFile folder(KxShell::GetFolder(KxSHF_APPLICATIONDATA_LOCAL) + wxS("\\LOOT\\") + m_PluginManager->GetLibLootConfig().GetFolderName());
		folder.CreateFolder();
		return folder.GetFullPath();
	}
	wxString LibLoot::GetMasterListPath() const
	{
		return GetDataPath() + wxS("\\MasterList.yaml");
	}
	wxString LibLoot::GetUserListPath() const
	{
		return GetDataPath() + wxS("\\UserList.yaml");
	}

	bool LibLoot::CanSortNow() const
	{
		return IModManager::GetInstance()->GetFileSystem().IsEnabled();
	}
	bool LibLoot::SortPlugins(KxStringVector& sortedList, KOperationWithProgressDialogBase* context)
	{
		context->GetEventHandler()->Bind(Events::LibLoot, &KOperationWithProgressDialogBase::OnFileOperation, context);
		auto ThrowError = [](const wxString& s)
		{
			throw std::runtime_error(ToLootString(s));
		};

		try
		{
			int gameID = GetLootGameID();
			if (gameID != (int)INVALID_GAME_ID)
			{
				//std::locale locale("");
				//loot::InitialiseLocale("en.UTF-8"); // Removed in v15.0
				loot::SetLoggingCallback([this, context](loot::LogLevel level, const char* s)
				{
					LoggerCallback((int)level, s, context);
				});

				const std::string virtualGameDir = ToLootString(IGameInstance::GetActive()->GetVirtualGameDir());
				auto gameInterface = loot::CreateGameHandle((loot::GameType)gameID, virtualGameDir, ToLootString(m_LootConfig.GetLocalGamePath()));
				if (gameInterface)
				{
					auto dataBase = gameInterface->GetDatabase();
					if (dataBase)
					{
						const std::string masterListPath = ToLootString(GetMasterListPath());
						const std::string userListPath = ToLootString(GetUserListPath());
						const std::string repositoryBranch = ToLootString(m_LootConfig.GetBranch());
						const std::string repositoryURL = ToLootString(m_LootConfig.GetRepository());

						// Update masterlist
						bool isMasterListUpdated = dataBase->UpdateMasterlist(masterListPath, repositoryURL, repositoryBranch);
						if (isMasterListUpdated && !dataBase->IsLatestMasterlist(masterListPath, repositoryBranch))
						{
							ThrowError(KTr("PluginManager.LibLoot.CanNotUpdateMasterlist"));
						}

						// Load list
						dataBase->LoadLists(masterListPath, KxFile(userListPath).IsFileExist() ? userListPath : "");

						// Main ESM
						if (m_ManagerConfig.HasMainStdContentID())
						{
							gameInterface->IdentifyMainMasterFile(ToLootString(m_ManagerConfig.GetMainStdContentID()));
						}

						KxStdStringVector pluginList;
						for (const auto& plugnEntry: m_PluginManager->GetPlugins())
						{
							std::string name = ToLootString(plugnEntry->GetName());
							if (gameInterface->IsValidPlugin(name))
							{
								pluginList.push_back(name);
							}
						}

						m_PluginManager->Save();
						gameInterface->LoadCurrentLoadOrderState();
						pluginList = gameInterface->SortPlugins(pluginList);

						// Return sorted list
						sortedList.clear();
						sortedList.reserve(pluginList.size());
						for (const std::string& s: pluginList)
						{
							sortedList.push_back(FromLootString(s));
						}
						return !sortedList.empty();
					}
				}
			}
		}
		catch (const std::exception& e)
		{
			LoggerCallback((int)loot::LogLevel::fatal, e.what(), context);
			sortedList.clear();

			BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtError, FromLootString(e.what()));
		}
		return false;
	}
}
