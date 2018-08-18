#pragma once
#include "stdafx.h"

class KTempFolderKeeperScopedFile
{
	private:
		const wxString m_FilePath;

	public:
		KTempFolderKeeperScopedFile(const wxString& filePath)
			:m_FilePath(filePath)
		{
		}
		~KTempFolderKeeperScopedFile();

	public:
		operator const wxString&() const
		{
			return m_FilePath;
		}
};

class KTempFolderKeeper
{
	private:
		static wxString ms_GlobalTemp;

	private:
		static void InitGlobalTemp();
		static wxString DoCreateTempFile(const wxString& folder);
		static wxString DoCreateTempFile(const wxString& folder, const wxString& suffix);

	public:
		static const wxString& GetGlobalTemp();
		static wxString CreateGlobalTempFile(const wxString& suffix = wxEmptyString);
		static KTempFolderKeeperScopedFile CreateScopedGlobalTempFile(const wxString& suffix = wxEmptyString);

	//////////////////////////////////////////////////////////////////////////

	private:
		const wxString m_TempFolder;

	private:
		wxString InitTempFolder() const;

	public:
		KTempFolderKeeper();
		virtual ~KTempFolderKeeper();

	public:
		const wxString& GetTempFolder() const
		{
			return m_TempFolder;
		}
		wxString CreateTempFile(const wxString& suffix = wxEmptyString) const;
		KTempFolderKeeperScopedFile CreateScopedTempFile(const wxString& suffix = wxEmptyString);
};
