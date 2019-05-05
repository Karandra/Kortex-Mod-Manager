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
#include "UI/KMainWindow.h"
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxJSON.h>

namespace Kortex::NetworkManager
{
	void NexusUpdateChecker::OnLoadInstance(IGameInstance& instance, const KxXMLNode& networkNode)
	{
		// Default interval is 5 minutes
		constexpr int defaultIntervalMin = 5;

		// Get the interval
		m_AutomaticCheckInterval = wxTimeSpan::Seconds(networkNode.GetFirstChildElement(wxS("ModUpdatesCheckInterval")).GetValueFloat() * 60.0);

		// Don't allow to query often than once per minute
		if (m_AutomaticCheckInterval < wxTimeSpan::Minutes(1))
		{
			m_AutomaticCheckInterval = wxTimeSpan::Minutes(defaultIntervalMin);
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
	wxString NexusUpdateChecker::GetModsActivityFor(ModActivity interval)
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
			return reply.AsString();
		}
		return {};
	}
	const KxJSONObject* NexusUpdateChecker::GetOrQueryActivity(ModActivity interval)
	{
		if (m_MonthlyModActivity.empty())
		{
			try
			{
				m_MonthlyModActivity = KxJSON::Load(GetModsActivityFor(interval));
				return &m_MonthlyModActivity;
			}
			catch (...)
			{
				return nullptr;
			}
		}
		else
		{
			return &m_MonthlyModActivity;
		}
	}
	
	void NexusUpdateChecker::CheckModUpdates()
	{
		IModManager* modManager = IModManager::GetInstance();
		const wxDateTime currentDate = wxDateTime::UNow();

		std::unordered_map<ModID::TValue, ModInfoReply> modInfoReplies;
		std::unordered_map<ModID::TValue, NexusRepository::GetModFiles2Result> modFileReplies;

		for (auto& gameMod: modManager->GetMods())
		{
			ModSourceItem* sourceItem = gameMod->GetModSourceStore().GetItem(m_Nexus);
			if (!sourceItem || !gameMod->IsInstalled())
			{
				continue;
			}

			const wxDateTime lastUpdateCheck = GetLastUpdateCheck(sourceItem->GetModInfo());
			auto IsLastCheckOlderThanDays = [&currentDate, &lastUpdateCheck](int days)
			{
				return !lastUpdateCheck.IsValid() || !lastUpdateCheck.IsEqualUpTo(currentDate, wxTimeSpan::Days(days));
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
			
			auto OnChecked = [this, &currentDate](const ModSourceItem& sourceItem, bool hasNewVersion)
			{
				SetLastUpdateCheck(sourceItem, currentDate, hasNewVersion);
			};
			auto CheckForSingleUpdate = [this, &GetOrQueryModInfo, &GetOrQueryModFiles, &OnChecked](ModID modID, ModFileID fileID, const IGameMod& gameMod, const ModSourceItem& sourceItem)
			{
				const NetworkModInfo modInfo = sourceItem.GetModInfo();
				if (modInfo.GetModID() == modID && modInfo.GetFileID() == fileID)
				{
					// Full match, use all possible info
					const NetworkManager::NexusRepository::GetModFiles2Result* filesUpdateInfo = GetOrQueryModFiles(modID);
					if (filesUpdateInfo)
					{
						// TODO
						OnChecked(sourceItem, false);
					}
					return true;
				}
				else if (modInfo.GetModID() == modID)
				{
					// Only mod ID matches, using only mod version
					const ModInfoReply* reply = GetOrQueryModInfo(modID);
					const bool hasNewVersion = reply && reply->Version > gameMod.GetVersion();
					OnChecked(sourceItem, hasNewVersion);

					return true;
				}
				return false;
			};
			
			const KxJSONObject* json = nullptr;
			if (IsLastCheckOlderThanDays(30))
			{
				if (const KxJSONObject* json = GetOrQueryActivity(ModActivity::Month))
				{
					for (const KxJSONObject& updatedItem: *json)
					{
						ModID modID = updatedItem["mod_id"].get<ModID::TValue>();
						ModFileID fileID = updatedItem["latest_file_update"].get<ModFileID::TValue>();

						if (CheckForSingleUpdate(modID, fileID, *gameMod, *sourceItem))
						{
							break;
						}
					}
				}
			}
			else if (IsLastCheckOlderThanDays(1))
			{
				const NetworkModInfo modInfo = sourceItem->GetModInfo();
				CheckForSingleUpdate(modInfo.GetModID(), modInfo.GetFileID(), *gameMod, *sourceItem);
			}
		}
	}

	void NexusUpdateChecker::OnInit()
	{
		// Start the timer
		if (m_AutomaticCheckInterval.IsPositive())
		{
			m_Timer.BindFunction(&NexusUpdateChecker::OnTimer, this);
			m_Timer.Start(m_AutomaticCheckInterval.GetMilliseconds().GetValue());
		}
	}
	void NexusUpdateChecker::OnUninit()
	{
		m_Timer.Stop();
	}
	void NexusUpdateChecker::OnTimer()
	{
		m_TimeElapsed += wxTimeSpan::Milliseconds(m_Timer.GetInterval());

		KMainWindow* mainWindow = KMainWindow::GetInstance();
		if (mainWindow && IApplication::GetInstance()->GetActiveWindow() && IsAutomaticCheckAllowed())
		{
			CheckModUpdates();
		}
	}

	bool NexusUpdateChecker::IsAutomaticCheckAllowed() const
	{
		return m_Repository.IsAutomaticUpdateCheckAllowed();
	}
	wxTimeSpan NexusUpdateChecker::GetAutomaticCheckInterval() const
	{
		return m_AutomaticCheckInterval;
	}

	bool NexusUpdateChecker::HasNewVesion(const NetworkModInfo& modInfo) const
	{
		auto it = m_UpdateInfo.find(modInfo);
		if (it != m_UpdateInfo.end())
		{
			return it->second.second;
		}
		return false;
	}
	wxDateTime NexusUpdateChecker::GetLastUpdateCheck(const NetworkModInfo& modInfo) const
	{
		auto it = m_UpdateInfo.find(modInfo);
		if (it != m_UpdateInfo.end())
		{
			return it->second.first;
		}
		return {};
	}
	void NexusUpdateChecker::SetLastUpdateCheck(const ModSourceItem& sourceItem, const wxDateTime& date, bool hasNewVersion)
	{
		const NetworkModInfo modInfo = sourceItem.GetModInfo();

		auto it = m_UpdateInfo.find(modInfo);
		if (it != m_UpdateInfo.end())
		{
			it->second.first = date;
		}
		else
		{
			m_UpdateInfo.insert_or_assign(modInfo, std::pair(date, hasNewVersion));
		}
	}
}
