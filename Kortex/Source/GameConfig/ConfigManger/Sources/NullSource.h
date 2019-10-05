#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/ISource.h"

namespace Kortex::GameConfig
{
	class NullSource: public KxRTTI::ExtendInterface<NullSource, ISource>
	{
		private:
			bool m_IsOpened = false;

		public:
			NullSource() = default;

		public:
			SourceFormatValue GetFormat() const override
			{
				return SourceFormat::None;
			}
			wxString GetPathDescription() const override
			{
				return wxEmptyString;
			}

			bool IsOpened() const override
			{
				return m_IsOpened;
			}
			bool Open() override
			{
				m_IsOpened = true;
				return true;
			}
			bool Save() override
			{
				return true;
			}
			void Close() override
			{
				m_IsOpened = false;
			}

			bool WriteValue(const Item& item, const ItemValue& value) override
			{
				return true;
			}
			bool ReadValue(Item& item, ItemValue& value) const override
			{
				return true;
			}
			void LoadUnknownItems(ItemGroup& group) override
			{
			}
	};
}
