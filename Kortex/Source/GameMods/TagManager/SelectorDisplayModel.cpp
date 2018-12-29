#include "stdafx.h"
#include "SelectorDisplayModel.h"
#include <Kortex/ModTagManager.hpp>
#include <Kortex/NetworkManager.hpp>

namespace Kortex::ModTagManager
{
	void SelectorDisplayModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &SelectorDisplayModel::OnActivate, this);
		GetView()->Bind(wxEVT_CHAR_HOOK, &SelectorDisplayModel::OnKeyDown, this);

		// Priority group
		if (IsFullFeatured())
		{
			auto info = GetView()->AppendColumn<KxDataViewToggleRenderer>(KTr("ModManager.Tags.PriorityGroup"), ColumnID::PriorityGroup, KxDATAVIEW_CELL_ACTIVATABLE);
			info.GetRenderer()->SetDefaultToggleType(KxDataViewToggleRenderer::ToggleType::RadioBox);
		}

		// Name
		GetView()->AppendColumn<KxDataViewBitmapTextToggleRenderer, KxDataViewTextEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE|KxDATAVIEW_CELL_ACTIVATABLE);

		// NexusID
		if (IsFullFeatured())
		{
			auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>("NexusID", ColumnID::NexusID, KxDATAVIEW_CELL_EDITABLE);
			info.GetColumn()->SetBitmap(KGetBitmap(Network::NexusProvider::GetInstance()->GetIcon()));
		}

		// Color
		if (IsFullFeatured())
		{
			GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewColorEditor>(KTr("Generic.Color"), ColumnID::Color, KxDATAVIEW_CELL_EDITABLE);
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
				case ColumnID::NexusID:
				{
					const INexusModTag* nexusTag = nullptr;
					if (entry->QueryInterface(nexusTag))
					{
						value = nexusTag->GetNexusID();
					}
					break;
				}
				case ColumnID::Color:
				{
					value = entry->GetColor();
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
					value = KxDataViewBitmapTextToggleValue(m_Data->HasTag(*entry), entry->GetName(), wxNullBitmap, KxDataViewBitmapTextToggleValue::CheckBox);
					break;
				}
				case ColumnID::PriorityGroup:
				{
					value = entry == m_PriorityGroupTag;
					break;
				}
				case ColumnID::NexusID:
				{
					const INexusModTag* nexusTag = nullptr;
					if (entry->QueryInterface(nexusTag))
					{
						value = nexusTag->GetNexusID();
					}
					else
					{
						value = KAux::MakeNoneLabel();
					}
					break;
				}
				case ColumnID::Color:
				{
					KxColor color = entry->GetColor();
					value = color.IsOk() ? color.GetAsString(KxColor::ToString::HTMLSyntax) : KAux::MakeNoneLabel();
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
				case ColumnID::NexusID:
				{
					INexusModTag* nexusTag = nullptr;
					int id = -1;
					if (value.GetAs(&id) && id > 0 && entry->QueryInterface(nexusTag))
					{
						nexusTag->SetNexusID(id);
						return true;
					}
					break;
				}
				case ColumnID::Color:
				{
					entry->SetColor(value.As<KxColor>());
					return true;
					break;
				}
			};
		}
		return false;
	}
	bool SelectorDisplayModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
	{
		return true;
	}
	bool SelectorDisplayModel::GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attribute, KxDataViewCellState cellState) const
	{
		const IModTag* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Color:
				{
					KxColor color = entry->GetColor();
					if (color.IsOk())
					{
						color.SetA(200);
						attribute.SetBackgroundColor(color);
						attribute.SetForegroundColor(color.Negate());
						return true;
					}
					break;
				}
			};
		}
		return false;
	}

	void SelectorDisplayModel::OnActivate(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		KxDataViewColumn* column = event.GetColumn();
		if (item.IsOK() && column)
		{
			GetView()->EditItem(item, column);
		}
	}
	void SelectorDisplayModel::OnKeyDown(wxKeyEvent& event)
	{
		if (event.GetKeyCode() == WXK_DELETE)
		{
			KxDataViewItem item = GetView()->GetSelection();
			KxDataViewColumn* column = GetView()->GetCurrentColumn();
			if (item.IsOK() && column->GetID() == ColumnID::Color)
			{
				if (IModTag* entry = GetDataEntry(GetRow(item)))
				{
					entry->ResetColor();
					ItemChanged(item);
				}
			}
		}
		event.Skip();
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
