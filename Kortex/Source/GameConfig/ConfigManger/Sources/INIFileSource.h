#pragma once
#include "stdafx.h"
#include "INISource.h"
#include "GameConfig/ConfigManger/IFileSource.h"

namespace Kortex::GameConfig
{
	class INIFileSource: public KxRTTI::ImplementInterface<INIFileSource, INISource, IFileSource>
	{
		private:
			wxString m_FilePath;
			wxString m_FileName;

		public:
			INIFileSource(const wxString& filePath)
				:m_FilePath(filePath), m_FileName(filePath.AfterLast(wxS('\\')))
			{
			}

		public:
			// IFileSource
			wxString GetFileName() const override
			{
				return m_FileName;
			}
			wxString GetFilePath() const override
			{
				return m_FilePath;
			}

			// ISource
			wxString GetPathDescription() const override
			{
				// No file name resolution is required, we can return it unchanged.
				return m_FileName;
			}

			bool Open() override;
			bool Save() override;
			void Close() override;
	};
}
