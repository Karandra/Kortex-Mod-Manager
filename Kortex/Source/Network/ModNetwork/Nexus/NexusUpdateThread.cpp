#include "stdafx.h"
#include "NexusUpdateThread.h"
#include "NexusUpdateChecker.h"
#include "GameMods/IModManager.h"
#include "Application/INotificationCenter.h"
#include "Utility/Log.h"
#include "Utility/DateTime.h"

namespace Kortex::NetworkManager
{
	const ModInfoReply* NexusUpdateThread::GetOrQueryModInfo(ModID id)
	{
		auto it = m_InfoReplies.find(id.GetValue());
		if (it != m_InfoReplies.end())
		{
			return &it->second;
		}
		else if (auto reply = m_Repository.GetModInfo(id))
		{
			return &m_InfoReplies.insert_or_assign(id.GetValue(), std::move(*reply)).first->second;
		}
		return nullptr;
	}
	const NexusRepository::GetModFiles2Result* NexusUpdateThread::GetOrQueryModFiles(ModID id)
	{
		auto it = m_FileReplies.find(id.GetValue());
		if (it != m_FileReplies.end())
		{
			return &it->second;
		}
		else if (auto files = m_Repository.GetModFiles2(id, true, true))
		{
			return &m_FileReplies.insert_or_assign(id.GetValue(), std::move(*files)).first->second;
		}
		return nullptr;
	}
	void NexusUpdateThread::OnUpdateChecked(NetworkModUpdateInfo& updateInfo, ModUpdateState state, std::optional<KxVersion> version)
	{
		updateInfo.SetState(state);
		updateInfo.SetUpdateCheckDate(m_CurrentDate);
		if (version)
		{
			updateInfo.SetVersion(*version);
		}

		if (updateInfo.AnyUpdated())
		{
			m_UpdatesCount++;
		}
	}
	void NexusUpdateThread::CheckForSingleUpdate(NetworkModUpdateInfo& updateInfo, IGameMod& gameMod, const NetworkModInfo& modInfo)
	{
		if (modInfo.HasFileID())
		{
			// Use all possible info
			if (const auto* fileUpdatesInfo = GetOrQueryModFiles(modInfo.GetModID()))
			{
				const auto& [files, fileUpdates] = *fileUpdatesInfo;

				auto it = files.find(modInfo.GetFileID());
				if (it != files.end())
				{
					// Find file in updates to see if it was changed
					const ModFileReply& thisFileInfo = it->second;

					// See if there's an update for that file
					auto it = fileUpdates.find(modInfo.GetFileID());
					if (it != fileUpdates.end())
					{
						const NexusModFileUpdateReply& fileUpdateInfo = it->second;
						const ModFileReply& newFileInfo = files.find(fileUpdateInfo.NewID)->second;

						// We have an update, look up new version
						OnUpdateChecked(updateInfo, ModUpdateState::ModFileUpdated, newFileInfo.Version);
					}
					else
					{
						// Check if marked as old file
						if (thisFileInfo.Category == ModFileCategory::OldVersion)
						{
							updateInfo.ModDetails(ModUpdateDetails::MarkedOld);
						}
						else
						{
							updateInfo.SetDetails(ModUpdateDetails::None);
						}

						// No updates available
						OnUpdateChecked(updateInfo, ModUpdateState::NoUpdates, gameMod.GetVersion());
					}
				}
				else
				{
					// File has been deleted
					OnUpdateChecked(updateInfo, ModUpdateState::ModFileDeleted, gameMod.GetVersion());
				}
			}
			else
			{
				OnUpdateChecked(updateInfo, ModUpdateState::ModDeleted, gameMod.GetVersion());
			}
		}
		else
		{
			// We have only mod ID, using overall mod version
			if (const ModInfoReply* reply = GetOrQueryModInfo(modInfo.GetModID()))
			{
				if (reply->Version > gameMod.GetVersion())
				{
					OnUpdateChecked(updateInfo, ModUpdateState::ModUpdated, reply->Version);
				}
				else
				{
					OnUpdateChecked(updateInfo, ModUpdateState::NoUpdates, gameMod.GetVersion());
				}
			}
			else
			{
				OnUpdateChecked(updateInfo, ModUpdateState::ModDeleted, gameMod.GetVersion());
			}
		}
	}

