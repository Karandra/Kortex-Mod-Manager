#include "stdafx.h"
#include "KModManagerTagSelector.h"
#include "KModManager.h"
#include "KModEntry.h"
#include "KAux.h"
#include <KxFramework/KxDataViewComboBox.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxString.h>

enum ColumnID
{
	Name,
	PriorityGroup,
};

void KModManagerTagSelector::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KModManagerTagSelector::OnActivate, this);

	// Override tag
	if (IsFullFeatured())
	{
		auto info = GetView()->AppendColumn<KxDataViewToggleRenderer>(T("ModManager.Tags.PriorityGroup"), ColumnID::PriorityGroup, KxDATAVIEW_CELL_ACTIVATABLE, KxCOL_WIDTH_AUTOSIZE, KxDV_COL_REORDERABLE);
		info.GetRenderer()->SetDefaultToggleType(KxDataViewToggleRenderer::ToggleType::RadioBox);
	}

	// Name
	if (IsFullFeatured())
	{
		GetView()->PrependColumn<KxDataViewBitmapTextToggleRenderer, KxDataViewTextEditor>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE|KxDATAVIEW_CELL_ACTIVATABLE);
	}
	else
	{
		GetView()->AppendColumn<KxDataViewBitmapTextToggleRenderer, KxDataViewTextEditor>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE|KxDATAVIEW_CELL_ACTIVATABLE);
	}
}

void KModManagerTagSelector::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KModTag* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = entry->GetLabel();
				break;
			}
		};
	}
}
void KModManagerTagSelector::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KModTag* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = KxDataViewBitmapTextToggleValue(HasTag(entry), entry->GetLabel(), KGetBitmap(!entry->IsSystemTag() ? KIMG_PLUS_SMALL : KIMG_NONE), KxDataViewBitmapTextToggleValue::CheckBox);
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
bool KModManagerTagSelector::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	KModTag* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				if (value.CheckType<wxString>())
				{
					wxString name = value.As<wxString>();
					if (!name.IsEmpty() && entry->GetValue() != name)
					{
						bool bHadTag = ToggleTag(entry, false);
						entry->SetValue(name);

						// Re-add this tag if needed
						if (bHadTag)
						{
							ToggleTag(entry, true);
						}
						return true;
					}
				}
				else
				{
					return ToggleTag(entry, value.As<bool>());
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
					m_PriorityGroupTag = NULL;
					m_IsModified = true;
					return true;
				}
				return false;
			}
		};
	}
	return false;
}
bool KModManagerTagSelector::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	return true;
}

void KModManagerTagSelector::OnActivate(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	if (item.IsOK() && column && column->GetID() != ColumnID::Name)
	{
		GetView()->EditItem(item, GetView()->GetColumnByID(ColumnID::Name));
	}
}

const KModTag* KModManagerTagSelector::FindStdTag(const wxString& tagValue) const
{
	return KModManager::GetTagManager().FindModTag(tagValue);
}
KxStringVector::const_iterator KModManagerTagSelector::FindTag(const KModTag* entry) const
{
	return std::find_if(m_Data->cbegin(), m_Data->cend(), [entry](const wxString& sTag)
	{
		return sTag == entry->GetValue();
	});
}
bool KModManagerTagSelector::ToggleTag(const KModTag* entry, bool add)
{
	auto it = FindTag(entry);
	if (add && it == m_Data->cend())
	{
		m_Data->emplace_back(entry->GetValue());
		m_IsModified = true;
		return true;
	}
	else if (!add && it != m_Data->cend())
	{
		m_Data->erase(it);
		m_IsModified = true;
		return true;
	}
	return false;
}

KModManagerTagSelector::KModManagerTagSelector(bool bFullFeatured)
	:m_FullFeatured(bFullFeatured)
{
	SetDataViewFlags(GetDataViewFlags()|KxDV_NO_TIMEOUT_EDIT);
}

void KModManagerTagSelector::SetDataVector(KxStringVector* data, KModEntry* modEntry)
{
	m_Data = data;
	m_ModEntry = modEntry;

	if (m_ModEntry)
	{
		m_PriorityGroupTag = FindStdTag(m_ModEntry->GetPriorityGroupTag());
	}
	RefreshItems();
}
size_t KModManagerTagSelector::GetItemCount() const
{
	return m_Data ? KModManager::GetTagManager().GetTagsCount() : 0;
}
KModTag* KModManagerTagSelector::GetDataEntry(size_t index) const
{
	KModTagArray& tTags = KModManager::GetTagManager().GetTagList();
	if (m_Data && index < tTags.size())
	{
		return &tTags[index];
	}
	return NULL;
}

void KModManagerTagSelector::ApplyChanges()
{
	if (m_ModEntry)
	{
		m_ModEntry->SetPriorityGroupTag(m_PriorityGroupTag ? m_PriorityGroupTag->GetValue() : wxEmptyString);
	}
}

