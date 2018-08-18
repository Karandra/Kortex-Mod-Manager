#include "stdafx.h"
#include "KArchiveImpl.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxWinUndef.h>
#pragma comment(lib, "7zpp_u.lib")

void KAcrhiveImplNotifier::SetCurrentFile(const TCharType* filePath)
{
	if (filePath && *filePath != _T('\000'))
	{
		m_CurrentFile = filePath;
	}
}
KxArchiveEvent KAcrhiveImplNotifier::CreateEvent(wxEventType type) const
{
	KxArchiveEvent event(type);
	event.Allow();
	event.SetEventObject(m_EvtHandler);
	event.SetCurrent(m_CurrentFile);
	event.SetMajorProcessed(m_LastMajor);
	event.SetMajorTotal(m_TotalSize);
	event.SetMinorProcessed(-2);
	event.SetMinorTotal(-2);

	return event;
}

KAcrhiveImplNotifier::KAcrhiveImplNotifier(wxEvtHandler* eventHandler)
	:m_EvtHandler(eventHandler)
{
}

bool KAcrhiveImplNotifier::Stop()
{
	return m_StopNeeded;
}
void KAcrhiveImplNotifier::OnStartWithTotal(const TCharType* filePath, int64_t totalBytes)
{
	SetCurrentFile(filePath);
	m_TotalSize = totalBytes;

	KxArchiveEvent event = CreateEvent();
	event.SetCurrent(filePath);
	m_EvtHandler->ProcessEvent(event);
}
void KAcrhiveImplNotifier::OnMajorProgress(const TCharType* filePath, int64_t bytesCompleted)
{
	SetCurrentFile(filePath);
	m_LastMajor = bytesCompleted;

	KxArchiveEvent event = CreateEvent();
	m_EvtHandler->ProcessEvent(event);
	m_StopNeeded = !event.IsAllowed();
}
void KAcrhiveImplNotifier::OnMinorProgress(const TCharType* filePath, int64_t bytesCompleted, int64_t totalBytes)
{
	SetCurrentFile(filePath);

	KxArchiveEvent event = CreateEvent();
	event.SetMinorProcessed(bytesCompleted);
	event.SetMinorTotal(totalBytes);
	m_EvtHandler->ProcessEvent(event);

	m_StopNeeded = !event.IsAllowed();
}
void KAcrhiveImplNotifier::OnDone(const TCharType* filePath)
{
	SetCurrentFile(filePath);

	KxArchiveEvent event = CreateEvent(KxEVT_ARCHIVE_DONE);
	event.SetMinorProcessed(m_TotalSize);
	m_EvtHandler->ProcessEvent(event);
}

//////////////////////////////////////////////////////////////////////////
SevenZip::SevenZipLibrary KArchiveImpl::ms_Library;

wxString KArchiveImpl::GetLibraryPath()
{
	wxString path = KApp::Get().GetDataFolder();
	#if defined _WIN64
	path += "\\7z x64.dll";
	#else
	path += "\\7z.dll";
	#endif
	return path;
}
bool KArchiveImpl::Init()
{
	ms_Library.Load(GetLibraryPath().wc_str());
	return IsLibraryLoaded();
}
bool KArchiveImpl::UnInit()
{
	if (IsLibraryLoaded())
	{
		ms_Library.Free();
		return true;
	}
	return false;
}

