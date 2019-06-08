#include "stdafx.h"
#include "NexusUpdateChecker.h"
#include "NexusRepository.h"
#include "NexusUtility.h"
#include "Nexus.h"
#include "Network/ModSourceStore.h"
#include "GameMods/IModManager.h"
#include "GameMods/IGameMod.h"
#include "Application/IApplication.h"
#include "Application/INotificationCenter.h"
#include "Utility/DateTime.h"
#include "Utility/Log.h"
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxJSON.h>
#include <KxFramework/KxXML.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxCallAtScopeExit.h>
#include <thread>

namespace Kortex::NetworkManager
{
	void NexusUpdateChecker::OnLoadInstance(IGameInstance& instance, const KxXMLNode& networkNode)
	{
		// Default interval is 5 minutes
		constexpr int defaultIntervalMin = 5;

		// Get the interval
		m_AutomaticCheckInterval = wxTimeSpan::Seconds(networkNode.GetFirstChildElement(wxS("AutomaticCheckInterval")).GetValueFloat() * 60.0);

		// Don't allow to query often than once per minute
		if (m_AutomaticCheckInterval < wxTimeSpan::Minutes(1))
		{
			m_AutomaticCheckInterval = wxTimeSpan::Minutes(defaultIntervalMin);
		}

		// Start the timer
		if (m_AutomaticCheckInterval.IsPositive())
		{
			m_Timer.BindFunction(&NexusUpdateChecker::OnTimer, this);
			m_Timer.Start(m_AutomaticCheckInterval.GetMilliseconds().GetValue());
		}
	}

	wxString NexusUpdateChecker::ModActivityToString(ModActivity interval) const
	{
		switch (interval)
		{
			case ModActivity::Day:
			{
				return wxS('d');
			}
			case ModActivity::Week:
			{
				return wxS('w');
			}
		};
		return wxS('m');
	}
	auto NexusUpdateChecker::GetModsActivityFor(ModActivity interval) const -> std::optional<ActivityMap>
	{
		// Get updates for last day/week/month (only one, no check for updates for last three months are supported).
		auto connection = m_Nexus.NewCURLSession(KxString::Format(wxS("%1/games/%2/mods/updated?period=1%3"),
												 m_Nexus.GetAPIURL(),
												 m_Nexus.TranslateGameIDToNetwork(),
												 ModActivityToString(interval))
		);
		KxCURLReply reply = connection->Send();
		if (m_Utility.TestRequestErrorSilent(reply).IsSuccessful())
		{
			try
			{
				ActivityMap activityMap;

				const KxJSONObject json = KxJSON::Load(reply.AsString());
				for (const KxJSONObject& updateItem: json)
				{
					ModID modID = updateItem["mod_id"].get<ModID::TValue>();
					ModFileID fileID = updateItem["latest_file_update"].get<ModFileID::TValue>();
					int64_t activityID = updateItem["latest_mod_activity"];

					NexusModActivityReply& value = activityMap.emplace(modID, NexusModActivityReply()).first->second;
					value.ModID = modID;
					value.LatestFileUpdate = fileID;
					value.LatestModActivity = activityID;
				}
				return activityMap;
			}
			catch (...)
			{
				return std::nullopt;
			}
		}
		return std::nullopt;
	}
	
