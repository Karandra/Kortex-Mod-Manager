#pragma once
#include "stdafx.h"

namespace Kortex
{
	class IGamePlugin;
	class IPluginManager;
}

namespace Kortex::PluginManager
{
	class IPluginReader: public RTTI::IInterface<IPluginReader>
	{
		friend class IPluginManager;

		protected:
			virtual void OnRead(const IGamePlugin& plugin) = 0;

		public:
			virtual bool IsOK() const = 0;
	};
}
