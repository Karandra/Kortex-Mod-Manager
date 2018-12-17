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
			auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Index"), ColumnID::Index, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH, defaultFlags);
			info.GetColumn()->SortAscending();
		}
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Type"), ColumnID::Type, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH, defaultFlags);
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
			case ColumnID::Index:
			{
				value = Kortex::IPluginManager::GetInstance()->FormatPriority(plugin);
				break;
			}
			case ColumnID::Type:
			{
				value = Kortex::IPluginManager::GetInstance()->GetPluginTypeName(plugin);
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
			case ColumnID::Index:
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
	bool BethesdaDisplayModel::Compare(const IGamePlugin& plugin1, const IGamePlugin& plugin2, const KxDataViewColumn* column) const
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				return KxComparator::IsLess(plugin1.GetName(), plugin1.GetName());
			}
			default:
			case ColumnID::Index:
			{
				return plugin1.GetOrderIndex() < plugin2.GetOrderIndex();
			}
			case ColumnID::Type:
			{
				const Kortex::IPluginManager* manager = Kortex::IPluginManager::GetInstance();
				return KxComparator::IsLess(manager->GetPluginTypeName(plugin1), manager->GetPluginTypeName(plugin2));
				break;
			}
			case ColumnID::PartOf:
			{
				return KxComparator::IsLess(GetPartOfName(plugin1), GetPartOfName(plugin2));
			}
			case ColumnID::Author:
			{
				return KxComparator::IsLess(GetPluginAuthor(plugin1), GetPluginAuthor(plugin2));
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
		return wxNullString;
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
