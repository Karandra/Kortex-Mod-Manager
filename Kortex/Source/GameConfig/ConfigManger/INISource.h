#pragma once
#include "stdafx.h"
#include "ISource.h"
#include <KxFramework/KxINI.h>

namespace Kortex::GameConfig
{
	class INISource: public ISource
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
			SourceTypeValue GetType() const override
			{
				return SourceType::FSPath;
			}
			SourceFormatValue GetFormat() const override
			{
				return SourceFormat::INI;
			}
			wxString GetPathDescription() const override
			{
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
