#include "stdafx.h"
#include "KArchive.h"
#include "KArchiveImpl.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxLibrary.h>

wxString KArchive::GetLibraryPath()
{
	return KArchiveImpl::GetLibraryPath();
}
wxString KArchive::GetLibraryVersion()
{
	return KxLibrary::GetVersionInfoFromFile(GetLibraryPath()).GetString("ProductVersion");
}
bool KArchive::IsLibraryLoaded()
{
	return KArchiveImpl::IsLibraryLoaded();
}
bool KArchive::Init()
{
	return KArchiveImpl::Init();
}
bool KArchive::UnInit()
{
	return KArchiveImpl::UnInit();
}

KArchive::KArchive()
{
}
KArchive::KArchive(const wxString& filePath)
{
	Open(filePath);
}
KArchive::~KArchive()
{
	Close();
}

bool KArchive::IsOpened() const
{
	return m_Impl != NULL;
}
bool KArchive::Open(const wxString& filePath)
{
	if (!IsOpened())
	{
		m_Impl = new KArchiveImpl(this, filePath);
		return IsOpened();
	}
	return false;
}
void KArchive::Close()
{
	delete m_Impl;
	m_Impl = NULL;
}

const wxString& KArchive::GetFilePath() const
{
	return m_Impl->GetFilePath();
}
wxString KArchive::GetItemName(size_t index) const
{
	return m_Impl->GetItemName(index);
}
size_t KArchive::GetItemCount() const
{
	return m_Impl->GetItemCount();
}
int64_t KArchive::GetOriginalSize() const
{
	return m_Impl->GetOriginalSize();
}
int64_t KArchive::GetCompressedSize() const
{
	return m_Impl->GetCompressedSize();
}
float KArchive::GetRatio() const
{
	return m_Impl->GetRatio();
}

KArchiveFormat KArchive::GetProperty_CompressionFormat() const
{
	return m_Impl->GetProperty_CompressionFormat();
}
void KArchive::SetProperty_CompressionFormat(KArchiveFormat value)
{
	m_Impl->SetProperty_CompressionFormat(value);
}

int KArchive::GetProperty_CompressionLevel() const
{
	return m_Impl->GetProperty_CompressionLevel();
}
void KArchive::SetProperty_CompressionLevel(int value)
{
	m_Impl->SetProperty_CompressionLevel(value);
}

int KArchive::GetProperty_DictionarySize() const
{
	return m_Impl->GetProperty_DictionarySize();
}
void KArchive::SetProperty_DictionarySize(int value)
{
	m_Impl->SetProperty_DictionarySize(value);
}

KArchiveMethod KArchive::GetProperty_CompressionMethod() const
{
	return m_Impl->GetProperty_CompressionMethod();
}
void KArchive::SetProperty_CompressionMethod(KArchiveMethod method)
{
	m_Impl->SetProperty_CompressionMethod(method);
}

bool KArchive::GetProperty_Solid() const
{
	return m_Impl->GetProperty_Solid();
}
void KArchive::SetProperty_Solid(bool bSolid)
{
	m_Impl->SetProperty_Solid(bSolid);
}

bool KArchive::GetProperty_MultiThreaded() const
{
	return m_Impl->GetProperty_MultiThreaded();
}
void KArchive::SetProperty_MultiThreaded(bool set)
{
	m_Impl->SetProperty_MultiThreaded(set);
}

size_t KArchive::FindFirstFileIn(const wxString& folder, const wxString& pattern) const
{
	return m_Impl->FindFirstFileIn(folder, pattern);
}
size_t KArchive::FindNextFileIn(size_t index, const wxString& folder, const wxString& pattern) const
{
	return m_Impl->FindNextFileIn(index, folder, pattern);
}
size_t KArchive::FindFirstFile(const wxString& pattern, bool fileNameOnly) const
{
	return m_Impl->FindFirstArcFile(pattern, fileNameOnly);
}
size_t KArchive::FindNextFile(size_t index, const wxString& pattern, bool fileNameOnly) const
{
	return m_Impl->FindNextArcFile(index, pattern, fileNameOnly);
}

bool KArchive::ExtractAll(const wxString& folderPath) const
{
	return m_Impl->ExtractAll(folderPath);
}
bool KArchive::Extract(const wxString& folderPath, const KxUInt32Vector& indexes) const
{
	return m_Impl->Extract(folderPath, indexes);
}
bool KArchive::Extract(const KxUInt32Vector& indexes, const KxStringVector& finalPaths) const
{
	return m_Impl->Extract(indexes, finalPaths);
}
KAcrhiveBuffer KArchive::Extract(uint32_t index) const
{
	return m_Impl->Extract(index);
}
KAcrhiveBufferMap KArchive::Extract(const KxUInt32Vector& indexes) const
{
	return m_Impl->Extract(indexes);
}

bool KArchive::CompressFiles(const wxString& directory, const wxString& searchFilter, bool recursive)
{
	return m_Impl->CompressFiles(directory, searchFilter, recursive);
}
bool KArchive::CompressAllFiles(const wxString& directory, bool recursive)
{
	return m_Impl->CompressAllFiles(directory, recursive);
}
bool KArchive::CompressDirectory(const wxString& directory, bool recursive)
{
	return m_Impl->CompressDirectory(directory, recursive);
}
bool KArchive::CompressSpecifiedFiles(const KxStringVector& sourceFiles, const KxStringVector& archivePaths)
{
	return m_Impl->CompressSpecifiedFiles(sourceFiles, archivePaths);
}
bool KArchive::CompressFile(const wxString& filePath)
{
	return m_Impl->CompressFile(filePath);
}
bool KArchive::CompressFile(const wxString& filePath, const wxString& archivePath)
{
	return m_Impl->CompressFile(filePath, archivePath);
}
