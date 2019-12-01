#include "stdafx.h"
#include "TempFolderKeeper.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxMath.h>

namespace
{
	static wxString g_GlobalTemp;
}

namespace Kortex::Utility
{
	ScopedTempFile::~ScopedTempFile()
	{
		KxFile(m_FilePath).RemoveFile();
	}
}

namespace Kortex::Utility
{
	void TempFolderKeeper::InitGlobalTemp()
	{
		if (g_GlobalTemp.IsEmpty())
		{
			g_GlobalTemp = wxFileName::GetTempDir() + wxS('\\') + IApplication::GetInstance()->GetName() + '\\';
			KxFile(g_GlobalTemp).CreateFolder();
		}
	}
	const wxString& TempFolderKeeper::GetGlobalTemp()
	{
		InitGlobalTemp();
		return g_GlobalTemp;
	}
	wxString TempFolderKeeper::DoCreateTempFile(const wxString& folder)
	{
		return wxFileName::CreateTempFileName(folder);
	}
	wxString TempFolderKeeper::DoCreateTempFile(const wxString& folder, const wxString& suffix)
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

	wxString TempFolderKeeper::CreateGlobalTempFile(const wxString& suffix)
	{
		return DoCreateTempFile(g_GlobalTemp, suffix);
	}
	ScopedTempFile TempFolderKeeper::CreateScopedGlobalTempFile(const wxString& suffix)
	{
		return ScopedTempFile(DoCreateTempFile(g_GlobalTemp, suffix));
	}

	wxString TempFolderKeeper::InitTempFolder() const
	{
		const void* pInstance = this + KxMath::RandomInt(std::numeric_limits<size_t>::max());

		// Global temp folder already has path separator at its end.
		return wxString::Format("%s0x%p\\", GetGlobalTemp(), pInstance);
	}

	TempFolderKeeper::TempFolderKeeper()
		:m_TempFolder(InitTempFolder())
	{
		KxFile(m_TempFolder).CreateFolder();
	}
	TempFolderKeeper::~TempFolderKeeper()
	{
		KxFile(m_TempFolder).RemoveFolderTree(true);
	}

	wxString TempFolderKeeper::CreateTempFile(const wxString& suffix) const
	{
		return DoCreateTempFile(m_TempFolder, suffix);
	}
	ScopedTempFile TempFolderKeeper::CreateScopedTempFile(const wxString& suffix)
	{
		return CreateTempFile(suffix);
	}
}
