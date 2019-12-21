#pragma once
#include "stdafx.h"

namespace Kortex
{
	class IGamePlugin;
}

namespace Kortex::PluginManager
{
	class IPluginReader: public KxRTTI::Interface<IPluginReader>
	{
		KxDecalreIID(IPluginReader, {0x83328e94, 0x58c0, 0x48a4, {0xbc, 0x71, 0x8b, 0xcd, 0xb2, 0x8a, 0xdc, 0x4e}});

		friend class IGamePlugin;

		protected:
			virtual void OnRead(const IGamePlugin& plugin) = 0;

		public:
			virtual bool IsOK() const = 0;
	};
}
