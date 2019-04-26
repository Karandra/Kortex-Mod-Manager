#include "stdafx.h"
#include "Bethesda2DisplayModel.h"
#include "BethesdaPluginManager2.h"

namespace Kortex::PluginManager
{
	void Bethesda2DisplayModel::GetValue(wxAny& value, const IGamePlugin& plugin, const KxDataViewColumn* column) const
	{
		switch (column->GetID())
		{
			case BSColumnID::Index:
			{
				if (plugin.IsActive())
				{
					const IBethesdaGamePlugin* bethesdaPlugin = nullptr;
					if (plugin.QueryInterface(bethesdaPlugin) && bethesdaPlugin->IsLight())
					{
						KxFormat formatter(wxS("0x%1:%2"));
						formatter.UpperCase();
						formatter(plugin.GetDisplayOrder(), 2, 16, wxS('0'));
						formatter(m_Manager.CountLightActiveBefore(plugin), 3, 16, wxS('0'));

						value = formatter.ToString();
						return;
					}
				}
				break;
			}
		};
		BethesdaDisplayModel::GetValue(value, plugin, column);
	}
}
