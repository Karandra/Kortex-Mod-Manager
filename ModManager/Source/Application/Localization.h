#pragma once
#include "Framework.hpp"

namespace Kortex
{
	kxf::String Localize(const kxf::ResourceID& id);

	template<class... Args>
	kxf::String Localize(const kxf::ResourceID& id, Args&&... arg)
	{
		return kxf::String::Format(Localize(id), std::forward<Args>(arg)...);
	}
}
