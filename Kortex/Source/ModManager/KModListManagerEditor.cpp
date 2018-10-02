#include "stdafx.h"
#include "KModListManagerEditor.h"
#include "KModManager.h"
#include "KEvents.h"
#include "KAux.h"
#include "KApp.h"
#include <KxFramework/KxButton.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxCheckBox.h>

enum ColumnID
{
	Current,
	Name,
};

void KModListManagerEditor::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KModListManagerEditor::OnActivate, this);

	// Columns
	{
		auto info = GetView()->AppendColumn<KxDataViewToggleRenderer>(wxEmptyString, ColumnID::Current, KxDATAVIEW_CELL_ACTIVATABLE);
		info.GetRenderer()->SetDefaultToggleType(KxDataViewToggleRenderer::ToggleType::RadioBox);
	}
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE);
}

void KModListManagerEditor::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	GetValueByRow(value, row, column);
}
void KModListManagerEditor::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KModList* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case  ColumnID::Current:
			{
				value = m_CurrentList == entry->GetID();
				return;
			}
			case ColumnID::Name:
			{
				value = entry->GetID();
				return;
			}
		};
	}
}
bool KModListManagerEditor::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	KModList* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Current:
			{
				m_CurrentList = entry->GetID();
				MarkModified();
				return true;
			}
			case ColumnID::Name:
			{
				wxString name = value.As<wxString>();
				if (!name.IsEmpty() && name != entry->GetID())
				{
					if (KModManager::GetListManager().HasList(name))
					{
						KxTaskDialog(GetViewTLW(), KxID_NONE, T(KxID_RENAME), T("ModManager.ModList.AlreadyExist"), KxBTN_OK, KxICON_WARNING).ShowModal();
						return false;
					}

					if (entry->SetID(name))
					{
						SetCurrentList(name);
						MarkModified();

						KModListEvent(KEVT_MODLIST_CHANGED, *entry).Send();
						return true;
					}
					else
					{
						KxTaskDialog(GetViewTLW(), KxID_NONE, T(KxID_RENAME), T("ModManager.ModList.RenameFailed"), KxBTN_OK, KxICON_ERROR).ShowModal();
					}
				}
			}
		};
	}
	return false;
}
bool KModListManagerEditor::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	return true;
}

void KModListManagerEditor::OnActivate(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	if (item.IsOK())
	{
		GetView()->EditItem(item, GetView()->GetColumnByID(ColumnID::Name));
	}
}

KModListManagerEditor::KModListManagerEditor()
	:m_CurrentList(KModManager::GetListManager().GetCurrentListID())
{
}

//////////////////////////////////////////////////////////////////////////
void KModListManagerEditorDialog::OnSelectItem(KxDataViewEvent& event)
{
	const KModList* entry = GetDataEntry(GetRow(event.GetItem()));
	if (entry)
	{
		m_CopyButton->Enable();
		m_RemoveButton->Enable(GetItemCount() > 1);
		
		m_LocalSavesCHK->Enable();
		m_LocalSavesCHK->SetValue(entry->IsLocalSavesEnabled());
		
		m_LocalConfigCHK->Enable();
		m_LocalConfigCHK->SetValue(entry->IsLocalConfigEnabled());
	}
	else
	{
		m_CopyButton->Disable();
		m_RemoveButton->Disable();

		m_LocalSavesCHK->Disable();
		m_LocalSavesCHK->SetValue(false);
		
		m_LocalConfigCHK->Disable();
		m_LocalConfigCHK->SetValue(false);
	}
}
void KModListManagerEditorDialog::OnAddList(wxCommandEvent& event)
{
	KModListEvent listEvent(KEVT_MODLIST_ADDING);
	listEvent.Send();
	if (!listEvent.IsAllowed())
	{
		return;
	}

	KModList& newModList = KModManager::GetListManager().CreateNewList(listEvent.GetModListID());
	KModListEvent(KEVT_MODLIST_ADDED, newModList).Send();

	MarkModified();
	RefreshItems();
	m_RemoveButton->Enable(!IsEmpty());

	KxDataViewItem newItem = GetItem(GetItemCount() - 1);
	SelectItem(newItem);
	GetView()->EditItem(newItem, GetView()->GetColumn(ColumnID::Name));
}
void KModListManagerEditorDialog::OnCopyList(wxCommandEvent& event)
{
	KxDataViewItem item = GetView()->GetSelection();
	if (KModList* entry = GetDataEntry(GetRow(item)))
	{
		KModListEvent listEvent(KEVT_MODLIST_ADDING, *entry);
		listEvent.Send();
		if (!listEvent.IsAllowed())
		{
			return;
		}

		KModList& newModList = KModManager::GetListManager().CreateListCopy(*entry, listEvent.GetModListID());
		KModListEvent(KEVT_MODLIST_ADDED, newModList).Send();

		MarkModified();
		RefreshItems();

		KxDataViewItem newItem = GetItem(GetItemCount() - 1);
		SelectItem(newItem);
		GetView()->EditItem(newItem, GetView()->GetColumn(ColumnID::Name));
	}
}
void KModListManagerEditorDialog::OnRemoveList(wxCommandEvent& event)
{
	if (KModManager::GetListManager().GetListsCount() > 1)
	{
		KxDataViewItem item = GetView()->GetSelection();
		if (KModList* entry = GetDataEntry(GetRow(item)))
		{
			KModListEvent listEvent(KEVT_MODLIST_REMOVING, *entry);
			listEvent.Send();
			if (!listEvent.IsAllowed())
			{
				return;
			}

			KxTaskDialog dialog(GetView(), KxID_NONE, T(KxID_REMOVE), T("ModManager.ModList.RemoveDialog"), KxBTN_YES|KxBTN_NO, KxICON_WARNING);
			if (dialog.ShowModal() == KxID_YES)
			{
				const wxString modListID = entry->GetID();
				bool isLocalCurrent = modListID == GetCurrentList();
				if (KModManager::GetListManager().RemoveList(*entry))
				{
					KModListEvent(KEVT_MODLIST_REMOVED, modListID).Send();

					if (isLocalCurrent)
					{
						SetCurrentList(KModManager::GetListManager().GetCurrentListID());
					}

					size_t index = GetRow(item);
					MarkModified();
					RefreshItems();
					SelectItem(GetItem(index < GetItemCount() ? index : GetItemCount() - 1));

					GetView()->SetFocus();
					m_RemoveButton->Enable(GetItemCount() != 0);
				}
			}
		}
	}
}

