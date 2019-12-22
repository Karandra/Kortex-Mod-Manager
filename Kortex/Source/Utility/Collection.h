#pragma once
#include "stdafx.h"

namespace Kortex::Utility::Collection
{
	template<class TCollection, class TFunc>
	void Enumerate(TCollection&& items, TFunc&& func)
	{
		for (auto&& value: items)
		{
			using TResult = decltype(func(value));
			if constexpr(std::is_same_v<TResult, bool>)
			{
				if (!func(value))
				{
					return;
				}
			}
			else
			{
				func(value);
			}	
		}
	}
}
