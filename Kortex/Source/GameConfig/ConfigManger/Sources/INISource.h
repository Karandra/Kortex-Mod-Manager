#pragma once
#include "stdafx.h"
#include "INIRefSource.h"

namespace Kortex::GameConfig
{
	class INISource: public RTTI::IExtendInterface<INISource, INIRefSource>
	{
		private:
			using Base = RTTI::IExtendInterface<INISource, INIRefSource>;

		private:
			KxINI m_INI;

		public:
			INISource(const wxString& iniText = {})
				:Base(m_INI), m_INI(iniText)
			{
			}
			INISource(wxInputStream& stream)
				:Base(m_INI), m_INI(stream)
			{
			}
	};
}
