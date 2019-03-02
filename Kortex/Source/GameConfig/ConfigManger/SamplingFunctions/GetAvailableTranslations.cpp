#include "stdafx.h"
#include "GetAvailableTranslations.h"
#include <Kortex/Application.hpp>

namespace Kortex::GameConfig::SamplingFunction
{
	void GetAvailableTranslations::OnCall(const ItemValue::Vector& arguments)
	{
		for (const auto&[locale, path]: IApplication::GetInstance()->GetAvailableTranslations())
		{
			m_Values.emplace_back(locale).SetLabel(KxTranslation::GetLanguageFullName(locale));
		}
	}
}
