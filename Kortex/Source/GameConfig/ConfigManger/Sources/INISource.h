#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/ISource.h"
#include "GameConfig/ConfigManger/IFileSource.h"
#include <KxFramework/KxINI.h>

namespace Kortex::GameConfig
{
	class INISource: public RTTI::IExtendInterface<INISource, ISource, IFileSource>
	{
		private:
			KxINI m_INI;
			wxString m_FilePath;
			wxString m_FileName;
			bool m_IsOpened = false;

		public:
			INISource(const wxString& filePath)
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
			SourceFormatValue GetFormat() const override
			{
				return SourceFormat::INI;
			}
			wxString GetPathDescription() const override
			{
				// No file name resolution is required, we can return it unchanged.
				return m_FileName;
			}

			bool IsOpened() const override
			{
				return m_IsOpened;
			}
			bool Open() override;
			bool Save() override;
			void Close() override;

			bool WriteValue(const Item& item, const ItemValue& value) override;
			bool ReadValue(Item& item, ItemValue& value) const override;
			void LoadUnknownItems(ItemGroup& group) override;
	};
}
