#include "stdafx.h"
#include "KPCRGroupsModel.h"
#include "KPCREntriesListModel.h"
#include "PackageProject/KPackageProject.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "PackageCreator/KPackageCreatorController.h"
#include "UI/KMainWindow.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxButton.h>
#include <KxFramework/KxDataViewComboBox.h>
#include <KxFramework/KxTextBoxDialog.h>
#include <KxFramework/KxTaskDialog.h>

enum ColumnID
{
	Name
};
enum MenuID
{
	AddGroup,
};

KxDataViewCtrl* KPCRGroupsModel::OnCreateDataView(wxWindow* window)
{
	m_ComboView = new KxDataViewComboBox();
	m_ComboView->SetDataViewFlags(KxDataViewComboBox::DefaultDataViewStyle|KxDV_NO_HEADER);
	m_ComboView->SetOptionEnabled(KxDVCB_OPTION_ALT_POPUP_WINDOW);
	m_ComboView->SetOptionEnabled(KxDVCB_OPTION_HORIZONTAL_SIZER);
	m_ComboView->SetOptionEnabled(KxDVCB_OPTION_DISMISS_ON_SELECT, false);
	m_ComboView->SetOptionEnabled(KxDVCB_OPTION_FORCE_GET_STRING_VALUE_ON_DISMISS);
	m_ComboView->Create(window, KxID_NONE);
	m_ComboView->ComboSetMaxVisibleItems(16);
	m_ComboView->Bind(KxEVT_DVCB_GET_STRING_VALUE, &KPCRGroupsModel::OnGetStringValue, this);

	return m_ComboView;
}
wxWindow* KPCRGroupsModel::OnGetDataViewWindow()
{
	return m_ComboView->GetComboControl();
}
void KPCRGroupsModel::OnInitControl()
{
	/* View */
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KPCRGroupsModel::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KPCRGroupsModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KPCRGroupsModel::OnContextMenu, this);

	/* Columns */
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(wxEmptyString, ColumnID::Name, KxDATAVIEW_CELL_EDITABLE);
}

void KPCRGroupsModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	KPPRRequirementsGroup* group = GetDataEntry(row);
	if (group)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = group->GetID();
				break;
			}
		};
	}
}
bool KPCRGroupsModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	KPPRRequirementsGroup* group = GetDataEntry(row);
	if (group)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				wxString newID = value.As<wxString>();
				if (newID != group->GetID())
				{
					if (newID.IsEmpty() || !m_Requirements->HasSetWithID(newID))
					{
						TrackChangeID(group->GetID(), newID);
						group->SetID(newID);
						ChangeNotify();
					}
					else
					{
						KPackageCreatorPageBase::WarnIDCollision(GetView(), GetView()->GetAdjustedItemRect(GetItem(row), column));
						return false;
					}
				}
				return true;
			}
		};
	}
	return false;
}

void KPCRGroupsModel::OnSelectItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	m_RemoveButton->Enable(item.IsOK());

	KPPRRequirementsGroup* group = GetDataEntry(item);
	if (group)
	{
		m_EntriesModel->SetRequirementsGroup(group);
		m_EntriesModel->SetDataVector(&group->GetEntries());
	}
	else
	{
		m_EntriesModel->SetRequirementsGroup(NULL);
		m_EntriesModel->SetDataVector();
	}
	RefreshComboControl();
}
void KPCRGroupsModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	if (item.IsOK() && column)
	{
		GetView()->EditItem(item, column);
	}
}
void KPCRGroupsModel::OnContextMenu(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	const KPPRRequirementsGroup* entry = GetDataEntry(item);

	KxMenu menu;
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddGroup, KTr(KxID_ADD)));
		item->SetBitmap(KGetBitmap(KIMG_PLUS_SMALL));
	}
	menu.AddSeparator();
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(KxID_REMOVE, KTr(KxID_REMOVE)));
		item->Enable(entry != NULL);
	}
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(KxID_CLEAR, KTr(KxID_CLEAR)));
		item->Enable(!IsEmpty());
	}

	switch (menu.Show(GetView()))
	{
		case MenuID::AddGroup:
		{
			OnAddGroup(true);
			break;
		}
		case KxID_REMOVE:
		{
			OnRemoveGroup(item);
			break;
		}
		case KxID_CLEAR:
		{
			OnClearList();
			break;
		}
	};
};
void KPCRGroupsModel::OnGetStringValue(KxDataViewEvent& event)
{
	KPPRRequirementsGroup* group = GetDataEntry(GetView()->GetSelection());
	event.SetString(group ? group->GetID() : KAux::MakeNoneLabel());
}

