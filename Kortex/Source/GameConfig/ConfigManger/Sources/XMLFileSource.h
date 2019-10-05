#pragma once
#include "stdafx.h"
#include "XMLSource.h"
#include "GameConfig/ConfigManger/IFileSource.h"

namespace Kortex::GameConfig
{
	class XMLFileSource: public KxRTTI::ExtendInterface<XMLFileSource, XMLSource, IFileSource>
	{
		private:
			wxString m_FilePath;
			wxString m_FileName;

		public:
			XMLFileSource(const wxString& filePath)
				:m_FilePath(filePath), m_FileName(filePath.AfterLast(wxS('\\')))
			{
			}

		public:
			// IFSSource
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
				return m_FileName;
			}

			bool Open() override;
			bool Save() override;
			void Close() override;
	};
}
