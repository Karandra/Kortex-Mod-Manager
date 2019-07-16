#include "stdafx.h"
#include "DownloadItem.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <KxFramework/KxFileItem.h>

namespace
{
	constexpr wxChar StreamName[] = wxS(":Kortex.Download");
}

namespace Kortex
{
	bool DownloadItem::Serialize(wxOutputStream& stream) const
	{
		KxXMLDocument xml;
		if (KxXMLNode rootNode = xml.NewElement("Download"); true)
		{
			if (KxXMLNode sourceNode = rootNode.NewElement("Source"); true)
			{
				if (m_ModNetwork)
				{
					sourceNode.SetAttribute("Name", m_ModNetwork->GetName());
				}
				if (m_TargetGame)
				{
					sourceNode.NewElement("Game").SetValue(m_TargetGame);
				}
				sourceNode.NewElement("ModID").SetValue(m_FileInfo.ModID.GetValue());
				sourceNode.NewElement("FileID").SetValue(m_FileInfo.ID.GetValue());
				if (m_DownloadInfo.URI.IsOk())
				{
					sourceNode.NewElement("URI").SetValue(m_DownloadInfo.URI.BuildUnescapedURI());
				}
			}

			if (KxXMLNode infoNode = rootNode.NewElement("Info"); true)
			{
				infoNode.NewElement("DisplayName").SetValue(m_FileInfo.DisplayName);
				infoNode.NewElement("Version").SetValue(m_FileInfo.Version);
				infoNode.NewElement("TotalSize").SetValue(m_FileInfo.Size);

				if (m_DownloadDate.IsValid())
				{
					infoNode.NewElement("DownloadDate").SetValue(m_DownloadDate.FormatISOCombined());
				}

				if (!m_FileInfo.ChangeLog.IsEmpty())
				{
					infoNode.NewElement("ChangeLog").SetValue(m_FileInfo.ChangeLog);
				}
			}

			if (KxXMLNode stateNode = rootNode.NewElement("State"); true)
			{
				stateNode.NewElement("Paused").SetValue(IsPaused());
				stateNode.NewElement("Hidden").SetValue(m_IsHidden);
				stateNode.NewElement("Failed").SetValue(m_IsFailed);
				stateNode.NewElement("Installed").SetValue(IsInstalled());
			}
		}
		return xml.Save(stream);
	}
	bool DownloadItem::Deserialize(wxInputStream& stream)
	{
		KxXMLDocument xml;
		xml.Load(stream);

		if (KxXMLNode rootNode = xml.GetFirstChildElement("Download"); rootNode.IsOK())
		{
			if (KxXMLNode sourceNode = rootNode.GetFirstChildElement("Source"); sourceNode.IsOK())
			{
				m_ModNetwork = INetworkManager::GetInstance()->GetModNetworkByName(sourceNode.GetAttribute("Name"));

				m_TargetGame = sourceNode.GetFirstChildElement("Game").GetValue();
				m_FileInfo.ModID = sourceNode.GetFirstChildElement("ModID").GetValueInt(-1);
				m_FileInfo.ID = sourceNode.GetFirstChildElement("FileID").GetValueInt(-1);
				m_DownloadInfo.URI = sourceNode.GetFirstChildElement("URI").GetValue();
			}

			if (KxXMLNode infoNode = rootNode.GetFirstChildElement("Info"); infoNode.IsOK())
			{
				m_FileInfo.DisplayName = infoNode.GetFirstChildElement("DisplayName").GetValue();
				m_FileInfo.Version = infoNode.GetFirstChildElement("Version").GetValue();
				m_FileInfo.Size = infoNode.GetFirstChildElement("TotalSize").GetValueInt(-1);
				m_FileInfo.ChangeLog = infoNode.GetFirstChildElement("ChangeLog").GetValue();
				m_DownloadDate.ParseISOCombined(infoNode.GetFirstChildElement("DownloadDate").GetValue());
			}

			if (KxXMLNode stateNode = rootNode.GetFirstChildElement("State"); stateNode.IsOK())
			{
				m_IsHidden = stateNode.GetFirstChildElement("Hidden").GetValueBool();
				m_IsFailed = stateNode.GetFirstChildElement("Failed").GetValueBool(m_DownloadedSize != m_FileInfo.Size) || !KxFile(GetFullPath()).IsFileExist();
				m_ShouldResume = stateNode.GetFirstChildElement().GetValueBool("Paused");
			}

			return true;
		}
		return false;
	}

	DownloadItem::DownloadItem(const ModDownloadReply& downloadInfo,
							   const ModFileReply& fileInfo,
							   ModNetworkRepository& modRepository,
							   const GameID& id
	)
		:m_DownloadInfo(downloadInfo), m_FileInfo(fileInfo), m_TargetGame(id)
	{
		SetModRepository(modRepository);
	}