void KModListManagerEditorDialog::OnLocalSavesEnabled(wxCommandEvent& event)
{
	KxDataViewItem item = GetView()->GetSelection();
	KModList* entry = GetDataEntry(GetRow(item));
	if (entry)
	{
		entry->SetLocalSavesEnabled(m_LocalSavesCHK->IsChecked());
		KModListEvent(KEVT_MODLIST_CHANGED, *entry).Send();
	}
}
void KModListManagerEditorDialog::OnLocalConfigEnabled(wxCommandEvent& event)
{
	KxDataViewItem item = GetView()->GetSelection();
	KModList* entry = GetDataEntry(GetRow(item));
	if (entry)
	{
		entry->SetLocalConfigEnabled(m_LocalConfigCHK->IsChecked());
		KModListEvent(KEVT_MODLIST_CHANGED, *entry).Send();
	}
}

KModListManagerEditorDialog::KModListManagerEditorDialog(wxWindow* parent)
{
	if (KxStdDialog::Create(parent, KxID_NONE, T("ModManager.ModList.Manage"), wxDefaultPosition, wxDefaultSize, KxBTN_OK))
	{
		SetMainIcon(KxICON_NONE);
		SetWindowResizeSide(wxBOTH);

		m_RemoveButton = AddButton(KxID_REMOVE, wxEmptyString, true).As<KxButton>();
		m_RemoveButton->Bind(wxEVT_BUTTON, &KModListManagerEditorDialog::OnRemoveList, this);
		m_RemoveButton->Disable();

		m_AddButton = AddButton(KxID_ADD, wxEmptyString, true).As<KxButton>();
		m_AddButton->Bind(wxEVT_BUTTON, &KModListManagerEditorDialog::OnAddList, this);
		
		m_CopyButton = AddButton(KxID_COPY, wxEmptyString, true).As<KxButton>();
		m_CopyButton->Bind(wxEVT_BUTTON, &KModListManagerEditorDialog::OnCopyList, this);
		m_CopyButton->Disable();

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
		m_ViewPane->SetSizer(sizer);
		PostCreate();

		// List
		SetDataViewFlags(KxDV_NO_HEADER);
		KModListManagerEditor::Create(m_ViewPane, sizer);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KModListManagerEditorDialog::OnSelectItem, this);
		SetDataVector(&KModManager::GetListManager().GetLists());
		RefreshItems();

		// Check boxes
		m_LocalSavesCHK = new KxCheckBox(m_ViewPane, KxID_NONE, T("ModManager.ModList.LocalSaves"));
		m_LocalSavesCHK->Bind(wxEVT_CHECKBOX, &KModListManagerEditorDialog::OnLocalSavesEnabled, this);
		m_LocalSavesCHK->Disable();
		sizer->Add(m_LocalSavesCHK, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);
		
		m_LocalConfigCHK = new KxCheckBox(m_ViewPane, KxID_NONE, T("ModManager.ModList.LocalConfig"));
		m_LocalConfigCHK->Bind(wxEVT_CHECKBOX, &KModListManagerEditorDialog::OnLocalConfigEnabled, this);
		m_LocalConfigCHK->Disable();
		sizer->Add(m_LocalConfigCHK, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);

		AdjustWindow(wxDefaultPosition, wxSize(500, 375));
		GetView()->SetFocus();
	}
}
KModListManagerEditorDialog::~KModListManagerEditorDialog()
{
	IncRef();
}
