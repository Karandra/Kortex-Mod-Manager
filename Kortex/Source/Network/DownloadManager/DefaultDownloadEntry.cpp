#include "stdafx.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/GameInstance.hpp>
#include "DefaultDownloadEntry.h"
#include "DefaultDownloadManager.h"
#include "Workspace.h"
#include "DisplayModel.h"
#include "Utility/KQuickThread.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileFinder.h>
#include <chrono>

namespace
{
	int64_t GetClockTime()
	{
		using namespace std::chrono;
		return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
	};
}

namespace Kortex::DownloadManager
{
	void DefaultDownloadEntry::Create()
	{
		m_EvtHandler.Bind(KEVT_QUICK_THREAD_END, &DefaultDownloadEntry::OnThreadExit, this);

		if (m_TargetGame == nullptr)
		{
			m_TargetGame = IGameInstance::GetActive();
		}
	}
	void DefaultDownloadEntry::CleanupDownload()
	{
		m_Session.reset();
		m_Stream.reset();
		m_IsPaused = false;
		m_Speed = 0;
		m_TimeStamp = 0;

		delete m_Thread;
		m_Thread = nullptr;
	}
	bool DefaultDownloadEntry::RequestNewLink()
	{
		ProviderRequest request(m_FileInfo->GetModID(), m_FileInfo->GetID(), GetTargetGameID());
		auto info = m_ModSource->GetFileDownloadLinks(request);
		if (!info.empty())
		{
			m_DownloadInfo = std::move(info.front());
			return true;
		}
		return false;
	}

	void DefaultDownloadEntry::OnDownload(KxCURLEvent& event)
	{
		m_Thread->TestDestroy();
		KxCURLStreamReply& reply = static_cast<KxCURLStreamReply&>(event.GetReply());

		// Update file size. Nexus reports file size in KB, so initial info maybe
		// incorrect, see 'NexusProvider::ReadFileInfo<T>' function for details.
		// Other sources can return correct sizes, but it's better to request it here.
		int64_t totalSize = event.GetMajorTotal();
		if (totalSize > 0)
		{
			m_FileInfo->SetSize(totalSize + reply.GetResumeFromPosition());
		}

		// Downloaded size
		m_DownloadedSize = event.GetMajorProcessed();

		// Speed
		m_Speed = event.GetSpeed();

		// Queue update
		int64_t time = GetClockTime();
		if (time - m_TimeStamp >= 200)
		{
			m_TimeStamp = time;

			IEvent::CallAfter([this]()
			{
				if (Workspace::GetInstance()->IsWorkspaceVisible())
				{
					IDownloadManager::GetInstance()->OnChangeEntry(*this, true);
				}
			});
		}
	}
	void DefaultDownloadEntry::OnThreadExit(wxNotifyEvent& event)
	{
		m_Thread = nullptr;
		CleanupDownload();
	}
	void DefaultDownloadEntry::DoRun(int64_t resumePos)
	{
		KxFileStream::Disposition disposition = resumePos > 0 ? KxFileStream::Disposition::OpenExisting : KxFileStream::Disposition::CreateAlways;
		m_Stream = std::make_unique<KxFileStream>(GetFullPath(), KxFileStream::Access::RW, disposition, KxFileStream::Share::Read);
		if (m_Stream->IsOk())
		{
			// Download session
			m_Session = std::make_unique<KxCURLSession>(m_DownloadInfo->GetURL());
			m_Session->Bind(KxEVT_CURL_DOWNLOAD, &DefaultDownloadEntry::OnDownload, this);

			// Initial view update
			IDownloadManager::GetInstance()->OnChangeEntry(*this, true);

			// Begin download, blocking operation.
			m_TimeStamp = GetClockTime();
			KxCURLStreamReply reply(*m_Stream, resumePos);
			m_Session->Download(reply);

			// Notify result
			if (!m_Session->IsStopped() && reply.IsOK() && m_DownloadedSize == m_Stream->GetLength())
			{
				IDownloadManager::GetInstance()->OnDownloadComplete(*this);
				return;
			}
		}

		m_IsFailed = true;
		if (m_Session && m_Session->IsStopped())
		{
			IDownloadManager::GetInstance()->OnDownloadStopped(*this);
		}
		else
		{
			IDownloadManager::GetInstance()->OnDownloadFailed(*this);
		}
	}