	void NexusUpdateChecker::DoRunUpdateCheckEntry(OnUpdateEvent onUpdate, size_t& updatesCount)
	{
		using namespace Utility::Log;

		IModManager* modManager = IModManager::GetInstance();
		const wxDateTime currentDate = Utility::DateTime::Now();

		// Get new activity list if needed (we don't have at all it or it's outdated)
		if (!m_MonthlyModActivity || Utility::DateTime::IsLaterThanBy(currentDate, m_MonthlyModActivityDate, m_AutomaticCheckInterval))
		{
			m_MonthlyModActivity = GetModsActivityFor(ModActivity::Month);
			m_MonthlyModActivityDate = currentDate;
		}
		if (!m_MonthlyModActivity)
		{
			return;
		}

		std::unordered_map<ModID::TValue, ModInfoReply> modInfoReplies;
		std::unordered_map<ModID::TValue, NexusRepository::GetModFiles2Result> modFileReplies;

		for (auto& gameMod: modManager->GetMods())
		{
			// Stop if there are too little requests left
			if (!m_Repository.IsAutomaticUpdateCheckAllowed())
			{
				INotificationCenter::GetInstance()->Notify(m_Nexus, KTrf("NetworkManager.RequestQuotaReched", m_Nexus.GetName()), KxICON_WARNING);
				return;
			}

			// Get mod source item and check if mod is installed
			const ModSourceItem* sourceItem = gameMod->GetModSourceStore().GetItem(m_Nexus);
			if (!sourceItem || !gameMod->IsInstalled())
			{
				continue;
			}
			const NetworkModInfo modInfo = sourceItem->GetModInfo();

			// Get existing update info or create new
			NetworkModUpdateInfo* updateInfo = GetUpdateInfoPtr(sourceItem->GetModInfo());
			if (updateInfo == nullptr)
			{
				updateInfo = &m_UpdateInfo.emplace(modInfo, NetworkModUpdateInfo()).first->second;
			}

			auto IsLastCheckOlderThan = [&currentDate, &updateInfo](const wxTimeSpan& span)
			{
				const wxDateTime checkDate = updateInfo->GetUpdateCheckDate();
				return !checkDate.IsValid() || Utility::DateTime::IsLaterThanBy(currentDate, checkDate, span);
			};
			auto IsLastCheckOlderThanUpdateInterval = [this, &IsLastCheckOlderThan]()
			{
				return IsLastCheckOlderThan(std::max(m_AutomaticCheckInterval, wxTimeSpan::Minutes(5)));
			};

			auto GetOrQueryModInfo = [this, &modInfoReplies](ModID id) -> const ModInfoReply*
			{
				auto it = modInfoReplies.find(id.GetValue());
				if (it != modInfoReplies.end())
				{
					return &it->second;
				}
				else if (auto reply = m_Repository.GetModInfo(id))
				{
					return &modInfoReplies.insert_or_assign(id.GetValue(), std::move(*reply)).first->second;
				}
				return nullptr;
			};
			auto GetOrQueryModFiles = [this, &modFileReplies](ModID id) -> const NexusRepository::GetModFiles2Result*
			{
				auto it = modFileReplies.find(id.GetValue());
				if (it != modFileReplies.end())
				{
					return &it->second;
				}
				else if (auto files = m_Repository.GetModFiles2(id, true, true))
				{
					return &modFileReplies.insert_or_assign(id.GetValue(), std::move(*files)).first->second;
				}
				return nullptr;
			};
			
			auto OnUpdateChecked = [&](ModUpdateState state, std::optional<KxVersion> version = {})
			{
				updateInfo->SetState(state);
				updateInfo->SetUpdateCheckDate(currentDate);
				if (version)
				{
					updateInfo->SetVersion(*version);
				}

				if (updateInfo->AnyUpdated())
				{
					updatesCount++;
				}
			};
			auto CheckForSingleUpdate = [&, this]()
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
								OnUpdateChecked(ModUpdateState::ModFileUpdated, newFileInfo.Version);
							}
							else
							{
								// Check if marked as old file
								if (thisFileInfo.Category == ModFileCategory::OldVersion)
								{
									updateInfo->ModDetails(ModUpdateDetails::MarkedOld);
								}
								else
								{
									updateInfo->SetDetails(ModUpdateDetails::None);
								}

								// No updates available
								OnUpdateChecked(ModUpdateState::NoUpdates, gameMod->GetVersion());
							}
						}
						else
						{
							// File has been deleted
							OnUpdateChecked(ModUpdateState::ModFileDeleted, gameMod->GetVersion());
						}
					}
					else
					{
						OnUpdateChecked(ModUpdateState::ModDeleted, gameMod->GetVersion());
					}
				}
				else
				{
					// We have only mod ID, using overall mod version
					if (const ModInfoReply* reply = GetOrQueryModInfo(modInfo.GetModID()))
					{
						if (reply->Version > gameMod->GetVersion())
						{
							OnUpdateChecked(ModUpdateState::ModUpdated, gameMod->GetVersion());
						}
						else
						{
							OnUpdateChecked(ModUpdateState::NoUpdates, gameMod->GetVersion());
						}
					}
					else
					{
						OnUpdateChecked(ModUpdateState::ModDeleted, gameMod->GetVersion());
					}
				}
			};
			
			if (IsLastCheckOlderThan(wxTimeSpan::Days(30)))
			{
				CheckForSingleUpdate();
			}
			else
			{
				auto it = m_MonthlyModActivity->find(modInfo.GetModID());
				if (it != m_MonthlyModActivity->end())
				{
					const NexusModActivityReply& activity = it->second;
					if (activity.LatestModActivity != updateInfo->GetActivityHash())
					{
						CheckForSingleUpdate();
						updateInfo->SetActivityHash(activity.LatestModActivity);
					}
					else
					{
						// Skip full check but check date
						updateInfo->SetUpdateCheckDate(currentDate);
						LogMessage(wxS("Skipping full mod update check for \"%1\" because no new activity has beed found"), gameMod->GetName());
					}
				}
				else if (updateInfo->GetState() == ModUpdateState::Unknown)
				{
					OnUpdateChecked(ModUpdateState::NoUpdates, gameMod->GetVersion());
				}
			}

			if (onUpdate)
			{
				onUpdate(*gameMod, *updateInfo);
			}
		}
	}
	bool NexusUpdateChecker::DoRunUpdateCheck(OnUpdateEvent onUpdate, OnUpdateDoneEvent onDone)
	{
		if (!m_UpdateCheckInProgress)
		{
			if (!m_Repository.IsAutomaticUpdateCheckAllowed())
			{
				INotificationCenter::GetInstance()->Notify(m_Nexus, KTr("NetworkManager.UpdateCheck.AutoCheckQuoteReqched"), KxICON_WARNING);
				return false;
			}

			std::thread([this, onUpdate = std::move(onUpdate), onDone = std::move(onDone)]()
			{
				m_UpdateCheckInProgress = true;
				INotificationCenter::GetInstance()->Notify(m_Nexus, KTr("NetworkManager.UpdateCheck.AutoCheckStarted"), KxICON_INFORMATION);

				size_t updatesCount = 0;
				DoRunUpdateCheckEntry(std::move(onUpdate), updatesCount);

				if (onDone)
				{
					onDone();
				}
				INotificationCenter::GetInstance()->Notify(m_Nexus, KTrf("NetworkManager.UpdateCheck.AutoCheckDone", updatesCount), KxICON_INFORMATION);
				
				SaveUpdateInfo();
				m_UpdateCheckInProgress = false;
			}).detach();
			return true;
		}
		return false;
	}

	void NexusUpdateChecker::OnInit()
	{
		LoadUpdateInfo();
	}
	void NexusUpdateChecker::OnUninit()
	{
		m_Timer.Stop();
	}
	void NexusUpdateChecker::OnTimer(wxTimerEvent& event)
	{
		m_TimeElapsed += wxTimeSpan::Milliseconds(event.GetInterval());

		if (IApplication::GetInstance()->IsMainWindowActive() && IsAutomaticCheckAllowed())
		{
			DoRunUpdateCheck();
		}
	}

	wxString NexusUpdateChecker::GetUpdateInfoFile() const
	{
		return m_Nexus.GetLocationInCache(KxString::Format(wxS("UpdateInfo-%1.xml"), KAux::MakeSafeFileName(m_Nexus.TranslateGameIDToNetwork())));
	}
	bool NexusUpdateChecker::SaveUpdateInfo()
	{
		KxFileStream stream(GetUpdateInfoFile(), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways);
		if (stream.IsOk())
		{
			KxXMLDocument xml;
			KxXMLNode rootNode = xml.NewElement(wxS("UpdateInfo"));

			const IModManager* modManager = IModManager::GetInstance();
			for (auto& [modInfo, updateInfo]: m_UpdateInfo)
			{
				if (modInfo.HasModID())
				{
					KxXMLNode node = rootNode.NewElement(wxS("Entry"));

					// Required mod info
					node.SetAttribute(wxS("ModID"), modInfo.GetModID().GetValue());
					if (modInfo.HasFileID())
					{
						node.SetAttribute(wxS("FileID"), modInfo.GetFileID().GetValue());
					}

					// Optional info that doesn't really needed by the updater, but useful to have in update info file
					if (const IGameMod* gameMod = modManager->FindModByModNetwork(m_Nexus, NetworkModInfo(modInfo.GetModID(), modInfo.GetFileID())))
					{
						node.NewElement(wxS("Name")).SetValue(gameMod->GetName());
					}

					// Update info
					updateInfo.Save(node);
				}
			}
			return xml.Save(stream);
		}
		return false;
	}
	bool NexusUpdateChecker::LoadUpdateInfo()
	{
		KxFileStream stream(GetUpdateInfoFile(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting);
		if (KxXMLDocument xml; stream.IsOk() && xml.Load(stream))
		{
			m_MonthlyModActivityDate = KxFile(stream.GetFileName()).GetFileTime(KxFILETIME_MODIFICATION);
			m_UpdateInfo.clear();

			KxXMLNode rootNode = xml.GetFirstChildElement(wxS("UpdateInfo"));
			for (KxXMLNode node = rootNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
			{
				ModID modID = node.GetAttributeInt(wxS("ModID"), ModID::GetInvalidValue());
				ModFileID fileID = node.GetAttributeInt(wxS("FileID"), ModFileID::GetInvalidValue());

				wxDateTime lastUpdateCheck;
				lastUpdateCheck.ParseISOCombined(node.GetFirstChildElement(wxS("LastUpdateCheck")).GetValue());

				NetworkModUpdateInfo updateInfo;
				updateInfo.Load(node);

				m_UpdateInfo.insert_or_assign(NetworkModInfo(modID, fileID), updateInfo);
			}
			return true;
		}
		return false;
	}

	NetworkModUpdateInfo* NexusUpdateChecker::GetUpdateInfoPtr(const NetworkModInfo& modInfo)
	{
		auto it = m_UpdateInfo.find(modInfo);
		if (it != m_UpdateInfo.end())
		{
			return &it->second;
		}
		return nullptr;
	}
	const NetworkModUpdateInfo* NexusUpdateChecker::GetUpdateInfoPtr(const NetworkModInfo& modInfo) const
	{
		return const_cast<NexusUpdateChecker*>(this)->GetUpdateInfoPtr(modInfo);
	}

	bool NexusUpdateChecker::RunUpdateCheck(OnUpdateEvent onUpdate, OnUpdateDoneEvent onDone)
	{
		return DoRunUpdateCheck(std::move(onUpdate), std::move(onDone));
	}

	bool NexusUpdateChecker::IsAutomaticCheckAllowed() const
	{
		return m_Repository.IsAutomaticUpdateCheckAllowed();
	}
	wxTimeSpan NexusUpdateChecker::GetAutomaticCheckInterval() const
	{
		return m_AutomaticCheckInterval;
	}

	NetworkModUpdateInfo NexusUpdateChecker::GetUpdateInfo(const NetworkModInfo& modInfo) const
	{
		if (const NetworkModUpdateInfo* info = GetUpdateInfoPtr(modInfo))
		{
			return *info;
		}
		return {};
	}
}
