#include "stdafx.h"
#include "KDownloadEntry.h"
#include "KDownloadManager.h"
#include "KDownloadWorkspace.h"
#include "Network/KNetwork.h"
#include "GameInstance/KGameInstance.h"
#include "ModManager/KModManager.h"
#include "KQuickThread.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileFinder.h>

void KDownloadEntry::Create()
{
	m_EvtHandler.Bind(KEVT_QUICK_THREAD_END, &KDownloadEntry::OnThreadExit, this);

	if (m_TargetProfile == NULL)
	{
		m_TargetProfile = KGameInstance::GetActive();
	}
}
void KDownloadEntry::CleanupDownload()
{
	m_Session.reset();
	m_Stream.reset();
	m_IsPaused = false;
	m_Speed = 0;

	delete m_Thread;
	m_Thread = NULL;
}
bool KDownloadEntry::RequestNewLink()
{
	auto info = m_Provider->GetFileDownloadLinks(m_FileInfo.GetModID(), m_FileInfo.GetID(), GetTargetProfileID());
	if (!info.empty())
	{
		m_DownloadInfo = info.front();
		return true;
	}
	return false;
}

void KDownloadEntry::DeSerializeDefault(const KxFileItem& fileItem)
{
	m_Provider = KNetwork::GetInstance()->GetCurrentProvider();
	m_TargetProfile = KGameInstance::GetActive();
	m_Date = fileItem.GetModificationTime();
	m_FileInfo.SetName(fileItem.GetName());
	
	if (fileItem.IsOK())
	{
		m_DownloadedSize = fileItem.GetFileSize();
		m_FileInfo.SetSize(fileItem.GetFileSize());
		m_IsFailed = false;
	}
	else
	{
		m_DownloadedSize = 0;
		m_FileInfo.SetSize(-1);
		m_IsFailed = true;
	}
}
bool KDownloadEntry::RestoreDownloadNexus()
{
	wxRegEx reg(u8R"((.*?)\-(\d+)\-(.*)\.)", wxRE_EXTENDED|wxRE_ADVANCED|wxRE_ICASE);
	if (reg.Matches(m_FileInfo.GetName()))
	{
		// Mod ID
		KNetworkModID modID = -1;
		reg.GetMatch(m_FileInfo.GetName(), 2).ToLongLong(&modID);

		if (modID != -1)
		{
			for (KNetworkProvider::FileInfo& fileInfo: m_Provider->GetFilesList(modID, GetTargetProfileID()))
			{
				if (fileInfo.GetName() == m_FileInfo.GetName())
				{
					m_FileInfo = std::move(fileInfo);

					// Fix size discrepancy caused by Nexus sending size in kilobytes
					constexpr const int64_t oneKB = 1024 * 1024;
					const int64_t difference = m_DownloadedSize - m_FileInfo.GetSize();
					if (difference > 0 && difference <= oneKB)
					{
						m_FileInfo.SetSize(m_DownloadedSize);
					}
					return true;
				}
			}
		}

		// If we got here, file is not found on Nexus, but we can try to restore as much as possible from file name
		// Set mod ID
		m_FileInfo.SetModID(modID);

		// Display name
		wxString name = reg.GetMatch(m_FileInfo.GetName(), 1);
		name.Replace("_", " ");
		m_FileInfo.SetDisplayName(name);

		// File version
		wxString version = reg.GetMatch(m_FileInfo.GetName(), 2);
		version.Replace("-", ".");
		m_FileInfo.SetVersion(version);
	}
	return false;
}
bool KDownloadEntry::RestoreDownloadTESALL()
{
	return true;
}
bool KDownloadEntry::RestoreDownloadLoversLab()
{
	return false;
}

