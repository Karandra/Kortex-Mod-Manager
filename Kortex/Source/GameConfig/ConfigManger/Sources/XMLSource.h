#pragma once
#include "stdafx.h"
#include "XMLRefSource.h"

namespace Kortex::GameConfig
{
	class XMLSource: public XMLRefSource
	{
		private:
			KxXMLDocument m_XML;

		public:
			XMLSource(const wxString& xmlText = {})
				:XMLRefSource(m_XML), m_XML(xmlText)
			{
			}
			XMLSource(wxInputStream& stream)
				:XMLRefSource(m_XML), m_XML(stream)
			{
			}
	};
}