KArchiveFormat KArchiveImpl::ConvertFormat(FormatEnum type) const
{
	switch (type)
	{
		case FormatEnum::SevenZip:
		{
			return KARC_FORMAT_7Z;
		}
		case FormatEnum::Zip:
		{
			return KARC_FORMAT_ZIP;
		}
		case FormatEnum::Rar:
		{
			return KARC_FORMAT_RAR;
		}
		case FormatEnum::Rar5:
		{
			return KARC_FORMAT_RAR5;
		}
		case FormatEnum::GZip:
		{
			return KARC_FORMAT_GZIP;
		}
		case FormatEnum::BZip2:
		{
			return KARC_FORMAT_BZIP2;
		}
		case FormatEnum::Tar:
		{
			return KARC_FORMAT_TAR;
		}
		case FormatEnum::Iso:
		{
			return KARC_FORMAT_ISO;
		}
		case FormatEnum::Cab:
		{
			return KARC_FORMAT_CAB;
		}
		case FormatEnum::Lzma:
		{
			return KARC_FORMAT_LZMA;
		}
		case FormatEnum::Lzma86:
		{
			return KARC_FORMAT_LZMA86;
		}
	};
	return KARC_FORMAT_UNKNOWN;
}
KArchiveImpl::FormatEnum KArchiveImpl::ConvertFormat(KArchiveFormat type) const
{
	switch (type)
	{
		case KARC_FORMAT_7Z:
		{
			return FormatEnum::Unknown;
		}
		case KARC_FORMAT_ZIP:
		{
			return FormatEnum::Zip;
		}
		case KARC_FORMAT_RAR:
		{
			return FormatEnum::Rar;
		}
		case KARC_FORMAT_RAR5:
		{
			return FormatEnum::Rar5;
		}
		case KARC_FORMAT_GZIP:
		{
			return FormatEnum::GZip;
		}
		case KARC_FORMAT_BZIP2:
		{
			return FormatEnum::BZip2;
		}
		case KARC_FORMAT_TAR:
		{
			return FormatEnum::Tar;
		}
		case KARC_FORMAT_ISO:
		{
			return FormatEnum::Iso;
		}
		case KARC_FORMAT_CAB:
		{
			return FormatEnum::Cab;
		}
		case KARC_FORMAT_LZMA:
		{
			return FormatEnum::Lzma;
		}
		case KARC_FORMAT_LZMA86:
		{
			return FormatEnum::Lzma86;
		}
	};
	return FormatEnum::Unknown;
}
void KArchiveImpl::InvalidateCache()
{
	m_OriginalSize = -1;
}
KArchiveImpl::TStringVector KArchiveImpl::RepackWXStringVector(const KxStringVector& array) const
{
	return KxUtility::RepackVector<wxString, SevenZip::TString>(array, [](const wxString& value)
	{
		return value.wc_str();
	});
}

size_t KArchiveImpl::FindFileAt(size_t index, const wxString& pattern, bool fileNameOnly)
{
	const wxString patternL = pattern.Lower();

	const auto& names = m_Archive.GetItemsNames();
	for (size_t i = index; i < names.size(); i++)
	{
		wxString name = wxString(names[i]);
		if (fileNameOnly)
		{
			name = name.AfterLast('\\').MakeLower();
		}
		else
		{
			name.MakeLower();
		}
		if (name.Matches(patternL))
		{
			return i;
		}
	}
	return (size_t)-1;
}
size_t KArchiveImpl::FindFileAtIn(size_t index, const wxString& pattern, const wxString& folder)
{
	const wxString patternL = pattern.Lower();
	const wxString folderL = folder.Lower();

	const auto& names = m_Archive.GetItemsNames();
	for (size_t i = index; i < names.size(); i++)
	{
		wxString name;
		wxString folderPath = wxString(names[i]).BeforeLast('\\', &name).MakeLower();
		name.MakeLower();

		if (folderPath.Matches(folderL) && name.Matches(patternL))
		{
			return i;
		}
	}
	return (size_t)-1;
}

KArchiveImpl::KArchiveImpl(wxEvtHandler* eventHandler, const wxString& filePath)
	:m_EvtHandler(eventHandler),
	m_Notifier(eventHandler),
	m_Archive(ms_Library, filePath.wc_str(), &m_Notifier),
	m_ArchiveFilePath(filePath)
{
	m_Archive.ReadInArchiveMetadata();
}
KArchiveImpl::~KArchiveImpl()
{
}

wxString KArchiveImpl::GetItemName(size_t index)
{
	return index < GetItemCount() ? m_Archive.GetItemsNames()[index] : wxEmptyString;
}
int64_t KArchiveImpl::GetOriginalSize()
{
	if (m_OriginalSize == -1)
	{
		m_OriginalSize = 0;

		const std::vector<size_t> sizes = m_Archive.GetOrigSizes();
		for (size_t nSize: sizes)
		{
			m_OriginalSize += nSize;
		}
	}
	return m_OriginalSize;
}
int64_t KArchiveImpl::GetCompressedSize()
{
	return KxFile(m_ArchiveFilePath).GetFileSize();
}
float KArchiveImpl::GetRatio()
{
	int64_t original = GetOriginalSize();
	if (original == 0)
	{
		return 1;
	}
	else if (original != -1)
	{
		int64_t compressed = GetCompressedSize();
		if (compressed != -1)
		{
			return (double)compressed / (double)original;
		}
	}
	return -1.0f;
}

