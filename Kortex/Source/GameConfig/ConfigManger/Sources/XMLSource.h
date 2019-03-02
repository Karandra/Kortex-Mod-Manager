#pragma once
#include "stdafx.h"
#include "XMLRefSource.h"

namespace Kortex::GameConfig
{
	class XMLSource: public RTTI::IExtendInterface<XMLSource, XMLRefSource>
	{
		private:
			using Base = RTTI::IExtendInterface<XMLSource, XMLRefSource>;

		private:
			KxXMLDocument m_XML;
			bool m_IsOpened = false;

		public:
			XMLSource(const wxString& xmlText = {})
				:Base(m_XML), m_XML(xmlText)
			{
			}
			XMLSource(wxInputStream& stream)
				:Base(m_XML), m_XML(stream)
			{
			}

		public:
			// ISource
			SourceFormatValue GetFormat() const override
			{
				return SourceFormat::INI;
			}
			wxString GetPathDescription() const override
			{
				return {};
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