void KDownloadEntry::OnDownload(KxCURLEvent& event)
{
	m_Thread->TestDestroy();
	KxCURLStreamReply& reply = static_cast<KxCURLStreamReply&>(event.GetReply());
	KDownloadManager* manager = KDownloadManager::GetInstance();

	// Update file size. Nexus reports file size in KB, so initial info maybe
	// incorrect, see 'KNetworkProviderNexus::ReadFileInfo<T>' function for details.
	// Other providers can return correct sizes, but it's better to request it here.
	int64_t totalSize = event.GetMajorTotal();
	if (totalSize > 0)
	{
		m_FileInfo.SetSize(totalSize + reply.GetResumeFromPosition());
	}

	// Downloaded size
	m_DownloadedSize = event.GetMajorProcessed();

	// Speed
	m_Speed = event.GetSpeed();

	// Queue update view
	if (KDownloadWorkspace::GetInstance()->IsWorkspaceVisible())
	{
		manager->CallAfter([this, manager]()
		{
			manager->OnChangeEntry(*this, true);
		});
	}
}
void KDownloadEntry::OnThreadExit(wxNotifyEvent& event)
{
	m_Thread = NULL;
	CleanupDownload();
}
void KDownloadEntry::DoRun(int64_t resumePos)
{
	m_Stream = std::make_unique<KxFileStream>(GetFullPath(), KxFS_ACCESS_WRITE, resumePos > 0 ? KxFS_DISP_OPEN_EXISTING : KxFS_DISP_CREATE_ALWAYS, KxFS_SHARE_READ);
	if (m_Stream->IsOk())
	{
		// Download session
		m_Session = std::make_unique<KxCURLSession>(m_DownloadInfo.GetURL());
		m_Session->Bind(KxEVT_CURL_DOWNLOAD, &KDownloadEntry::OnDownload, this);

		// Begin download, blocking operation.
		KxCURLStreamReply reply(*m_Stream, resumePos);
		m_Session->Download(reply);

		// Notify result
		if (!m_Session->IsStopped() && reply.IsOK() && m_DownloadedSize == m_Stream->GetLength())
		{
			KDownloadManager::GetInstance()->OnDownloadComplete(*this);
			return;
		}
	}

	m_IsFailed = true;
	if (m_Session->IsStopped())
	{
		KDownloadManager::GetInstance()->OnDownloadStopped(*this);
	}
	else
	{
		KDownloadManager::GetInstance()->OnDownloadFailed(*this);
	}
}

KDownloadEntry::KDownloadEntry()
{
	Create();
}
KDownloadEntry::KDownloadEntry(const KNetworkProvider::DownloadInfo& downloadInfo,
							   const KNetworkProvider::FileInfo& fileInfo,
							   const KNetworkProvider* provider,
							   const KGameID& id
)
	:m_DownloadInfo(downloadInfo), m_FileInfo(fileInfo), m_Date(wxDateTime::Now()), m_TargetProfile(KGameInstance::GetTemplate(id)), m_Provider(provider)
{
	Create();
}
KDownloadEntry::~KDownloadEntry()
{
	CleanupDownload();
}

wxString KDownloadEntry::GetFullPath() const
{
	return KDownloadManager::GetInstance()->GetDownloadsLocation() + '\\' + GetFileInfo().GetName();
}
wxString KDownloadEntry::GetMetaFilePath() const
{
	return KDownloadManager::GetInstance()->GetDownloadsLocation() + '\\' + m_FileInfo.GetName() + ".xml";
}

KGameID KDownloadEntry::GetTargetProfileID() const
{
	return m_TargetProfile ? m_TargetProfile->GetGameID() : KGameIDs::NullGameID;
}
void KDownloadEntry::SetTargetProfile(const KGameID& id)
{
	m_TargetProfile = KGameInstance::GetTemplate(id);
}

const KModEntry* KDownloadEntry::GetMod() const
{
	// Try to find download by its file name first
	KModEntry* mod = KModManager::Get().FindModByName(m_FileInfo.GetName());
	if (mod)
	{
		return mod;
	}

	// Try to find download by its file ID
	if (m_Provider)
	{
		return KModManager::Get().FindModByNetworkModID(m_Provider->GetID(), m_FileInfo.GetModID());
	}
	return NULL;
}

bool KDownloadEntry::IsInstalled() const
{
	const KModEntry* mod = GetMod();
	return mod && mod->IsInstalled();
}

