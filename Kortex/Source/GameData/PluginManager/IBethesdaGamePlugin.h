#pragma once
#include "stdafx.h"
#include "GameData/IGamePlugin.h"

namespace Kortex
{
	class IBethesdaGamePlugin: public KxRTTI::Interface<IBethesdaGamePlugin>
	{
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
