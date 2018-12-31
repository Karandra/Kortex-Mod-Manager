#pragma once
#include "stdafx.h"

namespace Kortex::Utility::Collection
{
	template<class TCollection, class TFunctor> void Enumerate(TCollection&& items, TFunctor&& functor)
	{
		for (auto&& value: items)
		{
			using TResult = decltype(functor(value));
			if constexpr(std::is_same_v<TResult, bool>)
			{
				if (!functor(value))
				{
					return;
				}
			}
			else
			{
				functor(value);
			}	
		}
	}
}
