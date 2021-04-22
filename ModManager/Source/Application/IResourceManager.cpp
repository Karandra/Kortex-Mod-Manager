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
			return kxf::String::Format(wxS("%1://%2"), scheme, path).MakeLower();
		}
		else
		{
			return kxf::String::Format(wxS("any://%1://%2"), scheme, path).MakeLower();
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
				return kxf::String::Format(wxS("%1://%2/%3"), scheme, category, path).MakeLower();
			}
			return kxf::String::Format(wxS("any://%1/%2"), category, path).MakeLower();
		}
	}
}