	bool DownloadItem::IsOK() const
	{
		return m_FileInfo.IsOK() && m_DownloadInfo.IsOK();
	}
	wxString DownloadItem::GetFullPath() const
	{
		return IDownloadManager::GetInstance()->GetDownloadsLocation() + wxS('\\') + m_FileInfo.Name;
	}

	const IGameMod* DownloadItem::GetMod() const
	{
		// Try to find download by its network ID first
		if (m_ModNetwork)
		{
			IGameMod* mod = IModManager::GetInstance()->FindModByModNetwork(*m_ModNetwork, m_FileInfo.ModID);
			if (mod)
			{
				return mod;
			}
		}

		// Then try to find by name (using the download file name)
		return IModManager::GetInstance()->FindModByName(m_FileInfo.Name);
	}
	bool DownloadItem::IsInstalled() const
	{
		const IGameMod* mod = GetMod();
		return mod && mod->IsInstalled();
	}

	ModNetworkRepository* DownloadItem::GetModRepository() const
	{
		return m_ModNetwork ? &m_ModNetwork->GetComponent<ModNetworkRepository>() : nullptr;
	}
	void DownloadItem::SetModRepository(ModNetworkRepository& modRepository)
	{
		m_ModNetwork = &modRepository.GetContainer();
	}

	bool DownloadItem::CanVisitSource() const
	{
		return m_FileInfo.IsIDsValid();
	}
	bool DownloadItem::CanQueryInfo() const
	{
		const ModNetworkRepository* repository = GetModRepository();
		return repository && m_TargetGame && !IsRunning();
	}
	bool DownloadItem::QueryInfo()
	{
		if (CanQueryInfo())
		{
			ModNetworkRepository* repository = GetModRepository();
			return repository->QueryDownload(KxFileItem(GetFullPath()), *this, m_FileInfo);
		}
		return false;
	}

	void DownloadItem::LoadDefault(const KxFileItem& fileItem)
	{
		m_ModNetwork = INetworkManager::GetInstance()->GetDefaultModNetwork();
		m_TargetGame = {};
		m_DownloadDate = fileItem.GetModificationTime();
		m_FileInfo.Name = fileItem.GetName();

		if (fileItem.IsOK())
		{
			m_DownloadedSize = fileItem.GetFileSize();
			m_FileInfo.Size = fileItem.GetFileSize();
			m_IsFailed = false;
		}
		else
		{
			m_DownloadedSize = 0;
			m_FileInfo.Size = -1;
			m_IsFailed = true;
		}
	}
	bool DownloadItem::Load(const KxFileItem& fileItem)
	{
		m_IsFailed = true;

		if (fileItem.IsOK())
		{
			KxFileStream stream(fileItem.GetFullPath() + StreamName, KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
			if (stream.IsOk() && Deserialize(stream))
			{
				m_FileInfo.Name = fileItem.GetName();
				m_DownloadedSize = fileItem.GetFileSize();
				m_IsFailed = m_DownloadedSize != m_FileInfo.Size && !IsPaused();

				if (!m_DownloadDate.IsValid())
				{
					m_DownloadDate = fileItem.GetModificationTime();
				}

				return true;
			}
		}
		return false;
	}
	bool DownloadItem::Save() const
	{
		KxFileStream stream(GetFullPath() + StreamName, KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
		return stream.IsOk() && Serialize(stream);
	}

	std::unique_ptr<IDownloadExecutor> DownloadItem::OnExecutorDone()
	{
		OnUpdateProgress();
		m_ShouldResume = false;

		return std::move(m_Executor);
	}
	void DownloadItem::OnUpdateProgress()
	{
		m_IsFailed = m_Executor->IsFailed();
		m_DownloadDate = m_Executor->GetStartDate();
		m_DownloadedSize = m_Executor->GetDownloadedSize();
		m_FileInfo.Size = m_Executor->GetTotalSize();
	}

	bool DownloadItem::CanStart() const
	{
		return IsOK() && !IsRunning() && !IsPaused() && m_ModNetwork;
	}
	bool DownloadItem::Start(int64_t startAt)
	{
		m_Executor = IDownloadManager::GetInstance()->NewDownloadExecutor(*this, m_DownloadInfo.URI, GetFullPath());
		m_IsFailed = false;
		m_ShouldResume = false;

		return m_Executor->Start(startAt);
	}
	bool DownloadItem::Stop()
	{
		if (m_Executor)
		{
			m_Executor->Stop();
			return true;
		}
		return false;
	}

	bool DownloadItem::Pause()
	{
		if (m_Executor)
		{
			return m_Executor->Pause();
		}
		return false;
	}
	bool DownloadItem::Resume()
	{
		if (m_ShouldResume)
		{
			return Start(m_DownloadedSize);
		}
		else if (m_Executor)
		{
			return m_Executor->Resume();
		}
		return false;
	}
}
