#include "stdafx.h"
#include "DownloadItem.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <KxFramework/KxFileItem.h>
#include "Archive/Common.h"
#include "Utility/Common.h"

namespace
{
	constexpr wxChar g_StreamName[] = wxS(":Kortex.Download");
	constexpr wxChar g_TempDownloadSuffix[] = wxS(".tmp");
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
				if (m_FileInfo.ModID)
				{
					sourceNode.NewElement("ModID").SetValue(m_FileInfo.ModID.GetValue());
				}
				if (m_FileInfo.ID)
				{
					sourceNode.NewElement("FileID").SetValue(m_FileInfo.ID.GetValue());
				}
				if (m_DownloadInfo.URI.IsOk())
				{
					sourceNode.NewElement("URI").SetValue(m_DownloadInfo.URI.BuildUnescapedURI());
				}
			}

			if (KxXMLNode infoNode = rootNode.NewElement("Info"); true)
			{
				infoNode.NewElement("DisplayName").SetValue(GetDisplayName());
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
				m_IsFailed = stateNode.GetFirstChildElement("Failed").GetValueBool(m_DownloadedSize != m_FileInfo.Size) || !KxFile(GetLocalPath()).IsFileExist();
				m_ShouldResume = stateNode.GetFirstChildElement().GetValueBool("Paused");
			}

			return true;
		}
		return false;
	}

	wxString DownloadItem::ConstructFileName() const
	{
		wxString name = m_FileInfo.Name;
		if (name.IsEmpty())
		{
			name = m_FileInfo.DisplayName;
			if (name.IsEmpty())
			{
				name = m_DownloadInfo.URI.GetPath().AfterLast(wxS('/'));
			}
		}

		return Utility::MakeSafeFileName(name);
	}
	bool DownloadItem::ChangeFileName(const wxString& newName)
	{
		if (IsCompleted())
		{
			m_FileInfo.Name = newName;
			return true;
		}
		return false;
	}
	bool DownloadItem::DoStart(int64_t startAt)
	{
		m_Executor = IDownloadManager::GetInstance()->NewDownloadExecutor(*this, m_DownloadInfo.URI, GetLocalPath());
		m_IsWaiting = false;
		m_IsFailed = false;
		m_ShouldResume = false;

		return m_Executor->Start(startAt);
	}

	std::unique_ptr<IDownloadExecutor> DownloadItem::OnExecutorEnd()
	{
		OnExecutorProgress();
		m_ShouldResume = false;

		// If download failed delete the file.
		if (m_IsFailed)
		{
			// This will remove meta-data stored in the ADS so save it again. It will create zero-sized file
			// and it will indicate failed status.
			m_FileInfo.Size = -1;
			Save();
		}
		else if (IsCompleted())
		{
			// If the extension isn't present try to detect archive format and add appropriate extension.
			// File is renamed to its real name by this time.
			if (!m_LocalFullPath.IsEmpty())
			{
				wxString ext = m_LocalFullPath.AfterLast(wxS('.'));
				if (ext.IsEmpty() || ext == m_LocalFullPath)
				{
					ext = Archive::GetExtensionFromFormat(Archive::DetectFormat(m_LocalFullPath));
					if (!ext.IsEmpty())
					{
						// Remember old path
						const wxString oldPath = m_LocalFullPath;

						// Update names
						m_LocalFullPath += wxS('.');
						m_LocalFullPath += ext;
						m_FileInfo.Name = m_LocalFullPath.AfterLast(wxS('\\'));

						// Invoke auto-rename
						if (IDownloadManager::GetInstance()->AutoRenameIncrement(*this))
						{
							// Apply name changes to full path
							KxFileItem item;
							item.SetFullPath(m_LocalFullPath);
							item.SetName(m_FileInfo.Name);

							m_LocalFullPath = item.GetFullPath();
						}

						// Do rename
						KxFile(oldPath).Rename(m_LocalFullPath, true);
					}
				}
			}
		}

		return std::move(m_Executor);
	}
	void DownloadItem::OnExecutorProgress()
	{
		m_IsFailed = m_Executor->IsFailed();
		m_DownloadSpeed = m_Executor->GetSpeed();
		m_DownloadedSize = m_Executor->GetDownloadedSize();
		m_FileInfo.Size = m_Executor->GetTotalSize();
		m_DownloadDate = m_Executor->GetStartDate();

		if (m_LocalFullPath.IsEmpty())
		{
			m_LocalFullPath = m_Executor->GetLocalPath();
		}
		if (!m_LocalFullTempPath.IsEmpty())
		{
			m_LocalFullTempPath = m_Executor->GetLocalTempPath();
		}
	}

	DownloadItem::DownloadItem(const ModDownloadReply& downloadInfo,
							   const ModFileReply& fileInfo,
							   ModNetworkRepository& modRepository,
							   const GameID& id
	)
		:m_DownloadInfo(downloadInfo), m_FileInfo(fileInfo), m_TargetGame(id)
	{
		SetModRepository(modRepository);

		if (m_FileInfo.Name.IsEmpty() && m_FileInfo.DisplayName.IsEmpty())
		{
			m_FileInfo.Name = ConstructFileName();
		}
	}

	bool DownloadItem::IsOK() const
	{
		return m_FileInfo.IsOK() && (!m_FileInfo.Name.IsEmpty() || !m_FileInfo.DisplayName.IsEmpty()) && m_DownloadInfo.URI.IsOk();
	}
	wxString DownloadItem::GetLocalPath() const
	{
		if (!m_LocalFullPath.IsEmpty())
		{
			return m_LocalFullPath;
		}
		return IDownloadManager::GetInstance()->GetDownloadsLocation() + wxS('\\') + GetName();
	}
	wxString DownloadItem::GetLocalTempPath() const
	{
		if (!m_LocalFullTempPath.IsEmpty())
		{
			return m_LocalFullTempPath;
		}
		return GetLocalPath() + g_TempDownloadSuffix;
	}
	wxString DownloadItem::GetTempPathSuffix() const
	{
		return g_TempDownloadSuffix;
	}

	wxString DownloadItem::GetName() const
	{
		return Utility::MakeSafeFileName(!m_FileInfo.Name.IsEmpty() ? m_FileInfo.Name : m_FileInfo.DisplayName);
	}
	wxString DownloadItem::GetDisplayName() const
	{
		return !m_FileInfo.DisplayName.IsEmpty() ? m_FileInfo.DisplayName : m_FileInfo.Name;
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
		return m_FileInfo.IsOK();
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
			return repository->QueryDownload(KxFileItem(GetLocalPath()), *this, m_FileInfo);
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
			KxFileStream stream(fileItem.GetFullPath() + g_StreamName, KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
			if (stream.IsOk() && Deserialize(stream))
			{
				m_FileInfo.Name = fileItem.GetName();
				m_DownloadedSize = fileItem.GetFileSize();

				// Try to use temp file to get downloaded size if the download were paused
				if (m_ShouldResume)
				{
					KxFileItem tempFile(fileItem.GetFullPath() + g_TempDownloadSuffix);
					if (tempFile.IsNormalItem() && tempFile.IsFile())
					{
						m_DownloadedSize = tempFile.GetFileSize();
					}
				}
				m_IsFailed = m_FileInfo.Size <= 0 || (m_DownloadedSize != m_FileInfo.Size && !m_ShouldResume);

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
		KxFileStream stream(GetLocalPath() + g_StreamName, KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
		return stream.IsOk() && Serialize(stream);
	}

	bool DownloadItem::CanStart() const
	{
		if (IsOK() && !m_IsWaiting && !IsRunning() && !IsPaused() && m_ModNetwork)
		{
			IDownloadManager* manager = IDownloadManager::GetInstance();
			return manager->GetActiveDownloadsCount() < manager->GetMaxConcurrentDownloads();
		}
		return false;
	}
	bool DownloadItem::Start()
	{
		return DoStart();
	}
	bool DownloadItem::Stop()
	{
		if (m_Executor)
		{
			m_Executor->Stop();
			return true;
		}
		else if (m_ShouldResume)
		{
			m_ShouldResume = false;
			return true;
		}
		return false;
	}

	bool DownloadItem::CanResume() const
	{
		if (IsOK() && !m_IsWaiting && !IsRunning() && IsPaused())
		{
			IDownloadManager* manager = IDownloadManager::GetInstance();
			if (manager->HasConcurrentDownloadsLimit())
			{
				return (int)manager->GetActiveDownloadsCount() < manager->GetMaxConcurrentDownloads();
			}
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
			return DoStart(m_DownloadedSize);
		}
		else if (m_Executor)
		{
			return m_Executor->Resume();
		}
		return false;
	}
}
