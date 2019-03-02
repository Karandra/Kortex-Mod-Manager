#include "stdafx.h"
#include "GetAvailableTranslations.h"
#include <Kortex/Application.hpp>

namespace Kortex::GameConfig::SamplingFunction
{
	void GetAvailableTranslations::OnCall(const ItemValue::Vector& arguments)
	{
		for (const auto&[fullName, path]: IApplication::GetInstance()->GetAvailableTranslations())
		{
			wxString component;
			wxString locale = fullName.BeforeFirst(wxS('.'), &component);
			if (component == wxS("Application"))
			{
				m_Values.emplace_back(locale).SetLabel(KxTranslation::GetLanguageFullName(locale));
			}
		}
	}
}
