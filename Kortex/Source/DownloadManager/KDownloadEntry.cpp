#include "stdafx.h"
#include "KDownloadEntry.h"
#include "KDownloadManager.h"
#include "KDownloadManagerWorkspace.h"
#include "Network/KNetwork.h"
#include "Profile/KProfile.h"
#include "KQuickThread.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxFile.h>

void KDownloadEntry::Create()
{
	m_EvtHandler.Bind(KEVT_QUICK_THREAD_END, &KDownloadEntry::OnThreadExit, this);

	if (m_TargetProfile == NULL)
	{
		m_TargetProfile = KApp::Get().GetCurrentProfile();
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
	auto info = m_Provider->GetFileDownloadLinks(m_FileInfo.GetModID(), m_FileInfo.GetID(), m_TargetProfile ? m_TargetProfile->GetID() : KProfileIDs::NullProfileID);
	if (!info.empty())
	{
		m_DownloadInfo = info.front();
		return true;
	}
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
	if (KDownloadManagerWorkspace::GetInstance()->IsWorkspaceVisible())
	{
		manager->CallAfter([this, manager]()
		{
			manager->OnChangeEntry(*this);
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
							   const KProfileID& id
)
	:m_DownloadInfo(downloadInfo), m_FileInfo(fileInfo), m_Date(wxDateTime::Now()), m_TargetProfile(KProfile::GetProfileTemplate(id)), m_Provider(provider)
{
	Create();
}

wxString KDownloadEntry::GetFullPath() const
{
	return KDownloadManager::GetInstance()->GetDownloadsLocation() + '\\' + GetFileInfo().GetName();
}
wxString KDownloadEntry::GetMetaFilePath() const
{
	return KDownloadManager::GetInstance()->GetDownloadsLocation() + '\\' + m_FileInfo.GetName() + ".xml";
}

void KDownloadEntry::SetTargetProfile(const KProfileID& id)
{
	m_TargetProfile = KProfile::GetProfileTemplate(id);
}

void KDownloadEntry::Stop()
{
	// Set failed if needed
	if (!IsCompleted())
	{
		m_IsFailed = true;
	}
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
			Run(m_DownloadedSize);
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
		rootNode.NewElement("TargetProfile").SetValue(m_TargetProfile->GetID());
	}

	rootNode.NewElement("URL").SetValue(m_DownloadInfo.GetURL());
	rootNode.NewElement("ModID").SetValue(m_FileInfo.GetModID());
	rootNode.NewElement("FileID").SetValue(m_FileInfo.GetID());
	rootNode.NewElement("Name").SetValue(m_FileInfo.GetName());
	rootNode.NewElement("DisplayName").SetValue(m_FileInfo.GetDisplayName());
	rootNode.NewElement("Version").SetValue(m_FileInfo.GetVersion());
	rootNode.NewElement("Date").SetValue(m_Date.FormatISOCombined());
	rootNode.NewElement("Size").SetValue(m_FileInfo.GetSize());
	rootNode.NewElement("DownloadedSize").SetValue(m_DownloadedSize);

	KxXMLNode stateNode = rootNode.NewElement("State");
	stateNode.SetAttribute("Paused", m_IsPaused);
	stateNode.SetAttribute("Hidden", m_IsHidden);
	stateNode.SetAttribute("Failed", m_IsFailed);
	stateNode.SetAttribute("Installed", m_IsInstalled);

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
		m_TargetProfile = KProfile::GetProfileTemplate(rootNode.GetFirstChildElement("TargetProfile").GetValue());
		if (m_TargetProfile == NULL)
		{
			m_TargetProfile = KApp::Get().GetCurrentProfile();
		}

		m_DownloadInfo.SetURL(rootNode.GetFirstChildElement("URL").GetValue());
		m_FileInfo.SetModID(rootNode.GetFirstChildElement("ModID").GetValueInt(-1));
		m_FileInfo.SetID(rootNode.GetFirstChildElement("FileID").GetValueInt(-1));
		m_FileInfo.SetName(rootNode.GetFirstChildElement("Name").GetValue());
		m_FileInfo.SetDisplayName(rootNode.GetFirstChildElement("DisplayName").GetValue());
		m_FileInfo.SetVersion(rootNode.GetFirstChildElement("Version").GetValue());
		m_Date.ParseISOCombined(rootNode.GetFirstChildElement("Date").GetValue());
		m_FileInfo.SetSize(rootNode.GetFirstChildElement("Size").GetValueInt(-1));
		m_DownloadedSize = rootNode.GetFirstChildElement("DownloadedSize").GetValueInt(-1);

		KxXMLNode stateNode = rootNode.GetFirstChildElement("State");
		m_IsPaused = stateNode.GetAttributeBool("Paused", false);
		m_IsHidden = stateNode.GetAttributeBool("Hidden", false);
		m_IsFailed = stateNode.GetAttributeBool("Failed", m_DownloadedSize != m_FileInfo.GetSize()) || !KxFile(GetFullPath()).IsFileExist();
		m_IsInstalled = stateNode.GetAttributeBool("Installed", false);

		return true;
	}
	return false;
}

bool KDownloadEntry::Serialize() const
{
	KxFileStream stream(GetMetaFilePath(), KxFS_ACCESS_WRITE, KxFS_DISP_CREATE_ALWAYS, KxFS_SHARE_READ);
	return stream.IsOk() && Serialize(stream);
}
bool KDownloadEntry::DeSerialize(const wxString& path)
{
	KxFileStream stream(path, KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	return stream.IsOk() && DeSerialize(stream);
}
