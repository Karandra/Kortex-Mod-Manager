#include "stdafx.h"
#include "GroupsModel.h"
#include "EntriesListModel.h"
#include "PackageProject/ModPackageProject.h"
#include "PackageCreator/PageBase.h"
#include "PackageCreator/WorkspaceDocument.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"
#include <KxFramework/KxButton.h>
#include <KxFramework/KxDataViewComboBox.h>
#include <KxFramework/KxTextBoxDialog.h>
#include <KxFramework/KxTaskDialog.h>

namespace Kortex::PackageDesigner
{
	enum ColumnID
	{
		Name,
		Operator
	};
	enum MenuID
	{
		AddGroup,
	};
}

namespace Kortex::PackageDesigner::PageRequirementsNS
{
	KxDataViewCtrl* GroupsModel::OnCreateDataView(wxWindow* window)
	{
		m_ComboView = new KxDataViewComboBox();
		m_ComboView->SetOptionEnabled(KxDVCB_OPTION_ALT_POPUP_WINDOW);
		m_ComboView->SetOptionEnabled(KxDVCB_OPTION_HORIZONTAL_SIZER);
		m_ComboView->SetOptionEnabled(KxDVCB_OPTION_DISMISS_ON_SELECT, false);
		m_ComboView->SetOptionEnabled(KxDVCB_OPTION_FORCE_GET_STRING_VALUE_ON_DISMISS);
		m_ComboView->Create(window, KxID_NONE);
		m_ComboView->ComboSetMaxVisibleItems(16);
		m_ComboView->Bind(KxEVT_DVCB_GET_STRING_VALUE, &GroupsModel::OnGetStringValue, this);
	
		return m_ComboView;
	}
	wxWindow* GroupsModel::OnGetDataViewWindow()
	{
		return m_ComboView->GetComboControl();
	}
	void GroupsModel::OnInitControl()
	{
		/* View */
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &GroupsModel::OnSelectItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &GroupsModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &GroupsModel::OnContextMenu, this);
	