size_t KArchiveImpl::FindFirstFileIn(const wxString& folder, const wxString& pattern)
{
	return FindFileAtIn(0, pattern, folder);
}
size_t KArchiveImpl::FindNextFileIn(size_t index, const wxString& folder, const wxString& pattern)
{
	return FindFileAtIn(index + 1, pattern, folder);
}
size_t KArchiveImpl::FindFirstArcFile(const wxString& pattern, bool fileNameOnly)
{
	return FindFileAt(0, pattern, fileNameOnly);
}
size_t KArchiveImpl::FindNextArcFile(size_t index, const wxString& pattern, bool fileNameOnly)
{
	return FindFileAt(index + 1, pattern, fileNameOnly);
}

bool KArchiveImpl::ExtractAll(const wxString& folderPath)
{
	KAcrhiveImplNotifier notifier(m_EvtHandler);
	return m_Archive.ExtractArchive(folderPath.wc_str(), &notifier);
}
bool KArchiveImpl::Extract(const wxString& folderPath, const KxUInt32Vector& indexes)
{
	KAcrhiveImplNotifier notifier(m_EvtHandler);
	return m_Archive.ExtractArchive(indexes, folderPath.wc_str(), &notifier);
}
bool KArchiveImpl::Extract(const KxUInt32Vector& indexes, const KxStringVector& tFinalPaths)
{
	TStringVector newFinalPaths = RepackWXStringVector(tFinalPaths);

	KAcrhiveImplNotifier notifier(m_EvtHandler);
	return m_Archive.ExtractArchive(indexes, newFinalPaths, &notifier);
}
KAcrhiveBuffer KArchiveImpl::Extract(uint32_t index)
{
	KAcrhiveBuffer buffer;
	if (index < GetItemCount())
	{
		KAcrhiveImplNotifier notifier(m_EvtHandler);
		m_Archive.ExtractToMemory(index, buffer, &notifier);
	}
	return buffer;
}
KAcrhiveBufferMap KArchiveImpl::Extract(const KxUInt32Vector& indexes)
{
	KAcrhiveBufferMap bufferMap;
	KAcrhiveImplNotifier notifier(m_EvtHandler);
	m_Archive.ExtractToMemory(indexes, bufferMap, &notifier);
	return bufferMap;
}

bool KArchiveImpl::CompressFiles(const wxString& directory, const wxString& sSearchFilter, bool recursive)
{
	KAcrhiveImplNotifier notifier(m_EvtHandler);
	return m_Archive.CompressFiles(directory.wc_str(), sSearchFilter.wc_str(), recursive, &notifier);
}
bool KArchiveImpl::CompressAllFiles(const wxString& directory, bool recursive)
{
	KAcrhiveImplNotifier notifier(m_EvtHandler);
	return m_Archive.CompressAllFiles(directory.wc_str(), recursive, &notifier);
}
bool KArchiveImpl::CompressDirectory(const wxString& directory, bool recursive )
{
	KAcrhiveImplNotifier notifier(m_EvtHandler);
	return m_Archive.CompressDirectory(directory.wc_str(), recursive, &notifier);
}
bool KArchiveImpl::CompressSpecifiedFiles(const KxStringVector& sourceFiles, const KxStringVector& archivePaths)
{
	TStringVector newSourceFiles = RepackWXStringVector(sourceFiles);
	TStringVector newArchivePaths = RepackWXStringVector(archivePaths);

	KAcrhiveImplNotifier notifier(m_EvtHandler);
	return m_Archive.CompressSpecifiedFiles(newSourceFiles, newArchivePaths, &notifier);
}
bool KArchiveImpl::CompressFile(const wxString& filePath)
{
	KAcrhiveImplNotifier notifier(m_EvtHandler);
	return m_Archive.CompressFile(filePath.wc_str(), &notifier);
}
bool KArchiveImpl::CompressFile(const wxString& filePath, const wxString& archivePath)
{
	KAcrhiveImplNotifier notifier(m_EvtHandler);
	return m_Archive.CompressFile(filePath.wc_str(), archivePath.wc_str(), &notifier);
}
