#include "stdafx.h"
#include "KLootAPI.h"
#include "PluginManager/KPluginManager.h"
#include "ModManager/KModManager.h"
#include "GameInstance/KInstanceManagement.h"
#include "GameInstance/Config/KPluginManagerConfig.h"
#include "KApp.h"
#include "KOperationWithProgress.h"
#include "Events/KLogEvent.h"
#include <KxFramework/KxShell.h>
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxTranslation.h>
#include "loot/api.h"
#include "loot/enum/game_type.h"
#include "loot/loot_version.h"

#if _WIN64
	#pragma comment(lib, "LOOT API/x64/loot_api.lib")
#else
	#pragma comment(lib, "LOOT API/x86/loot_api.lib")
#endif

wxDEFINE_EVENT(KEVT_LOOTAPI, KxFileOperationEvent);

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

int KLootAPI::GetLootGameID() const
{
	loot::GameType gameType = (loot::GameType)INVALID_GAME_ID;
	if (KPluginManager::HasInstance() && KGameInstance::GetActive())
	{
		const KGameID gameID = KGameInstance::GetActive()->GetGameID();

		// TES
		if (gameID == KGameIDs::Oblivion)
		{
			gameType = loot::GameType::tes4;
		}
		else if (gameID == KGameIDs::Skyrim)
		{
			gameType = loot::GameType::tes5;
		}
		else if (gameID == KGameIDs::SkyrimSE)
		{
			gameType = loot::GameType::tes5se;
		}
		else if (gameID == KGameIDs::SkyrimVR)
		{
			gameType = loot::GameType::tes5vr;
		}

		// Fallout
		else if (gameID == KGameIDs::Fallout3)
		{
			gameType = loot::GameType::fo3;
		}
		else if (gameID == KGameIDs::FalloutNV)
		{
			gameType = loot::GameType::fonv;
		}
		else if (gameID == KGameIDs::Fallout4)
		{
			gameType = loot::GameType::fo4;
		}
		else if (gameID == KGameIDs::Fallout4VR)
		{
			gameType = loot::GameType::fo4vr;
		}
	}
	return (int)gameType;
}
void KLootAPI::LoggerCallback(int level, const char* value, KOperationWithProgressDialogBase* context)
{
	using LogLevel = loot::LogLevel;
	switch (level)
	{
		case (int)LogLevel::trace:
		{
			wxLogTrace("", "[LOOT API] %s", value);
			break;
		}
		case (int)LogLevel::debug:
		{
			wxLogDebug("[LOOT API] %s", value);
			break;
		}
		case (int)LogLevel::info:
		{
			wxLogInfo("[LOOT API] %s", value);

			KxFileOperationEvent event(KEVT_LOOTAPI);
			event.SetEventObject(context->GetEventHandler());
			event.SetCurrent(value);
			context->ProcessEvent(event);
			break;
		}
		case (int)LogLevel::warning:
		{
			wxLogWarning("[LOOT API] %s", value);
			break;
		}
		case (int)LogLevel::error:
		{
			wxLogError("[LOOT API] %s", value);
			break;
		}
		case (int)LogLevel::fatal:
		{
			wxLogMessage("Fatal Error: [LOOT API] %s", value);
			break;
		}
	};
}

wxString KLootAPI::GetVersion() const
{
	return FromLootString(loot::LootVersion().string());
}
bool KLootAPI::IsSupported() const
{
	return KPluginManagerConfig::HasInstance() && KPluginManagerConfig::GetInstance()->HasLootAPI();
}
bool KLootAPI::CanSortNow() const
{
	return KModManager::GetInstance()->IsVFSMounted();
}

wxString KLootAPI::GetDataPath() const
{
	const KPluginManagerConfig::LootAPI& lootAPI = KPluginManagerConfig::GetInstance()->GetLootAPI();

	KxFile folder(KxShell::GetFolder(KxSHF_APPLICATIONDATA_LOCAL) + wxS("\\LOOT\\") + lootAPI.GetFolderName());
	folder.CreateFolder();
	return folder.GetFullPath();
}
wxString KLootAPI::GetMasterListPath() const
{
	return GetDataPath() + wxS("\\MasterList.yaml");
}
wxString KLootAPI::GetUserListPath() const
{
	return GetDataPath() + wxS("\\UserList.yaml");
}

bool KLootAPI::SortPlugins(KxStringVector& sortedList, KOperationWithProgressDialogBase* context)
{
	context->GetEventHandler()->Bind(KEVT_LOOTAPI, &KOperationWithProgressDialogBase::OnFileOperation, context);
	auto ThrowError = [](const wxString& s)
	{
		throw std::runtime_error(ToLootString(s));
	};

	try
	{
		int gameID = GetLootGameID();
		if (gameID != INVALID_GAME_ID)
		{
			KPluginManager* pluginManager = KPluginManager::GetInstance();
			const KPluginManagerConfig* pluginManagerOptions = KPluginManagerConfig::GetInstance();
			const KPluginManagerConfig::LootAPI& lootAPI = pluginManagerOptions->GetLootAPI();

			std::locale locale("");
			loot::InitialiseLocale("en.UTF-8");
			loot::SetLoggingCallback([this, context](loot::LogLevel level, const char* s)
			{
				LoggerCallback((int)level, s, context);
			});

			
			const std::string virtualGameDir = ToLootString(KGameInstance::GetActive()->GetVirtualGameDir());
			auto gameInterface = loot::CreateGameHandle((loot::GameType)gameID, virtualGameDir, ToLootString(lootAPI.GetLocalGamePath()));
			if (gameInterface)
			{
				auto dataBase = gameInterface->GetDatabase();
				if (dataBase)
				{
					const std::string masterListPath = ToLootString(GetMasterListPath());
					const std::string userListPath = ToLootString(GetUserListPath());
					const std::string repositoryBranch = ToLootString(lootAPI.GetBranch());
					const std::string repositoryURL = ToLootString(lootAPI.GetRepository());

					// Update masterlist
					bool isMasterListUpdated = dataBase->UpdateMasterlist(masterListPath, repositoryURL, repositoryBranch);
					if (isMasterListUpdated && !dataBase->IsLatestMasterlist(masterListPath, repositoryBranch))
					{
						ThrowError(KTr("PluginManager.LootAPI.CanNotUpdateMasterlist"));
					}

					// Load list
					dataBase->LoadLists(masterListPath, KxFile(userListPath).IsFileExist() ? userListPath : "");

					// Main ESM
					if (pluginManagerOptions->HasMainStdContentID())
					{
						gameInterface->IdentifyMainMasterFile(ToLootString(pluginManagerOptions->GetMainStdContentID()));
					}

					KxStdStringVector pluginList;
					for (const auto& plugnEntry: pluginManager->GetEntries())
					{
						std::string name = ToLootString(plugnEntry->GetName());
						if (gameInterface->IsValidPlugin(name))
						{
							pluginList.push_back(name);
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

		KLogEvent(FromLootString(e.what()), KLOG_ERROR).Send();
	}
	return false;
}