bool KPCRGroupsModel::DoTrackID(const wxString& trackedID, const wxString& newID, bool remove) const
{
	const wxString trackedFlagName = KPPRRequirementsGroup::GetFlagName(trackedID);
	const wxString newFlagName = KPPRRequirementsGroup::GetFlagName(newID);

	// Default groups
	TrackID_ReplaceOrRemove(trackedID, newID, GetProject().GetRequirements().GetDefaultGroup(), remove);

	// Manual components and flags
	for (auto& step: GetProject().GetComponents().GetSteps())
	{
		for (auto& group: step->GetGroups())
		{
			for (auto& entry: group->GetEntries())
			{
				TrackID_ReplaceOrRemove(trackedID, newID, entry->GetRequirements(), remove);

				// Flags
				TrackID_ReplaceOrRemove(trackedFlagName, newFlagName, entry->GetAssignedFlags(), remove);
				TrackID_ReplaceOrRemove(trackedFlagName, newFlagName, entry->GetTDConditions(), remove);
			}
		}
	}

	// Conditional steps flags
	for (auto& step: GetProject().GetComponents().GetConditionalSteps())
	{
		TrackID_ReplaceOrRemove(trackedFlagName, newFlagName, step->GetConditions(), remove);
	}

	return true;
}
void KPCRGroupsModel::RefreshComboControl()
{
	m_ComboView->ComboRefreshLabel();
	m_ComboView->GetComboControl()->Enable(!IsEmpty());
}

void KPCRGroupsModel::OnAddGroup(bool useDialog)
{
	wxString name;
	if (useDialog)
	{
		KxTextBoxDialog dialog(m_Controller->GetWorkspace(), KxID_NONE, KTr("PackageCreator.NewGroupDialog"));
		dialog.Bind(KxEVT_STDDIALOG_BUTTON, [this, &dialog](wxNotifyEvent& event)
		{
			if (event.GetId() == KxID_OK)
			{
				if (m_Requirements->HasSetWithID(dialog.GetValue()))
				{
					KPackageCreatorPageBase::WarnIDCollision(dialog.GetDialogMainCtrl());
					event.Veto();
				}
			}
		});

		if (dialog.ShowModal() == KxID_OK)
		{
			name = dialog.GetValue();
		}
	}

	auto& group = GetDataVector()->emplace_back(new KPPRRequirementsGroup());
	group->SetID(name);

	KxDataViewItem item = GetItem(GetItemCount() - 1);
	NotifyAddedItem(item);
	if (!useDialog)
	{
		GetView()->EditItem(item, GetView()->GetColumnByID(ColumnID::Name));
	}
}
void KPCRGroupsModel::OnRemoveGroup(const KxDataViewItem& item)
{
	if (KPPRRequirementsGroup* group = GetDataEntry(item))
	{
		KxTaskDialog dialog(m_Controller->GetWorkspace(), KxID_NONE, KTrf("PackageCreator.RemoveGroupDialog", group->GetID()), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
		if (dialog.ShowModal() == KxID_YES)
		{
			if (IsEmpty())
			{
				m_ComboView->ComboDismiss();
			}

			TrackRemoveID(group->GetID());
			RemoveItemAndNotify(*GetDataVector(), item);
			RefreshComboControl();
		}
	}
}
void KPCRGroupsModel::OnClearList()
{
	for (size_t i = 0; i < GetItemCount(); i++)
	{
		TrackRemoveID(GetDataEntry(i)->GetID());
	}

	m_ComboView->ComboDismiss();
	ClearItemsAndNotify(*GetDataVector());
	RefreshComboControl();
}

void KPCRGroupsModel::Create(KPackageCreatorController* controller, wxWindow* window, wxSizer* sizer)
{
	KPackageCreatorVectorModel::Create(controller, window, sizer);

	m_AddButton = new KxButton(window, KxID_NONE, KTr(KxID_ADD));
	m_AddButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
	{
		OnAddGroup(true);
	});
	sizer->Add(m_AddButton, 0, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING_SMALL);

	// Remove button
	m_RemoveButton = new KxButton(window, KxID_NONE, KTr(KxID_REMOVE));
	m_RemoveButton->Enable(false);
	m_RemoveButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
	{
		OnRemoveGroup(GetView()->GetSelection());
	});
	sizer->Add(m_RemoveButton, 0, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING_SMALL);
}
void KPCRGroupsModel::ChangeNotify()
{
	KPackageCreatorVectorModel::ChangeNotify();
	RefreshComboControl();
}
void KPCRGroupsModel::SetProject(KPackageProject& projectData)
{
	m_Requirements = &projectData.GetRequirements();

	KPackageCreatorVectorModel::SetDataVector(&m_Requirements->GetGroups());
	SelectItem(GetItemCount() != 0 ? 0 : (size_t)-1);
	RefreshComboControl();
}
