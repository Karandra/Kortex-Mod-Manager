#include "stdafx.h"
#include "KPluginViewModelBethesda.h"
#include "KPluginEntry.h"
#include "KPluginReaderBethesda.h"
#include "KPluginManager.h"
#include <KxFramework/KxComparator.h>
#include "Profile/KPluginManagerConfig.h"

void KPluginViewModelBethesda::OnInitControl()
{
	KxDataViewColumnFlags defaultFlags = KxDV_COL_DEFAULT_FLAGS|KxDV_COL_SORTABLE;

	GetView()->AppendColumn<KxDataViewBitmapTextToggleRenderer>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_ACTIVATABLE, KxDVC_DEFAULT_WIDTH, defaultFlags);
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Index"), ColumnID::Index, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH, defaultFlags);
		info.GetColumn()->SortAscending();
	}
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Type"), ColumnID::Type, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH, defaultFlags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.PartOf"), ColumnID::PartOf, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH, defaultFlags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Author"), ColumnID::Author, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH, defaultFlags);
}

void KPluginViewModelBethesda::GetValue(wxAny& value, const KPluginEntry& pluginEntry, const KxDataViewColumn* column) const
{
	switch (column->GetID())
	{
		case ColumnID::Name:
		{
			value = KxDataViewBitmapTextToggleValue(pluginEntry.IsEnabled(), pluginEntry.GetName(), wxNullBitmap, KxDataViewBitmapTextToggleValue::CheckBox);
			break;
		}
		case ColumnID::Index:
		{
			value = KPluginManager::GetInstance()->FormatPriority(pluginEntry);
			break;
		}
		case ColumnID::Type:
		{
			value = KPluginManager::GetInstance()->GetPluginTypeName(pluginEntry);
			break;
		}
		case ColumnID::PartOf:
		{
			value = GetPartOfName(pluginEntry);
			break;
		}
		case ColumnID::Author:
		{
			value = GetPluginAuthor(pluginEntry);
			break;
		}
	};
}
bool KPluginViewModelBethesda::SetValue(const wxAny& value, KPluginEntry& pluginEntry, const KxDataViewColumn* column)
{
	switch (column->GetID())
	{
		case ColumnID::Name:
		{
			pluginEntry.SetEnabled(value.As<bool>());
			return true;
		}
	};
	return false;
}

bool KPluginViewModelBethesda::IsEnabled(const KPluginEntry& pluginEntry, const KxDataViewColumn* column) const
{
	if (column->GetID() == ColumnID::Name)
	{
		return pluginEntry.CanToggleEnabled();
	}
	return true;
}
bool KPluginViewModelBethesda::IsEditorEnabled(const KPluginEntry& pluginEntry, const KxDataViewColumn* column) const
{
	return false;
}

bool KPluginViewModelBethesda::GetAttributes(const KPluginEntry& pluginEntry, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
{
	switch (column->GetID())
	{
		case ColumnID::Index:
		{
			attributes.SetFontFace("Consolas");
			return true;
		}
		case ColumnID::PartOf:
		{
			const KModEntry* modEntry = pluginEntry.GetParentMod();
			if (modEntry)
			{
				if (cellState & KxDATAVIEW_CELL_HIGHLIGHTED && column->IsHotTracked())
				{
					attributes.SetUnderlined(true);
				}

				const KPluginManagerConfigStdContentEntry* standardContentEntry = pluginEntry.GetStdContentEntry();
				if (modEntry->ToFixedEntry() && !standardContentEntry)
				{
					attributes.SetItalic(true);
				}
			}
			break;
		}
	};
	return !attributes.IsDefault();
}
bool KPluginViewModelBethesda::Compare(const KPluginEntry& pluginEntry1, const KPluginEntry& pluginEntry2, const KxDataViewColumn* column) const
{
	switch (column->GetID())
	{
		case ColumnID::Name:
		{
			return KxComparator::IsLess(pluginEntry1.GetName(), pluginEntry1.GetName());
		}
		default:
		case ColumnID::Index:
		{
			KPluginManager* manager = KPluginManager::GetInstance();
			return manager->GetPluginOrderIndex(pluginEntry1) < manager->GetPluginOrderIndex(pluginEntry2);
		}
		#if 0
		case ColumnID::Type:
		{
			return -entry1->GetFormat() < -entry2->GetFormat();
			break;
		}
		#endif
		case ColumnID::PartOf:
		{
			return KxComparator::IsLess(GetPartOfName(pluginEntry1), GetPartOfName(pluginEntry2));
		}
		case ColumnID::Author:
		{
			return KxComparator::IsLess(GetPluginAuthor(pluginEntry1), GetPluginAuthor(pluginEntry2));
		}
	};
	return false;
}

wxString KPluginViewModelBethesda::GetPartOfName(const KPluginEntry& entry) const
{
	if (const KPluginManagerConfigStdContentEntry* standardContentEntry = entry.GetStdContentEntry())
	{
		return standardContentEntry->GetName();
	}
	else if (const KModEntry* modEntry = entry.GetParentMod())
	{
		return modEntry->GetName();
	}
	return wxNullString;
}
wxString KPluginViewModelBethesda::GetPluginAuthor(const KPluginEntry& entry) const
{
	const KPluginReaderBethesda* bethesdaReader = NULL;
	if (entry.HasReader() && entry.GetReader()->As(bethesdaReader))
	{
		return bethesdaReader->GetAuthor();
	}
	return wxEmptyString;
}
