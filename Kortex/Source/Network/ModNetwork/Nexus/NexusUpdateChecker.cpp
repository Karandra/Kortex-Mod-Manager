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

namespace Kortex::NetworkManager
{
	void NexusUpdateChecker::OnLoadInstance(IGameInstance& instance, const KxXMLNode& networkNode)
	{
		// Default interval is 15 minutes, minimum is 5 minutes
		constexpr int defaultInterval = 15;
		constexpr int minInterval = 5;

		// Get the interval
		m_AutomaticCheckInterval = wxTimeSpan::Seconds(networkNode.GetFirstChildElement(wxS("AutomaticCheckInterval")).GetValueFloat() * 60.0);

		// Don't allow to query often than minimum interval
		if (m_AutomaticCheckInterval < wxTimeSpan::Minutes(minInterval))
		{
			m_AutomaticCheckInterval = wxTimeSpan::Minutes(defaultInterval);
		}

		// Start the timer
		if (m_AutomaticCheckInterval.IsPositive())
		{
			m_Timer.BindFunction(&NexusUpdateChecker::OnTimer, this);
			m_Timer.Start(m_AutomaticCheckInterval.GetMilliseconds().GetValue());
		}
	}
	auto NexusUpdateChecker::GetModsActivityFor(ModActivity interval) const -> std::optional<ActivityMap>
	{
		auto ModActivityToString = [](ModActivity interval)
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
		};

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
			RunUpdateCheck();
		}
	}
	void NexusUpdateChecker::OnThreadFinished(KxThreadEvent& event)
	{
		m_UpdateInfo = m_Thread.TakeUpdateInfo();
		m_LastCheckDate = m_Thread.GetCheckDate();
		SaveUpdateInfo();

		//INotificationCenter::Notify(m_Nexus, KTrf("NetworkManager.UpdateCheck.AutoCheckDone", m_Thread.GetUpdatesCount()), KxICON_INFORMATION);
		m_UpdateCheckInProgress = false;
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
			m_LastCheckDate = KxFile(stream.GetFileName()).GetFileTime(KxFILETIME_MODIFICATION);
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

	NexusUpdateChecker::NexusUpdateChecker(NexusModNetwork& nexus, NexusUtility& utility, NexusRepository& repository)
		:m_Nexus(nexus), m_Utility(utility), m_Repository(repository), m_Thread(repository, *this)
	{
		m_Thread.GetEvtHandler().Bind(KxThreadEvent::EvtFinished, &NexusUpdateChecker::OnThreadFinished, this);
	}

	bool NexusUpdateChecker::RunUpdateCheck()
	{
		if (!m_UpdateCheckInProgress)
		{
			if (!m_Repository.IsAutomaticUpdateCheckAllowed())
			{
				INotificationCenter::Notify(m_Nexus, KTr("NetworkManager.UpdateCheck.AutoCheckQuoteReqched"), KxICON_WARNING);
				return false;
			}

			m_UpdateCheckInProgress = true;
			//INotificationCenter::Notify(m_Nexus, KTr("NetworkManager.UpdateCheck.AutoCheckStarted"), KxICON_INFORMATION);

			if (m_Thread.Run(m_UpdateInfo))
			{
				return true;
			}
			m_UpdateCheckInProgress = false;
		}
		return false;
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
