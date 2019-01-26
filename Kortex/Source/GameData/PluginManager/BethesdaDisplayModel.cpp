#include "stdafx.h"
#include <Kortex/PluginManager.hpp>
#include <Kortex/ModManager.hpp>
#include <KxFramework/KxComparator.h>

namespace Kortex::PluginManager
{
	void BethesdaDisplayModel::OnInitControl()
	{
		KxDataViewColumnFlags defaultFlags = KxDV_COL_DEFAULT_FLAGS|KxDV_COL_SORTABLE;

		GetView()->AppendColumn<KxDataViewBitmapTextToggleRenderer>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_ACTIVATABLE, KxDVC_DEFAULT_WIDTH, defaultFlags);
		{
			auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Priority"), ColumnID::Priority, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH, defaultFlags);
			info.GetColumn()->SortAscending();
		}
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Index"), BSColumnID::Index, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH, defaultFlags);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.PartOf"), ColumnID::PartOf, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH, defaultFlags);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Author"), ColumnID::Author, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH, defaultFlags);
	}

	void BethesdaDisplayModel::GetValue(wxAny& value, const IGamePlugin& plugin, const KxDataViewColumn* column) const
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = KxDataViewBitmapTextToggleValue(plugin.IsActive(), plugin.GetName(), wxNullBitmap, KxDataViewBitmapTextToggleValue::CheckBox);
				break;
			}
			case BSColumnID::Index:
			{
				if (plugin.IsActive())
				{
					value = KxFormat(wxS("0x%1")).UpperCase()(plugin.GetPriority(), 2, 16, wxS('0')).ToString();
				}
				break;
			}
			case ColumnID::Priority:
			{
				value = plugin.GetOrderIndex();
				break;
			}
			case ColumnID::Type:
			{
				value = IPluginManager::GetInstance()->GetPluginTypeName(plugin);
				break;
			}
			case ColumnID::PartOf:
			{
				value = GetPartOfName(plugin);
				break;
			}
			case ColumnID::Author:
			{
				value = GetPluginAuthor(plugin);
				break;
			}
		};
	}
	bool BethesdaDisplayModel::SetValue(const wxAny& value, IGamePlugin& plugin, const KxDataViewColumn* column)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				plugin.SetActive(value.As<bool>());
				
				// If this item state has changed, this makes control query indexes of displayed items again
				GetView()->Refresh();
				return true;
			}
		};
		return false;
	}

	bool BethesdaDisplayModel::IsEnabled(const IGamePlugin& plugin, const KxDataViewColumn* column) const
	{
		if (column->GetID() == ColumnID::Name)
		{
			return plugin.CanToggleActive();
		}
		return true;
	}
	bool BethesdaDisplayModel::IsEditorEnabled(const IGamePlugin& plugin, const KxDataViewColumn* column) const
	{
		return false;
	}

	bool BethesdaDisplayModel::GetAttributes(const IGamePlugin& plugin, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
	{
		switch (column->GetID())
		{
			case ColumnID::Priority:
			case BSColumnID::Index:
			{
				attributes.SetFontFace(wxS("Consolas"));
				return true;
			}
			case ColumnID::PartOf:
			{
				const IGameMod* modEntry = plugin.GetOwningMod();
				if (modEntry)
				{
					if (cellState & KxDATAVIEW_CELL_HIGHLIGHTED && column->IsHotTracked())
					{
						attributes.SetUnderlined(true);
					}

					const StdContentEntry* standardContentEntry = plugin.GetStdContentEntry();
					if (modEntry->QueryInterface<ModManager::IFixedGameMod>() && !standardContentEntry)
					{
						attributes.SetItalic(true);
					}
				}
				break;
			}
		};
		return !attributes.IsDefault();
	}
	bool BethesdaDisplayModel::Compare(const IGamePlugin& pluginLeft, const IGamePlugin& pluginRight, const KxDataViewColumn* column) const
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				return KxComparator::IsLess(pluginLeft.GetName(), pluginRight.GetName());
			}
			default:
			case ColumnID::Priority:
			case BSColumnID::Index:
			{
				return pluginLeft.GetOrderIndex() < pluginRight.GetOrderIndex();
			}
			case ColumnID::Type:
			{
				const IPluginManager* manager = IPluginManager::GetInstance();
				return KxComparator::IsLess(manager->GetPluginTypeName(pluginLeft), manager->GetPluginTypeName(pluginRight));
				break;
			}
			case ColumnID::PartOf:
			{
				return KxComparator::IsLess(GetPartOfName(pluginLeft), GetPartOfName(pluginRight));
			}
			case ColumnID::Author:
			{
				return KxComparator::IsLess(GetPluginAuthor(pluginLeft), GetPluginAuthor(pluginRight));
			}
		};
		return false;
	}

	wxString BethesdaDisplayModel::GetPartOfName(const IGamePlugin& plugin) const
	{
		if (const StdContentEntry* standardContentEntry = plugin.GetStdContentEntry())
		{
			return standardContentEntry->GetName();
		}
		if (const IGameMod* modEntry = plugin.GetOwningMod())
		{
			return modEntry->GetName();
		}
		return KxNullWxString;
	}
	wxString BethesdaDisplayModel::GetPluginAuthor(const IGamePlugin& plugin) const
	{
		const IBethesdaGamePlugin* bethesdaPlugin = nullptr;
		if (plugin.QueryInterface(bethesdaPlugin))
		{
			return bethesdaPlugin->GetAuthor();
		}
		return wxEmptyString;
	}
}
