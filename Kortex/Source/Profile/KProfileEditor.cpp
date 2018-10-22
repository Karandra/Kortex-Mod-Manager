#include "stdafx.h"
#include "KProfileEditor.h"
#include "ModManager/KModManager.h"
#include "ModManager/KModEntry.h"
#include "GameInstance/KInstanceManagement.h"
#include "Profile/KProfile.h"
#include "KEvents.h"
#include "KAux.h"
#include "KApp.h"
#include <KxFramework/KxButton.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxCheckBox.h>
#include <KxFramework/KxComparator.h>

enum ColumnID
{
	Name,
	LocalSaves,
	LocalConfig,
};

void KProfileEditor::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KProfileEditor::OnActivate, this);

	// Columns
	{
		auto info = GetView()->AppendColumn<KxDataViewBitmapTextToggleRenderer>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_ACTIVATABLE, 250);
		info.GetRenderer()->SetDefaultToggleType(KxDataViewToggleRenderer::ToggleType::RadioBox);
	}
	GetView()->AppendColumn<KxDataViewToggleRenderer>(T("ModManager.Profile.LocalSaves"), ColumnID::LocalSaves, KxDATAVIEW_CELL_ACTIVATABLE);
	GetView()->AppendColumn<KxDataViewToggleRenderer>(T("ModManager.Profile.LocalConfig"), ColumnID::LocalConfig, KxDATAVIEW_CELL_ACTIVATABLE);
}

void KProfileEditor::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	GetValueByRow(value, row, column);
}
void KProfileEditor::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KProfile* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				bool isCurrent = KxComparator::IsEqual(entry->GetID(), m_NewCurrentProfile, true);
				value = KxDataViewBitmapTextToggleValue(isCurrent, entry->GetID(), wxNullBitmap, KxDataViewBitmapTextToggleValue::RadioBox);
				return;
			}
			case ColumnID::LocalSaves:
			{
				value = entry->IsLocalSavesEnabled();
				return;
			}
			case ColumnID::LocalConfig:
			{
				value = entry->IsLocalConfigEnabled();
				return;
			}
		};
	}
}
bool KProfileEditor::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	KProfile* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				if (value.CheckType<wxString>())
				{
					wxString name = value.As<wxString>();
					if (!name.IsEmpty() && name != entry->GetID())
					{
						if (KGameInstance::GetActive()->HasProfile(name))
						{
							KxTaskDialog(GetViewTLW(), KxID_NONE, T(KxID_RENAME), T("ModManager.Profile.AlreadyExist"), KxBTN_OK, KxICON_WARNING).ShowModal();
							return false;
						}

						bool isCurrent = entry->IsActive();
						if (KGameInstance::GetActive()->RenameProfile(*entry, name))
						{
							if (!isCurrent)
							{
								SetNewProfile(entry->GetID());
								MarkModified();
							}
							KProfileEvent(KEVT_PROFILE_CHANGED, *entry).Send();
							return true;
						}
						else
						{
							KxTaskDialog(GetViewTLW(), KxID_NONE, T(KxID_RENAME), T("ModManager.Profile.RenameFailed"), KxBTN_OK, KxICON_ERROR).ShowModal();
						}
					}
				}
				else
				{
					m_NewCurrentProfile = entry->GetID();
					MarkModified();
					return true;
				}
				return false;
			}
			case ColumnID::LocalSaves:
			{
				entry->SetLocalSavesEnabled(value.As<bool>());
				KProfileEvent(KEVT_PROFILE_CHANGED, *entry).Send();
				return true;
			}
			case ColumnID::LocalConfig:
			{
				entry->SetLocalConfigEnabled(value.As<bool>());
				KProfileEvent(KEVT_PROFILE_CHANGED, *entry).Send();
				return true;
			}
		};
	}
	return false;
}
bool KProfileEditor::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	return true;
}