void KDownloadEntry::Stop()
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
void KDownloadEntry::Pause()
{
	if (IsRunning() && m_Session->Pause())
	{
		m_Thread->Pause();
		m_IsPaused = true;
		KDownloadManager::GetInstance()->OnDownloadPaused(*this);
	}
}
void KDownloadEntry::Resume()
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
			KDownloadManager::GetInstance()->OnDownloadResumed(*this);
		}
	}
}
void KDownloadEntry::Run(int64_t resumePos)
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
bool KDownloadEntry::Restart()
{
	if (!IsRunning() && m_Provider)
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

bool KDownloadEntry::RepairBrokedDownload()
{
	if (m_Provider)
	{
		bool isSucceed = false;
		switch (m_Provider->GetID())
		{
			case KNETWORK_PROVIDER_ID_NEXUS:
			{
				isSucceed = RestoreDownloadNexus();
				break;
			}
			case KNETWORK_PROVIDER_ID_TESALL:
			{
				isSucceed = RestoreDownloadTESALL();
				break;
			}
			case KNETWORK_PROVIDER_ID_LOVERSLAB:
			{
				isSucceed = RestoreDownloadLoversLab();
				break;
			}
		};

		if (isSucceed)
		{
			Serialize();
		}
		return isSucceed;
	}
	return false;
}
bool KDownloadEntry::QueryInfo()
{
	if (m_Provider)
	{
		m_FileInfo = m_Provider->GetFileInfo(m_FileInfo.GetModID(), m_FileInfo.GetID(), GetTargetProfileID());
		return m_FileInfo.IsOK();
	}
	return false;
}

bool KDownloadEntry::Serialize(wxOutputStream& stream) const
{
	KxXMLDocument xml;
	KxXMLNode rootNode = xml.NewElement("Download");

	if (m_Provider)
	{
		rootNode.NewElement("Provider").SetValue(m_Provider->GetName());
	}
	if (m_TargetProfile)
	{
		rootNode.NewElement("Game").SetValue(m_TargetProfile->GetGameID().ToString());
	}

	rootNode.NewElement("ModID").SetValue(m_FileInfo.GetModID());
	rootNode.NewElement("FileID").SetValue(m_FileInfo.GetID());
	rootNode.NewElement("Name").SetValue(m_FileInfo.GetName());
	rootNode.NewElement("DisplayName").SetValue(m_FileInfo.GetDisplayName());
	rootNode.NewElement("Version").SetValue(m_FileInfo.GetVersion());
	rootNode.NewElement("Date").SetValue(m_Date.FormatISOCombined());
	
	if (m_FileInfo.HasChangeLog())
	{
		rootNode.NewElement("ChangeLog").SetValue(m_FileInfo.GetChangeLog(), true);
	}
	
	KxXMLNode sizeNode = rootNode.NewElement("Size");
	sizeNode.SetAttribute("Total", m_FileInfo.GetSize());
	sizeNode.SetAttribute("Downloaded", m_DownloadedSize);

	KxXMLNode stateNode = rootNode.NewElement("State");
	stateNode.SetAttribute("Paused", m_IsPaused);
	stateNode.SetAttribute("Hidden", m_IsHidden);
	stateNode.SetAttribute("Failed", m_IsFailed);
	stateNode.SetAttribute("Installed", IsInstalled());

	return xml.Save(stream);
}
bool KDownloadEntry::DeSerialize(wxInputStream& stream)
{
	KxXMLDocument xml;
	bool loaded = xml.Load(stream);

	KxXMLNode rootNode = xml.GetFirstChildElement("Download");
	if (loaded && rootNode.IsOK())
	{
		m_Provider = KNetwork::GetInstance()->FindProvider(rootNode.GetFirstChildElement("Provider").GetValue());
		m_TargetProfile = KGameInstance::GetTemplate(rootNode.GetFirstChildElement("Game").GetValue());
		if (m_TargetProfile == NULL)
		{
			m_TargetProfile = KGameInstance::GetActive();
		}

		m_FileInfo.SetModID(rootNode.GetFirstChildElement("ModID").GetValueInt(-1));
		m_FileInfo.SetID(rootNode.GetFirstChildElement("FileID").GetValueInt(-1));
		m_FileInfo.SetName(rootNode.GetFirstChildElement("Name").GetValue());
		m_FileInfo.SetDisplayName(rootNode.GetFirstChildElement("DisplayName").GetValue());
		m_FileInfo.SetVersion(rootNode.GetFirstChildElement("Version").GetValue());
		m_Date.ParseISOCombined(rootNode.GetFirstChildElement("Date").GetValue());
		m_FileInfo.SetChangeLog(rootNode.GetFirstChildElement("ChangeLog").GetValue());
		
		KxXMLNode sizeNode = rootNode.GetFirstChildElement("Size");
		m_FileInfo.SetSize(sizeNode.GetAttributeInt("Total", -1));
		m_DownloadedSize = sizeNode.GetAttributeInt("Downloaded", -1);

		KxXMLNode stateNode = rootNode.GetFirstChildElement("State");
		m_IsPaused = stateNode.GetAttributeBool("Paused", false);
		m_IsHidden = stateNode.GetAttributeBool("Hidden", false);
		m_IsFailed = stateNode.GetAttributeBool("Failed", m_DownloadedSize != m_FileInfo.GetSize()) || !KxFile(GetFullPath()).IsFileExist();

		return true;
	}
	return false;
}

bool KDownloadEntry::Serialize() const
{
	KxFileStream stream(GetMetaFilePath(), KxFS_ACCESS_WRITE, KxFS_DISP_CREATE_ALWAYS, KxFS_SHARE_READ);
	return stream.IsOk() && Serialize(stream);
}
bool KDownloadEntry::DeSerialize(const wxString& xmlFile, const KxFileItem& fileItem)
{
	KxFileStream stream(xmlFile, KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	if (stream.IsOk() && DeSerialize(stream))
	{
		if (fileItem.IsOK())
		{
			m_DownloadedSize = fileItem.GetFileSize();
			if (m_DownloadedSize == m_FileInfo.GetSize())
			{
				m_IsPaused = false;
				m_IsFailed = false;
			}
		}
		return true;
	}
	return false;
}
