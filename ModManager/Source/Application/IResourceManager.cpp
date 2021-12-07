#include "pch.hpp"
#include "IResourceManager.h"

namespace Kortex
{
	kxf::ResourceID IResourceManager::MakeResourceID(const kxf::String& path, const kxf::String& scheme)
	{
		if (path.IsEmpty())
		{
			return {};
		}

		if (!scheme.IsEmpty())
		{
			return kxf::Format(kxS("{}://{}"), scheme, path).MakeLower();
		}
		else
		{
			return kxf::Format(kxS("any://{}://{}"), scheme, path).MakeLower();
		}
	}
	kxf::ResourceID IResourceManager::MakeResourceIDWithCategory(const kxf::String& category, const kxf::String& path, const kxf::String& scheme)
	{
		if (path.IsEmpty())
		{
			return {};
		}

		if (category.IsEmpty())
		{
			return MakeResourceID(path, scheme);
		}
		else
		{
			if (!scheme.IsEmpty())
			{
				return kxf::Format(kxS("{}://{}/{}"), scheme, category, path).MakeLower();
			}
			return kxf::Format(kxS("any://{}/{}"), category, path).MakeLower();
		}
	}
}