	void NexusUpdateThread::OnExecute(KxThreadEvent& event)
	{
		using namespace Utility;
		using namespace Utility::Log;

		m_CurrentDate = DateTime::Now();

		// Get new activity list if needed (we don't have at all it or it's outdated)
		if (!m_MonthlyModActivity || DateTime::IsLaterThanBy(m_CurrentDate, m_MonthlyModActivityDate, m_UpdateChecker.GetAutomaticCheckInterval()))
		{
			m_MonthlyModActivity = m_UpdateChecker.GetModsActivityFor(ModActivity::Month);
			m_MonthlyModActivityDate = m_CurrentDate;
		}
		if (!m_MonthlyModActivity)
		{
			return;
		}

		IModNetwork& nexus = m_UpdateChecker.GetContainer();
		for (IGameMod* gameMod: IModManager::GetInstance()->GetMods())
		{
			// Stop if there are too little requests left
			if (!m_UpdateChecker.IsAutomaticCheckAllowed())
			{
				INotificationCenter::Notify(nexus,
											KTrf("NetworkManager.RequestQuotaReched", nexus.GetName()),
											KxICON_WARNING
				);
				return;
			}

			// Get mod source item and check if mod is installed
			const ModSourceItem* sourceItem = gameMod->GetModSourceStore().GetItem(nexus);
			if (!sourceItem || !gameMod->IsInstalled())
			{
				continue;
			}
			const NetworkModInfo modInfo = sourceItem->GetModInfo();

			// Get existing update info or create new
			NetworkModUpdateInfo* updateInfo = nullptr;
			if (auto it = m_UpdateInfo.find(sourceItem->GetModInfo()); it != m_UpdateInfo.end())
			{
				updateInfo = &it->second;
			}
			else
			{
				updateInfo = &m_UpdateInfo.emplace(modInfo, NetworkModUpdateInfo()).first->second;
			}

			auto IsLastCheckOlderThan = [this, &updateInfo](const wxTimeSpan& span)
			{
				const wxDateTime checkDate = updateInfo->GetUpdateCheckDate();
				return !checkDate.IsValid() || Utility::DateTime::IsLaterThanBy(m_CurrentDate, checkDate, span);
			};
			auto IsLastCheckOlderThanUpdateInterval = [this, &IsLastCheckOlderThan]()
			{
				return IsLastCheckOlderThan(std::max(m_UpdateChecker.GetAutomaticCheckInterval(), wxTimeSpan::Minutes(5)));
			};

			if (IsLastCheckOlderThan(wxTimeSpan::Days(30)))
			{
				CheckForSingleUpdate(*updateInfo, *gameMod, modInfo);
			}
			else
			{
				auto it = m_MonthlyModActivity->find(modInfo.GetModID());
				if (it != m_MonthlyModActivity->end())
				{
					const NexusModActivityReply& activity = it->second;
					if (activity.LatestModActivity != updateInfo->GetActivityHash())
					{
						CheckForSingleUpdate(*updateInfo, *gameMod, modInfo);
						updateInfo->SetActivityHash(activity.LatestModActivity);
					}
					else
					{
						// Skip full check but check date
						updateInfo->SetUpdateCheckDate(m_CurrentDate);
						LogMessage(wxS("Skipping full mod update check for \"%1\" because no new activity has beed found"), gameMod->GetName());
					}
				}
				else if (updateInfo->GetState() == ModUpdateState::Unknown)
				{
					OnUpdateChecked(*updateInfo, ModUpdateState::NoUpdates, gameMod->GetVersion());
				}
			}
		}
	}

	NexusUpdateThread::NexusUpdateThread(NexusRepository& repository, NexusUpdateChecker& updateChecker)
		:m_Repository(repository), m_UpdateChecker(updateChecker)
	{
		m_Thread.Bind(KxThreadEvent::EvtExecute, &NexusUpdateThread::OnExecute, this);
		m_MonthlyModActivityDate = Utility::DateTime::Now();
	}

	bool NexusUpdateThread::Run(const UpdateInfoMap& updateInfo)
	{
		m_UpdateInfo = updateInfo;
		m_CurrentDate = wxDefaultDateTime;
		m_UpdatesCount = 0;

		if (m_Thread.Create() == wxTHREAD_NO_ERROR)
		{
			return m_Thread.Run() == wxTHREAD_NO_ERROR;
		}
		return false;
	}
}
