#include "pch.hpp"
#include "Localization.h"
#include "IApplication.h"

namespace
{
	kxf::LocalizationItem DoGetItem(const kxf::ResourceID& id)
	{
		return Kortex::IApplication::GetInstance().GetLocalizationPackage().GetItem(id);
	}
}

namespace Kortex
{
	kxf::String Localize(const kxf::ResourceID& id)
	{
		return DoGetItem(id).GetString();
	}
}
