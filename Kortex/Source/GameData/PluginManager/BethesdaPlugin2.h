#pragma once
#include "stdafx.h"
#include "BethesdaPlugin.h"

namespace Kortex::PluginManager
{
	class BethesdaPlugin2: public BethesdaPlugin
	{
		protected:
			void OnRead(IPluginReader& reader) override;

		public:
			BethesdaPlugin2() = default;
			BethesdaPlugin2(const wxString& fullPath)
				:BethesdaPlugin(fullPath)
			{
			}
	};
}