	DefaultDownloadEntry::DefaultDownloadEntry()
		:m_ModSource(INetworkManager::GetInstance()->GetDefaultModSource())
	{
		m_FileInfo = m_ModSource->NewModFileInfo();
		m_DownloadInfo = m_ModSource->NewModDownloadInfo();

		Create();
	}
	DefaultDownloadEntry::DefaultDownloadEntry(const IModDownloadInfo& downloadInfo,
											   const IModFileInfo& fileInfo,
											   IModSource* modSource,
											   const GameID& id
	)
		:m_DownloadInfo(downloadInfo.Clone()),
		m_FileInfo(fileInfo.Clone()),
		m_Date(wxDateTime::Now()),
		m_TargetGame(IGameInstance::GetTemplate(id)),
		m_ModSource(modSource)
	{
		Create();
	}
	DefaultDownloadEntry::~DefaultDownloadEntry()
	{
		CleanupDownload();
	}

	wxString DefaultDownloadEntry::GetFullPath() const
	{
		return IDownloadManager::GetInstance()->GetDownloadsLocation() + '\\' + GetFileInfo().GetName();
	}
	wxString DefaultDownloadEntry::GetMetaFilePath() const
	{
		return IDownloadManager::GetInstance()->GetDownloadsLocation() + '\\' + m_FileInfo->GetName() + ".xml";
	}

	const IGameMod* DefaultDownloadEntry::GetMod() const
	{
		// Try to find download by its file name first
		IGameMod* mod = IModManager::GetInstance()->FindModByName(m_FileInfo->GetName());
		if (mod)
		{
			return mod;
		}

		// Try to find download by its ID
		if (m_ModSource)
		{
			return IModManager::GetInstance()->FindModBySource(*m_ModSource, m_FileInfo->GetModID());
		}
		return nullptr;
	}
	bool DefaultDownloadEntry::IsInstalled() const
	{
		const IGameMod* mod = GetMod();
		return mod && mod->IsInstalled();
	}

	void DefaultDownloadEntry::Stop()
	{
		// If download is paused resume it first
		if (IsPaused())
		{
			if (m_Session)
			{
				m_Session->Resume();
			}
			if (m_Thread)
			{
				m_Thread->Resume();
			}
			m_IsPaused = false;
		}

		// Set failed if needed
		if (!IsCompleted())
		{
			m_IsFailed = true;
		}

		// Stop CURL now. This will cause download thread to terminate itself
		if (m_Session)
		{
			m_Session->Stop();
		}
	}
	void DefaultDownloadEntry::Pause()
	{
		if (IsRunning() && m_Session->Pause())
		{
			m_Thread->Pause();
			m_IsPaused = true;
			IDownloadManager::GetInstance()->OnDownloadPaused(*this);
		}
	}
	void DefaultDownloadEntry::Resume()
	{
		if (IsFailed())
		{
			Run();
		}
		else
		{
			// Resume after restart
			if (!m_Session)
			{
				if (RequestNewLink())
				{
					Run(m_DownloadedSize);
				}
				return;
			}

			if (IsPaused() && m_Session->Resume())
			{
				m_Thread->Resume();
				m_IsPaused = false;
				IDownloadManager::GetInstance()->OnDownloadResumed(*this);
			}
		}
	}
	void DefaultDownloadEntry::Run(int64_t resumePos)
	{
		// Explicitly reset failed flag
		m_IsFailed = false;

		// Usual cleanup
		CleanupDownload();

		// Run thread
		m_Thread = new KQuickThread([this, resumePos](KQuickThread& thread)
		{
			DoRun(resumePos);
		}, &m_EvtHandler);
		m_Thread->Run();
	}
	bool DefaultDownloadEntry::Restart()
	{
		if (!IsRunning() && m_ModSource)
		{
			// Most likely link is expired or unavailable by now so request a new one.
			if (RequestNewLink())
			{
				m_DownloadedSize = 0;
				m_Date = wxDateTime::Now();

				Run();
				return true;
			}
		}
		return false;
	}

	bool DefaultDownloadEntry::RepairBrokedDownload()
	{
		if (m_ModSource && m_ModSource->RestoreBrokenDownload(GetFullPath(), *this))
		{
			Save();
			return true;
		}
		return false;
	}
	bool DefaultDownloadEntry::QueryInfo()
	{
		if (m_ModSource)
		{
			ProviderRequest request(m_FileInfo->GetModID(), m_FileInfo->GetID(), GetTargetGameID());
			m_FileInfo = m_ModSource->GetFileInfo(request);
			return m_FileInfo && m_FileInfo->IsOK();
		}
		return false;
	}

