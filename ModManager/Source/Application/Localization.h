#pragma once
#include "Framework.hpp"
#include <kxf/Localization/ILocalizationPackage.h>

namespace Kortex
{
	KORTEX_API kxf::String Localize(const kxf::ResourceID& id);
	KORTEX_API kxf::LocalizationItem LocalizeItem(const kxf::ResourceID& id);

	template<class... Args>
	kxf::String Localize(const kxf::ResourceID& id, Args&&... arg)
	{
		return kxf::Format(Localize(id), std::forward<Args>(arg)...);
	}

	template<class... Args>
	kxf::LocalizationItem LocalizeItem(const kxf::ResourceID& id, Args&&... arg)
	{
		return kxf::Format(LocalizeItem(id), std::forward<Args>(arg)...);
	}
}
