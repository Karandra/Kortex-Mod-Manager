#include "stdafx.h"
#include "KLootAPI.h"
#include "PluginManager/KPluginManager.h"
#include "ModManager/KModManager.h"
#include "GameInstance/KGameInstance.h"
#include "GameInstance/KGameID.h"
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
	if (KPluginManager::HasInstance())
	{
		const wxString& templateID = KApp::Get().GetCurrentGameID();

		// TES
		if (templateID == KGameIDs::Oblivion)
		{
			gameType = loot::GameType::tes4;
		}
		else if (templateID == KGameIDs::Skyrim)
		{
			gameType = loot::GameType::tes5;
		}
		else if (templateID == KGameIDs::SkyrimSE)
		{
			gameType = loot::GameType::tes5se;
		}
		else if (templateID == KGameIDs::SkyrimVR)
		{
			gameType = loot::GameType::tes5vr;
		}

		// Fallout
		else if (templateID == KGameIDs::Fallout3)
		{
			gameType = loot::GameType::fo3;
		}
		else if (templateID == KGameIDs::FalloutNV)
		{
			gameType = loot::GameType::fonv;
		}
		else if (templateID == KGameIDs::Fallout4)
		{
			gameType = loot::GameType::fo4;
		}
		else if (templateID == KGameIDs::Fallout4VR)
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
	return KModManager::Get().IsVFSMounted();
}

wxString KLootAPI::GetDataPath() const
{
	const KPluginManagerConfig::LootAPI& lootAPI = KPluginManagerConfig::GetInstance()->GetLootAPI();

	KxFile folder(KxShell::GetFolder(KxSHF_APPLICATIONDATA_LOCAL) + "\\LOOT\\" + lootAPI.GetFolderName());
	folder.CreateFolder();
	return folder.GetFullPath();
}
wxString KLootAPI::GetMasterListPath() const
{
	return GetDataPath() + '\\' + "MasterList.yaml";
}
wxString KLootAPI::GetUserListPath() const
{
	return GetDataPath() + '\\' + "UserList.yaml";
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
		std::locale locale("");
		std::string gamePath = ToLootString(KModManager::Get().GetVirtualGameRoot());

		KPluginManager* pluginManager = KPluginManager::GetInstance();
		const KPluginManagerConfig* pluginManagerOptions = KPluginManagerConfig::GetInstance();
		const KPluginManagerConfig::LootAPI& lootAPI = pluginManagerOptions->GetLootAPI();

		int gameID = GetLootGameID();
		if (gameID != INVALID_GAME_ID)
		{
			loot::SetLoggingCallback([this, context](loot::LogLevel level, const char* s)
			{
				LoggerCallback((int)level, s, context);
			});

			loot::InitialiseLocale("en.UTF-8");
			auto gameInterface = loot::CreateGameHandle((loot::GameType)gameID, gamePath, ToLootString(lootAPI.GetLocalGamePath()));
			if (gameInterface)
			{
				auto dataBase = gameInterface->GetDatabase();
				if (dataBase)
				{
					std::string sMasterListPath = ToLootString(GetMasterListPath());
					std::string sUserListPath = ToLootString(GetUserListPath());
					std::string sRepositoryBranch = ToLootString(lootAPI.GetBranch());
					std::string sRepositoryURL = ToLootString(lootAPI.GetRepository());

					// Update masterlist
					bool isMasterListUpdated = dataBase->UpdateMasterlist(sMasterListPath, sRepositoryURL, sRepositoryBranch);
					if (isMasterListUpdated && !dataBase->IsLatestMasterlist(sMasterListPath, sRepositoryBranch))
					{
						ThrowError(T("PluginManager.LootAPI.CanNotUpdateMasterlist"));
					}

					// Load list
					dataBase->LoadLists(sMasterListPath, KxFile(sUserListPath).IsFileExist() ? sUserListPath : "");
					
					// Main ESM
					if (pluginManagerOptions->HasMainStdContentID())
					{
						gameInterface->IdentifyMainMasterFile(ToLootString(pluginManagerOptions->GetMainStdContentID()));
					}

					KxStdStringVector pluginsList;
					for (const auto& pPlugnEntry: pluginManager->GetEntries())
					{
						std::string name = ToLootString(pPlugnEntry->GetName());
						if (gameInterface->IsValidPlugin(name))
						{
							pluginsList.push_back(name);
						}
					}

					pluginManager->Save();
					gameInterface->LoadCurrentLoadOrderState();
					pluginsList = gameInterface->SortPlugins(pluginsList);

					// Return sorted list
					sortedList.clear();
					sortedList.reserve(pluginsList.size());
					for (const std::string& s: pluginsList)
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
