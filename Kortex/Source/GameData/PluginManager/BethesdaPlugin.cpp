#include "stdafx.h"
#include "BethesdaPlugin.h"
#include "IBethesdaPluginReader.h"

namespace Kortex::PluginManager
{
	void BethesdaPlugin::OnRead(IPluginReader& reader)
	{
		IBethesdaPluginReader* bethesdaReader = nullptr;
		if (reader.QueryInterface(bethesdaReader))
		{
			m_Data = std::move(bethesdaReader->GetData());
		}
	}
}
