#include "stdafx.h"
#include "SelectorDisplayModel.h"
#include <Kortex/ModTagManager.hpp>

namespace Kortex::ModTagManager
{
	void SelectorDisplayModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &SelectorDisplayModel::OnActivate, this);

		// Override tag
		if (IsFullFeatured())
		{
			auto info = GetView()->AppendColumn<KxDataViewToggleRenderer>(KTr("ModManager.Tags.PriorityGroup"), ColumnID::PriorityGroup, KxDATAVIEW_CELL_ACTIVATABLE, KxCOL_WIDTH_AUTOSIZE, KxDV_COL_REORDERABLE);
			info.GetRenderer()->SetDefaultToggleType(KxDataViewToggleRenderer::ToggleType::RadioBox);
		}

		// Name
		if (IsFullFeatured())
		{
			GetView()->PrependColumn<KxDataViewBitmapTextToggleRenderer, KxDataViewTextEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE|KxDATAVIEW_CELL_ACTIVATABLE);
		}
		else
		{
			GetView()->AppendColumn<KxDataViewBitmapTextToggleRenderer, KxDataViewTextEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE|KxDATAVIEW_CELL_ACTIVATABLE);
		}
	}

	void SelectorDisplayModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		const IModTag* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = entry->GetName();
					break;
				}
			};
		}
	}
	void SelectorDisplayModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		const IModTag* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = KxDataViewBitmapTextToggleValue(m_Data->HasTag(*entry), entry->GetName(), KGetBitmap(!entry->IsSystemTag() ? KIMG_PLUS_SMALL : KIMG_NONE), KxDataViewBitmapTextToggleValue::CheckBox);
					break;
				}
				case ColumnID::PriorityGroup:
				{
					value = entry == m_PriorityGroupTag;
					break;
				}
			};
		}
	}
	bool SelectorDisplayModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
	{
		IModTag* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					if (value.CheckType<wxString>())
					{
						wxString name = value.As<wxString>();
						if (!name.IsEmpty() && entry->GetID() != name)
						{
							const bool hasTag = m_Data->HasTag(*entry);
							m_Data->ToggleTag(*entry, false);
							entry->SetID(name);

							// Re-add this tag if needed
							if (hasTag)
							{
								m_Data->ToggleTag(*entry, true);
							}
							return true;
						}
					}
					else
					{
						const bool hasTag = m_Data->HasTag(*entry);
						m_Data->ToggleTag(*entry, value.As<bool>());
						return hasTag;
					}
					return false;
				}
				case ColumnID::PriorityGroup:
				{
					bool checked = value.As<bool>();
					if (checked && m_PriorityGroupTag != entry)
					{
						m_PriorityGroupTag = entry;
						m_IsModified = true;
						return true;
					}
					else if (!checked && m_PriorityGroupTag)
					{
						m_PriorityGroupTag = nullptr;
						m_IsModified = true;
						return true;
					}
					return false;
				}
			};
		}
		return false;
	}
	bool SelectorDisplayModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
	{
		return true;
	}

	void SelectorDisplayModel::OnActivate(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		KxDataViewColumn* column = event.GetColumn();
		if (item.IsOK() && column && column->GetID() != ColumnID::Name)
		{
			GetView()->EditItem(item, GetView()->GetColumnByID(ColumnID::Name));
		}
	}
	const IModTag* SelectorDisplayModel::FindStdTag(const wxString& tagID) const
	{
		return IModTagManager::GetInstance()->FindTagByID(tagID);
	}

	SelectorDisplayModel::SelectorDisplayModel(bool isFullFeatured)
		:m_FullFeatured(isFullFeatured)
	{
		SetDataViewFlags(GetDataViewFlags()|KxDV_NO_TIMEOUT_EDIT);
	}

	void SelectorDisplayModel::SetDataVector(ModTagStore* tagStore, IGameMod* mod)
	{
		m_Data = tagStore;
		m_GameMod = mod;

		if (m_GameMod)
		{
			m_PriorityGroupTag = FindStdTag(m_GameMod->GetPriorityGroupTag());
		}
		RefreshItems();
	}
	size_t SelectorDisplayModel::GetItemCount() const
	{
		return m_Data ? IModTagManager::GetInstance()->GetTagsCount() : 0;
	}
	IModTag* SelectorDisplayModel::GetDataEntry(size_t index) const
	{
		IModTag::Vector& tags = IModTagManager::GetInstance()->GetTags();
		if (m_Data && index < tags.size())
		{
			return tags[index].get();
		}
		return nullptr;
	}

	void SelectorDisplayModel::ApplyChanges()
	{
		if (m_GameMod)
		{
			m_GameMod->SetPriorityGroupTag(m_PriorityGroupTag ? m_PriorityGroupTag->GetID() : wxEmptyString);
		}
	}
}
