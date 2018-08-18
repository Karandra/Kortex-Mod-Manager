#include "stdafx.h"
#include "KModManagerModListEditor.h"
#include "KModManager.h"
#include "KAux.h"
#include "KApp.h"
#include <KxFramework/KxButton.h>
#include <KxFramework/KxString.h>

enum ColumnID
{
	Current,
	Name,
};

void KModManagerModListEditor::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KModManagerModListEditor::OnActivate, this);

	// Columns
	{
		auto info = GetView()->AppendColumn<KxDataViewToggleRenderer>(wxEmptyString, ColumnID::Current, KxDATAVIEW_CELL_ACTIVATABLE);
		info.GetRenderer()->SetDefaultToggleType(KxDataViewToggleRenderer::ToggleType::RadioBox);
	}
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE);
}

void KModManagerModListEditor::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	GetValueByRow(value, row, column);
}
void KModManagerModListEditor::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
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
bool KModManagerModListEditor::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	ValueType* entry = GetDataEntry(row);
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
						KxTaskDialog(GetView(), KxID_NONE, T(KxID_RENAME), T("ModManager.ModList.AlreadyExist"), KxBTN_OK, KxICON_ERROR).ShowModal();
						return false;
					}

					entry->SetID(name);
					SetCurrentList(name);
					MarkModified();
					return true;
				}
			}
		};
	}
	return false;
}
bool KModManagerModListEditor::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	return true;
}

void KModManagerModListEditor::OnActivate(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	if (item.IsOK())
	{
		GetView()->EditItem(item, GetView()->GetColumnByID(ColumnID::Name));
	}
}

KModManagerModListEditor::KModManagerModListEditor()
	:m_CurrentList(KModManager::GetListManager().GetCurrentListID())
{
}

//////////////////////////////////////////////////////////////////////////
void KModManagerModListEditorDialog::OnSelectItem(KxDataViewEvent& event)
{
	const KModList* entry = GetDataEntry(GetRow(event.GetItem()));
	if (entry)
	{
		m_CopyButton->Enable();
		m_RemoveButton->Enable(GetItemCount() > 1);
	}
	else
	{
		m_CopyButton->Disable();
		m_RemoveButton->Disable();
	}
}
void KModManagerModListEditorDialog::OnAddList(wxCommandEvent& event)
{
	KModManager::GetListManager().CreateNewList(wxEmptyString);
	MarkModified();
	RefreshItems();
	m_RemoveButton->Enable(!IsEmpty());

	KxDataViewItem tNewItem = GetItem(GetItemCount() - 1);
	SelectItem(tNewItem);
	GetView()->EditItem(tNewItem, GetView()->GetColumn(ColumnID::Name));
}
void KModManagerModListEditorDialog::OnCopyList(wxCommandEvent& event)
{
	KxDataViewItem item = GetView()->GetSelection();
	if (const KModList* entry = GetDataEntry(GetRow(item)))
	{
		KModManager::GetListManager().CreateListCopy(*entry, wxEmptyString);
		MarkModified();
		RefreshItems();

		KxDataViewItem tNewItem = GetItem(GetItemCount() - 1);
		SelectItem(tNewItem);
		GetView()->EditItem(tNewItem, GetView()->GetColumn(ColumnID::Name));
	}
}
void KModManagerModListEditorDialog::OnRemoveList(wxCommandEvent& event)
{
	if (KModManager::GetListManager().GetListsCount() > 1)
	{
		KxTaskDialog dialog(GetView(), KxID_NONE, T(KxID_REMOVE), T("ModManager.ModList.RemoveDialog"), KxBTN_YES|KxBTN_NO, KxICON_WARNING);
		if (dialog.ShowModal() == KxID_YES)
		{
			KxDataViewItem item = GetView()->GetSelection();
			if (const KModList* entry = GetDataEntry(GetRow(item)))
			{
				bool bLocalCurrent = entry->GetID() == GetCurrentList();
				if (KModManager::GetListManager().RemoveList(*entry))
				{
					if (bLocalCurrent)
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

KModManagerModListEditorDialog::KModManagerModListEditorDialog(wxWindow* parent)
{
	if (KxStdDialog::Create(parent, KxID_NONE, T("ModManager.ModList.Manage"), wxDefaultPosition, wxDefaultSize, KxBTN_OK))
	{
		SetMainIcon(KxICON_NONE);
		SetWindowResizeSide(wxBOTH);

		m_RemoveButton = AddButton(KxID_REMOVE, wxEmptyString, true).As<KxButton>();
		m_RemoveButton->Bind(wxEVT_BUTTON, &KModManagerModListEditorDialog::OnRemoveList, this);
		m_RemoveButton->Disable();

		m_AddButton = AddButton(KxID_ADD, wxEmptyString, true).As<KxButton>();
		m_AddButton->Bind(wxEVT_BUTTON, &KModManagerModListEditorDialog::OnAddList, this);
		
		m_CopyButton = AddButton(KxID_COPY, wxEmptyString, true).As<KxButton>();
		m_CopyButton->Bind(wxEVT_BUTTON, &KModManagerModListEditorDialog::OnCopyList, this);
		m_CopyButton->Disable();

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
		m_ViewPane->SetSizer(sizer);
		PostCreate();

		// List
		SetDataViewFlags(KxDV_NO_HEADER);
		KModManagerModListEditor::Create(m_ViewPane, sizer);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KModManagerModListEditorDialog::OnSelectItem, this);
		SetDataVector(&KModManager::GetListManager().GetLists());
		RefreshItems();

		AdjustWindow(wxDefaultPosition, wxSize(500, 350));
		GetView()->SetFocus();
	}
}
KModManagerModListEditorDialog::~KModManagerModListEditorDialog()
{
	IncRef();
}