//////////////////////////////////////////////////////////////////////////
KxDataViewCtrl* KModManagerTagSelectorCB::OnCreateDataView(wxWindow* window)
{
	m_ComboView = new KxDataViewComboBox();
	m_ComboView->SetDataViewFlags(KxDV_NO_HEADER);
	m_ComboView->SetOptionEnabled(KxDVCB_OPTION_ALT_POPUP_WINDOW);
	m_ComboView->SetOptionEnabled(KxDVCB_OPTION_FORCE_GET_STRING_VALUE_ON_DISMISS);
	m_ComboView->Create(window, KxID_NONE);

	m_ComboView->Bind(KxEVT_DVCB_GET_STRING_VALUE, &KModManagerTagSelectorCB::OnGetStringValue, this);
	return m_ComboView;
}
wxWindow* KModManagerTagSelectorCB::OnGetDataViewWindow()
{
	return m_ComboView->GetComboControl();
}

wxString KModManagerTagSelectorCB::DoGetStingValue() const
{
	if (m_Data->size() != 0)
	{
		KxStringVector names;
		for (const wxString& tagID: *m_Data)
		{
			if (const KModTag* pTag = FindStdTag(tagID))
			{
				names.push_back(pTag->GetLabel());
			}
			else
			{
				names.push_back(tagID);
			}
		}
		return KxString::Join(names, ", ");
	}
	else
	{
		return V("<$T(ID_NONE)>");
	}
}
void KModManagerTagSelectorCB::SetStringValue(const wxString& value)
{
	m_ComboView->GetComboControl()->SetText(value);
}
void KModManagerTagSelectorCB::OnGetStringValue(KxDataViewEvent& event)
{
	event.SetString(DoGetStingValue());
}
bool KModManagerTagSelectorCB::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
{
	bool bRet = KModManagerTagSelector::SetValueByRow(data, row, column);
	if (bRet)
	{
		SetStringValue(DoGetStingValue());
	}
	return bRet;
}
bool KModManagerTagSelectorCB::IsEditorEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	return false;
}

void KModManagerTagSelectorCB::SetDataVector(KxStringVector* data)
{
	KModManagerTagSelector::SetDataVector(data);
	SetStringValue(m_Data ? DoGetStingValue() : wxEmptyString);
}

//////////////////////////////////////////////////////////////////////////
bool KModManagerTagSelectorDialog::IsEditorEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	const KModTag* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				return !entry->IsSystemTag();
			}
		};
	}
	return KModManagerTagSelector::IsEditorEnabledByRow(row, column);
}
void KModManagerTagSelectorDialog::OnSelectItem(KxDataViewEvent& event)
{
	const KModTag* entry = GetDataEntry(event.GetItem());
	m_RemoveButton->Enable(entry);
}

void KModManagerTagSelectorDialog::OnAddTag(wxCommandEvent& event)
{
	const KModTag* pNewTag = KModManager::GetTagManager().AddModTag(KAux::MakeBracketedLabel(T(KxID_NEW)));
	RefreshItems();

	KxDataViewItem tNewItem = GetItem(GetItemCount() - 1);
	SelectItem(tNewItem);
	GetView()->EditItem(tNewItem, GetView()->GetColumnByID(ColumnID::Name));
}
void KModManagerTagSelectorDialog::OnRemoveTag(wxCommandEvent& event)
{
	KxDataViewItem item = GetView()->GetSelection();
	const KModTag* entry = GetDataEntry(item);
	if (entry)
	{
		// Find all mods with this tag and remove it from them
		for (KModEntry* modEntry: KModManager::Get().GetEntries())
		{
			if (KModEntry::ToggleTag(modEntry->GetTags(), entry->GetValue(), false))
			{
				m_IsModified = true;
				if (m_AllowSave)
				{
					modEntry->Save();
				}
			}
		}

		// Remove tag from system
		KModManager::GetTagManager().RemoveModTag(entry->GetValue());

		// Reload control
		KxDataViewItem tPrevItem = GetPrevItem(item);
		RefreshItems();
		SelectItem(tPrevItem);
		GetView()->SetFocus();
	}
}

KModManagerTagSelectorDialog::KModManagerTagSelectorDialog(wxWindow* parent, const wxString& caption)
	:KModManagerTagSelector(true)
{
	if (KxStdDialog::Create(parent, KxID_NONE, caption, wxDefaultPosition, wxDefaultSize, KxBTN_OK))
	{
		SetMainIcon(KxICON_NONE);
		SetWindowResizeSide(wxBOTH);

		m_RemoveButton = AddButton(KxID_REMOVE, wxEmptyString, true).As<KxButton>();
		m_RemoveButton->Bind(wxEVT_BUTTON, &KModManagerTagSelectorDialog::OnRemoveTag, this);
		m_RemoveButton->Disable();

		m_AddButton = AddButton(KxID_ADD, wxEmptyString, true).As<KxButton>();
		m_AddButton->Bind(wxEVT_BUTTON, &KModManagerTagSelectorDialog::OnAddTag, this);

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
		m_ViewPane->SetSizer(sizer);
		PostCreate();

		// List
		KModManagerTagSelector::Create(m_ViewPane, sizer);

		AdjustWindow(wxDefaultPosition, wxSize(400, 550));
		GetView()->SetFocus();
	}
}
KModManagerTagSelectorDialog::~KModManagerTagSelectorDialog()
{
	IncRef();
}
