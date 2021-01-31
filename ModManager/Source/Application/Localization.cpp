#include "pch.hpp"
#include "Localization.h"
#include "IApplication.h"

namespace Kortex
{
	kxf::String Localize(const kxf::ResourceID& id)
	{
		return LocalizeItem(id).GetString();
	}
	kxf::LocalizationItem LocalizeItem(const kxf::ResourceID& id)
	{
		return IApplication::GetInstance().GetLocalizationPackage().GetItem(id);
	}
}