		/* Columns */
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE);
		{
			auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewComboBoxEditor>(KTr("Generic.Operator"), ColumnID::Operator, KxDATAVIEW_CELL_EDITABLE);
			info.GetEditor()->SetItems(ModPackageProject::CreateOperatorSymNamesList({PackageProject::Operator::And, PackageProject::Operator::Or}));
		}
	}
	
	void GroupsModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		PackageProject::RequirementGroup* group = GetDataEntry(row);
		if (group)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = group->GetID();
					break;
				}
				case ColumnID::Operator:
				{
					value = group->GetOperator() == PackageProject::Operator::And ? 0 : 1;
					break;
				}
			};
		}
	}
	void GroupsModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		PackageProject::RequirementGroup* group = GetDataEntry(row);
		if (group)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = group->GetID();
					break;
				}
				case ColumnID::Operator:
				{
					value = ModPackageProject::OperatorToSymbolicName(group->GetOperator());
					break;
				}
			};
		}
	}
	bool GroupsModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
	{
		PackageProject::RequirementGroup* group = GetDataEntry(row);
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
							PageBase::WarnIDCollision(GetView(), GetView()->GetAdjustedItemRect(GetItem(row), column));
							return false;
						}
					}
					return true;
				}
				case ColumnID::Operator:
				{
					group->SetOperator(value.As<int>() == 0 ? PackageProject::Operator::And : PackageProject::Operator::Or);
					return true;
				}
			};
		}
		return false;
	}
	
	void GroupsModel::OnSelectItem(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		m_RemoveButton->Enable(item.IsOK());
	
		PackageProject::RequirementGroup* group = GetDataEntry(item);
		if (group)
		{
			m_EntriesModel->SetRequirementsGroup(group);
			m_EntriesModel->SetDataVector(&group->GetEntries());
		}
		else
		{
			m_EntriesModel->SetRequirementsGroup(nullptr);
			m_EntriesModel->SetDataVector();
		}
		RefreshComboControl();
	}
	void GroupsModel::OnActivateItem(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		KxDataViewColumn* column = event.GetColumn();
		if (item.IsOK() && column)
		{
			GetView()->EditItem(item, column);
		}
	}
	void GroupsModel::OnContextMenu(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		const PackageProject::RequirementGroup* entry = GetDataEntry(item);
	
		KxMenu menu;
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddGroup, KTr(KxID_ADD)));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::PlusSmall));
		}
		menu.AddSeparator();
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(KxID_REMOVE, KTr(KxID_REMOVE)));
			item->Enable(entry != nullptr);
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
	void GroupsModel::OnGetStringValue(KxDataViewEvent& event)
	{
		PackageProject::RequirementGroup* group = GetDataEntry(GetView()->GetSelection());
		event.SetString(group ? group->GetID() : KAux::MakeNoneLabel());
	}
	
	bool GroupsModel::DoTrackID(const wxString& trackedID, const wxString& newID, bool remove) const
	{
		const wxString trackedFlagName = PackageProject::RequirementGroup::GetFlagName(trackedID);
		const wxString newFlagName = PackageProject::RequirementGroup::GetFlagName(newID);
	
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
					TrackID_ReplaceOrRemove(trackedFlagName, newFlagName, entry->GetConditionalFlags().GetFlags(), remove);
					for (PackageProject::Condition& condition: entry->GetTDConditionGroup().GetConditions())
					{
						TrackID_ReplaceOrRemove(trackedFlagName, newFlagName, condition.GetFlags(), remove);
					}
				}
			}
		}
	
		// Conditional steps flags
		for (auto& step: GetProject().GetComponents().GetConditionalSteps())
		{
			for (PackageProject::Condition& condition : step->GetConditionGroup().GetConditions())
			{
				TrackID_ReplaceOrRemove(trackedFlagName, newFlagName, condition.GetFlags(), remove);
			}
		}
	
		return true;
	}
	void GroupsModel::RefreshComboControl()
	{
		m_ComboView->ComboRefreshLabel();
		m_ComboView->GetComboControl()->Enable(!IsEmpty());
	}
	
	void GroupsModel::OnAddGroup(bool useDialog)
	{
		wxString name;
		if (useDialog)
		{
			KxTextBoxDialog dialog(GetView(), KxID_NONE, KTr("PackageCreator.NewGroupDialog"));
			dialog.Bind(KxEVT_STDDIALOG_BUTTON, [this, &dialog](wxNotifyEvent& event)
			{
				if (event.GetId() == KxID_OK)
				{
					if (m_Requirements->HasSetWithID(dialog.GetValue()))
					{
						PageBase::WarnIDCollision(dialog.GetDialogMainCtrl());
						event.Veto();
					}
				}
			});
	
			if (dialog.ShowModal() == KxID_OK)
			{
				name = dialog.GetValue();
			}
		}
	
		auto& group = GetDataVector()->emplace_back(new PackageProject::RequirementGroup());
		group->SetID(name);
	
		KxDataViewItem item = GetItem(GetItemCount() - 1);
		NotifyAddedItem(item);
		if (!useDialog)
		{
			GetView()->EditItem(item, GetView()->GetColumnByID(ColumnID::Name));
		}
	}
	void GroupsModel::OnRemoveGroup(const KxDataViewItem& item)
	{
		if (PackageProject::RequirementGroup* group = GetDataEntry(item))
		{
			KxTaskDialog dialog(GetView(), KxID_NONE, KTrf("PackageCreator.RemoveGroupDialog", group->GetID()), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
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
	void GroupsModel::OnClearList()
	{
		for (size_t i = 0; i < GetItemCount(); i++)
		{
			TrackRemoveID(GetDataEntry(i)->GetID());
		}
	
		m_ComboView->ComboDismiss();
		ClearItemsAndNotify(*GetDataVector());
		RefreshComboControl();
	}
	
	void GroupsModel::Create(WorkspaceDocument* controller, wxWindow* window, wxSizer* sizer)
	{
		VectorModel::Create(controller, window, sizer);
	
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
	void GroupsModel::ChangeNotify()
	{
		VectorModel::ChangeNotify();
		RefreshComboControl();
	}
	void GroupsModel::SetProject(ModPackageProject& projectData)
	{
		m_Requirements = &projectData.GetRequirements();
	
		VectorModel::SetDataVector(&m_Requirements->GetGroups());
		SelectItem(GetItemCount() != 0 ? 0 : (size_t)-1);
		RefreshComboControl();
	}
}
