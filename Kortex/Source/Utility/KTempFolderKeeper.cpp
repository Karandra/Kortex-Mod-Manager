#include "stdafx.h"
#include "KTempFolderKeeper.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxMath.h>

KTempFolderKeeperScopedFile::~KTempFolderKeeperScopedFile()
{
	KxFile(m_FilePath).RemoveFile();
}
//////////////////////////////////////////////////////////////////////////

wxString KTempFolderKeeper::ms_GlobalTemp;

void KTempFolderKeeper::InitGlobalTemp()
{
	if (ms_GlobalTemp.IsEmpty())
	{
		ms_GlobalTemp = wxFileName::GetTempDir() + '\\' + Kortex::IApplication::GetInstance()->GetName() + '\\';
		KxFile(ms_GlobalTemp).CreateFolder();
	}
}
const wxString& KTempFolderKeeper::GetGlobalTemp()
{
	InitGlobalTemp();
	return ms_GlobalTemp;
}
wxString KTempFolderKeeper::DoCreateTempFile(const wxString& folder)
{
	return wxFileName::CreateTempFileName(folder);
}
wxString KTempFolderKeeper::DoCreateTempFile(const wxString& folder, const wxString& suffix)
{
	if (suffix.IsEmpty())
	{
		return DoCreateTempFile(folder);
	}
	else
	{
		wxString tempFile = DoCreateTempFile(folder);
		wxString newTempFile = tempFile + suffix;
		KxFile(tempFile).Rename(newTempFile, true);
		return newTempFile;
	}
}

wxString KTempFolderKeeper::CreateGlobalTempFile(const wxString& suffix)
{
	return DoCreateTempFile(ms_GlobalTemp, suffix);
}
KTempFolderKeeperScopedFile KTempFolderKeeper::CreateScopedGlobalTempFile(const wxString& suffix)
{
	return KTempFolderKeeperScopedFile(DoCreateTempFile(ms_GlobalTemp, suffix));
}

//////////////////////////////////////////////////////////////////////////
wxString KTempFolderKeeper::InitTempFolder() const
{
	const void* pInstance = this + KxMath::RandomInt(std::numeric_limits<size_t>::max());

	// Global temp folder already has path separator at its end.
	return wxString::Format("%s0x%p\\", GetGlobalTemp(), pInstance);
}

KTempFolderKeeper::KTempFolderKeeper()
	:m_TempFolder(InitTempFolder())
{
	KxFile(m_TempFolder).CreateFolder();
}
KTempFolderKeeper::~KTempFolderKeeper()
{
	KxFile(m_TempFolder).RemoveFolderTree(true);
}

wxString KTempFolderKeeper::CreateTempFile(const wxString& suffix) const
{
	return DoCreateTempFile(m_TempFolder, suffix);
}
KTempFolderKeeperScopedFile KTempFolderKeeper::CreateScopedTempFile(const wxString& suffix)
{
	return KTempFolderKeeperScopedFile(CreateTempFile(suffix));
}
