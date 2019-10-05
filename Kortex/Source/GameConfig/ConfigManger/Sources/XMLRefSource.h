#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/ISource.h"
#include <KxFramework/KxXML.h>

namespace Kortex::GameConfig
{
	class XMLRefSource: public KxRTTI::ExtendInterface<XMLRefSource, ISource>
	{
		private:
			KxXMLDocument& m_XML;
			bool m_IsOpened = false;

		protected:
			const KxXMLDocument& GetXML() const
			{
				return m_XML;
			}
			KxXMLDocument& GetXML()
			{
				return m_XML;
			}

		public:
			XMLRefSource(KxXMLDocument& xml)
				:m_XML(xml)
			{
			}

		public:
			// ISource
			SourceFormatValue GetFormat() const override
			{
				return SourceFormat::XML;
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