	bool DefaultDownloadEntry::Serialize(wxOutputStream& stream) const
	{
		KxXMLDocument xml;
		KxXMLNode rootNode = xml.NewElement("Download");

		if (m_ModSource)
		{
			rootNode.NewElement("Source").SetValue(m_ModSource->GetName());
		}
		if (m_TargetGame)
		{
			rootNode.NewElement("Game").SetValue(m_TargetGame->GetGameID().ToString());
		}

		rootNode.NewElement("ModID").SetValue(m_FileInfo->GetModID().GetValue());
		rootNode.NewElement("FileID").SetValue(m_FileInfo->GetID().GetValue());
		rootNode.NewElement("Name").SetValue(m_FileInfo->GetName());
		rootNode.NewElement("DisplayName").SetValue(m_FileInfo->GetDisplayName());
		rootNode.NewElement("Version").SetValue(m_FileInfo->GetVersion());
		rootNode.NewElement("Date").SetValue(m_Date.FormatISOCombined());

		if (wxString changeLog = m_FileInfo->GetChangeLog(); !changeLog.IsEmpty())
		{
			rootNode.NewElement("ChangeLog").SetValue(changeLog, true);
		}

		KxXMLNode sizeNode = rootNode.NewElement("Size");
		sizeNode.SetAttribute("Total", m_FileInfo->GetSize());
		sizeNode.SetAttribute("Downloaded", m_DownloadedSize);

		KxXMLNode stateNode = rootNode.NewElement("State");
		stateNode.SetAttribute("Paused", m_IsPaused);
		stateNode.SetAttribute("Hidden", m_IsHidden);
		stateNode.SetAttribute("Failed", m_IsFailed);
		stateNode.SetAttribute("Installed", IsInstalled());

		return xml.Save(stream);
	}
	bool DefaultDownloadEntry::DeSerialize(wxInputStream& stream)
	{
		KxXMLDocument xml;
		bool loaded = xml.Load(stream);

		KxXMLNode rootNode = xml.GetFirstChildElement("Download");
		if (loaded && rootNode.IsOK())
		{
			m_ModSource = INetworkManager::GetInstance()->GetModSource(rootNode.GetFirstChildElement("Source").GetValue());
			if (m_ModSource == nullptr)
			{
				// TODO: maybe I should create 'dummy' modSource?
				m_ModSource = nullptr;
			}

			if (m_ModSource)
			{
				m_FileInfo = m_ModSource->NewModFileInfo();
				m_DownloadInfo = m_ModSource->NewModDownloadInfo();
			}

			m_TargetGame = IGameInstance::GetTemplate(rootNode.GetFirstChildElement("Game").GetValue());
			if (m_TargetGame == nullptr)
			{
				m_TargetGame = IGameInstance::GetActive();
			}

			KxXMLNode sizeNode = rootNode.GetFirstChildElement("Size");
			if (m_FileInfo)
			{
				m_FileInfo->SetModID(rootNode.GetFirstChildElement("ModID").GetValueInt(-1));
				m_FileInfo->SetID(rootNode.GetFirstChildElement("FileID").GetValueInt(-1));
				m_FileInfo->SetName(rootNode.GetFirstChildElement("Name").GetValue());
				m_FileInfo->SetDisplayName(rootNode.GetFirstChildElement("DisplayName").GetValue());
				m_FileInfo->SetVersion(rootNode.GetFirstChildElement("Version").GetValue());
				m_FileInfo->SetChangeLog(rootNode.GetFirstChildElement("ChangeLog").GetValue());
				m_FileInfo->SetSize(sizeNode.GetAttributeInt("Total", -1));
			}

			m_Date.ParseISOCombined(rootNode.GetFirstChildElement("Date").GetValue());
			m_DownloadedSize = sizeNode.GetAttributeInt("Downloaded", -1);
			
			KxXMLNode stateNode = rootNode.GetFirstChildElement("State");
			m_IsPaused = stateNode.GetAttributeBool("Paused", false);
			m_IsHidden = stateNode.GetAttributeBool("Hidden", false);
			m_IsFailed = stateNode.GetAttributeBool("Failed", m_DownloadedSize != m_FileInfo->GetSize()) || !KxFile(GetFullPath()).IsFileExist();

			return true;
		}
		return false;
	}
	void DefaultDownloadEntry::LoadDefault(const KxFileItem& fileItem)
	{
		m_ModSource = INetworkManager::GetInstance()->GetDefaultModSource();
		m_TargetGame = IGameInstance::GetActive();
		m_Date = fileItem.GetModificationTime();
		m_FileInfo->SetName(fileItem.GetName());

		if (fileItem.IsOK())
		{
			m_DownloadedSize = fileItem.GetFileSize();
			m_FileInfo->SetSize(fileItem.GetFileSize());
			m_IsFailed = false;
		}
		else
		{
			m_DownloadedSize = 0;
			m_FileInfo->SetSize(-1);
			m_IsFailed = true;
		}
	}
}
