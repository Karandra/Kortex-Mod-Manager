#pragma once
#include "stdafx.h"

namespace Kortex::Utility
{
	class ScopedTempFile final
	{
		private:
			const wxString m_FilePath;

		public:
			ScopedTempFile(const wxString& filePath)
				:m_FilePath(filePath)
			{
			}
			~ScopedTempFile();

		public:
			operator const wxString&() const
			{
				return m_FilePath;
			}
	};
}

namespace Kortex::Utility
{
	class TempFolderKeeper
	{
		private:
			static void InitGlobalTemp();
			static wxString DoCreateTempFile(const wxString& folder);
			static wxString DoCreateTempFile(const wxString& folder, const wxString& suffix);

		public:
			static const wxString& GetGlobalTemp();
			static wxString CreateGlobalTempFile(const wxString& suffix = wxEmptyString);
			static ScopedTempFile CreateScopedGlobalTempFile(const wxString& suffix = wxEmptyString);

		private:
			const wxString m_TempFolder;

		private:
			wxString InitTempFolder() const;

		public:
			TempFolderKeeper();
			virtual ~TempFolderKeeper();

		public:
			wxString GetFolder() const
			{
				return m_TempFolder;
			}
			
			wxString CreateTempFile(const wxString& suffix = wxEmptyString) const;
			ScopedTempFile CreateScopedTempFile(const wxString& suffix = wxEmptyString);
	};
}
