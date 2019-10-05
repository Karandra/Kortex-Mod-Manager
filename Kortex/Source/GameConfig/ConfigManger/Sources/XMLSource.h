#pragma once
#include "stdafx.h"
#include "XMLRefSource.h"

namespace Kortex::GameConfig
{
	class XMLSource: public KxRTTI::ExtendInterface<XMLSource, XMLRefSource>
	{
		private:
			using Base = KxRTTI::ExtendInterface<XMLSource, XMLRefSource>;

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
