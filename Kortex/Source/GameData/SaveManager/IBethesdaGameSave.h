#pragma once
#include "stdafx.h"
#include "BaseGameSave.h"

namespace Kortex::SaveManager
{
	class IBethesdaGameSave: public RTTI::IExtendInterface<IBethesdaGameSave, BaseGameSave>
	{
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