void KProfileEditor::OnActivate(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	if (item.IsOK())
	{
		GetView()->EditItem(item, GetView()->GetColumnByID(ColumnID::Name));
	}
}

KProfileEditor::KProfileEditor()
	:m_NewCurrentProfile(KGameInstance::GetActive()->GetActiveProfileID())
{
}

//////////////////////////////////////////////////////////////////////////
void KModListManagerEditorDialog::OnSelectItem(KxDataViewEvent& event)
{
	const KProfile* entry = GetDataEntry(GetRow(event.GetItem()));
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
void KModListManagerEditorDialog::OnAddList(wxCommandEvent& event)
{
	KProfileEvent listEvent(KEVT_PROFILE_ADDING);
	listEvent.Send();
	if (!listEvent.IsAllowed())
	{
		return;
	}

	KProfile* newModList = KGameInstance::GetActive()->CreateProfile(listEvent.GetProfileID());
	if (newModList)
	{
		KProfileEvent(KEVT_PROFILE_ADDED, *newModList).Send();

		MarkModified();
		RefreshItems();
		m_RemoveButton->Enable(!IsEmpty());

		KxDataViewItem newItem = GetItem(GetItemCount() - 1);
		SelectItem(newItem);
		GetView()->EditItem(newItem, GetView()->GetColumn(ColumnID::Name));
	}
}
void KModListManagerEditorDialog::OnCopyList(wxCommandEvent& event)
{
	KxDataViewItem item = GetView()->GetSelection();
	if (KProfile* entry = GetDataEntry(GetRow(item)))
	{
		KProfileEvent listEvent(KEVT_PROFILE_ADDING, *entry);
		listEvent.Send();
		if (!listEvent.IsAllowed())
		{
			return;
		}

		KProfile* newModList = KGameInstance::GetActive()->ShallowCopyProfile(*entry, listEvent.GetProfileID());
		if (newModList)
		{
			KProfileEvent(KEVT_PROFILE_ADDED, *newModList).Send();

			MarkModified();
			RefreshItems();

			KxDataViewItem newItem = GetItem(GetItemCount() - 1);
			SelectItem(newItem);
			GetView()->EditItem(newItem, GetView()->GetColumn(ColumnID::Name));
		}
	}
}
void KModListManagerEditorDialog::OnRemoveList(wxCommandEvent& event)
{
	if (KGameInstance::GetActive()->HasProfiles())
	{
		KxDataViewItem item = GetView()->GetSelection();
		if (KProfile* entry = GetDataEntry(GetRow(item)))
		{
			KProfileEvent listEvent(KEVT_PROFILE_REMOVING, *entry);
			listEvent.Send();
			if (!listEvent.IsAllowed())
			{
				return;
			}

			KxTaskDialog dialog(GetView(), KxID_NONE, T(KxID_REMOVE), T("ModManager.Profile.RemoveDialog"), KxBTN_YES|KxBTN_NO, KxICON_WARNING);
			if (dialog.ShowModal() == KxID_YES)
			{
				const wxString profileID = entry->GetID();
				if (KGameInstance::GetActive()->RemoveProfile(*entry))
				{
					SetNewProfile(KGameInstance::GetActive()->GetActiveProfileID());
					KProfileEvent(KEVT_PROFILE_REMOVED, profileID).Send();

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

KModListManagerEditorDialog::KModListManagerEditorDialog(wxWindow* parent)
{
	if (KxStdDialog::Create(parent, KxID_NONE, T("ModManager.Profile.Configure"), wxDefaultPosition, wxDefaultSize, KxBTN_OK))
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
		KProfileEditor::Create(m_ViewPane, sizer);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KModListManagerEditorDialog::OnSelectItem, this);
		SetDataVector(&KGameInstance::GetActive()->GetProfiles());
		RefreshItems();

		AdjustWindow(wxDefaultPosition, wxSize(500, 375));
		GetView()->SetFocus();
	}
}
KModListManagerEditorDialog::~KModListManagerEditorDialog()
{
	IncRef();
}
