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

		public:
			XMLSource(const wxString& xmlText = {})
				:Base(m_XML), m_XML(xmlText)
			{
			}
			XMLSource(wxInputStream& stream)
				:Base(m_XML), m_XML(stream)
			{
			}
	};
}
