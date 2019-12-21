#pragma once
#include "stdafx.h"
#include "BaseGameSave.h"

namespace Kortex::SaveManager
{
	class IBethesdaGameSave: public KxRTTI::ExtendInterface<IBethesdaGameSave, BaseGameSave>
	{
		KxDecalreIID(IBethesdaGameSave, {0xe1e2da7b, 0x59a3, 0x41e9, {0xa2, 0x46, 0x78, 0x88, 0xf8, 0xc, 0xd5, 0x5d}});

		public:
			using float32_t = float;
			using float64_t = double;

		public:
			virtual KxStringVector GetPlugins() const = 0;
			virtual size_t GetPluginsCount() const = 0;
			virtual bool HasPlugins() const = 0;
			
			virtual uint32_t GetVersion() const = 0;
	};
}
