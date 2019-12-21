#pragma once
#include "stdafx.h"
#include "GameData/IGamePlugin.h"

namespace Kortex
{
	class IBethesdaGamePlugin: public KxRTTI::Interface<IBethesdaGamePlugin>
	{
		KxDecalreIID(IBethesdaGamePlugin, {0x5b28e8bc, 0xf6df, 0x4864, {0xae, 0x82, 0xc1, 0x99, 0x12, 0x2a, 0xdb, 0x94}});

		public:
			virtual bool IsLocalized() const = 0;
			virtual bool IsMaster() const = 0;
			virtual bool IsLight() const = 0;
			bool IsNormal() const
			{
				return !IsMaster() && !IsLight();
			}

			virtual uint32_t GetFormVersion() const = 0;
			bool IsForm43() const
			{
				return GetFormVersion() == 43;
			}
			bool IsForm44() const
			{
				return GetFormVersion() == 44;
			}

			virtual KxStringVector GetRequiredPlugins() const = 0;
			virtual wxString GetAuthor() const = 0;
			virtual wxString GetDescription() const = 0;
	};
}
