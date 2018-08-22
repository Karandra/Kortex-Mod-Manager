#pragma once
#include "stdafx.h"

namespace KRTTI
{
	template<class Base> class CastAsIs
	{
		public:
			virtual ~CastAsIs() = default;

		public:
			template<class T> bool As(T*& ptr)
			{
				static_assert(std::is_base_of<Base, T>::value, "T must be derived from 'KPluginEntry'");
				ptr = dynamic_cast<T*>(this);
				return ptr != NULL;
			}
			template<class T> bool As(const T*& ptr) const
			{
				static_assert(std::is_base_of<Base, T>::value, "T must be derived from 'KPluginEntry'");
				ptr = dynamic_cast<const T*>(this);
				return ptr != NULL;
			}

			template<class T> bool Is() const
			{
				const T*& ptr = NULL;
				return As<T>(ptr);
			}
	};
}
