#pragma once
#include "stdafx.h"

namespace KPluginEntryTypesInterface
{
	class Bethesda
	{
		public:
			virtual ~Bethesda() = default;

			virtual bool IsMaster() const = 0;
			virtual bool IsLight() const = 0;
			bool IsNormal() const
			{
				return !IsMaster() && !IsLight();
			}
	};
}

