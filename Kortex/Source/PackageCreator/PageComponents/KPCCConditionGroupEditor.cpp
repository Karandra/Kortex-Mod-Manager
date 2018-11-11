#include "stdafx.h"
#include "KPCCConditionGroupEditor.h"
#include "PackageCreator/KPackageCreatorController.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "PackageProject/KPackageProjectComponents.h"
#include "UI/KMainWindow.h"
#include "KApp.h"
#include "KAux.h"
#include "KBitmapSize.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxComboBox.h>
#include <KxFramework/KxDataViewComboBox.h>
#include <KxFramework/KxTaskDialog.h>

enum ColumnID
{
	Name,
	Value,
};
enum MenuID
{
	AddNewCondition,
	AddToCondition,

	RemoveCondition,
	RemoveFromCondition,
	RemoveAll,
};
enum RemoveMode
{
	FIRST = KxID_HIGHEST,

	Remove,
	RemoveTrack,
	RemoveRename
};

void KPCCConditionGroupEditor::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KPCCConditionGroupEditor::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KPCCConditionGroupEditor::OnContextMenu, this);

	// Label
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewComboBoxEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE);
		m_LabelEditor = info.GetEditor();
	}

	// Value
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewComboBoxEditor>(KTr("Generic.Value"), ColumnID::Value, KxDATAVIEW_CELL_EDITABLE);
		m_ValueEditor = info.GetEditor();
	}

	RefreshItems();
	SetDataViewFlags(KxDV_NO_TIMEOUT_EDIT|KxDV_MODEL_ROW_HEIGHT);
	m_ConditionColor = KxUtility::GetThemeColor_Caption(GetView());
}

bool KPCCConditionGroupEditor::IsContainer(const KxDataViewItem& item) const
{
	if (const KPPCCondition* condition = GetConditionEntry(item))
	{
		return condition->HasFlags();
	}
	return false;
}
void KPCCConditionGroupEditor::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
{
	if (item.IsTreeRootItem())
	{
		for (const KPPCCondition& condition: m_ConditionGroup.GetConditions())
		{
			children.push_back(MakeItem(condition));
		}
	}
	else if (const KPPCCondition* condition = GetConditionEntry(item))
	{
		for (const KPPCFlagEntry& flag: condition->GetFlags())
		{
			children.push_back(MakeItem(flag));
		}
	}
}
KxDataViewItem KPCCConditionGroupEditor::GetParent(const KxDataViewItem& item) const
{
	if (const KPPCFlagEntry* itemFlag = GetFlagEntry(item))
	{
		for (KPPCCondition& condition: m_ConditionGroup.GetConditions())
		{
			for (const KPPCFlagEntry& flag: condition.GetFlags())
			{
				if (&flag == itemFlag)
				{
					return MakeItem(condition);
				}
			}
		}
	}
	return KxDataViewItem();
}

bool KPCCConditionGroupEditor::IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	return true;
}
bool KPCCConditionGroupEditor::IsEditorEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	if (const KPPCCondition* condition = GetConditionEntry(item))
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				return true;
			}
		};
	}
	else if (const KPPCFlagEntry* flag = GetFlagEntry(item))
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			case ColumnID::Value:
			{
				return true;
			}
		};
	}
	return false;
}
void KPCCConditionGroupEditor::GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	if (const KPPCCondition* condition = GetConditionEntry(item))
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				m_LabelEditor->SetItems(KPackageProject::CreateOperatorSymNamesList({KPP_OPERATOR_AND, KPP_OPERATOR_OR}));
				m_LabelEditor->SetEditable(false);

				value = condition->GetOperator() == KPP_OPERATOR_AND ? 0 : 1;
				break;
			}
		};
	}
	else if (const KPPCFlagEntry* flag = GetFlagEntry(item))
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				m_LabelEditor->SetItems(m_Project.GetComponents().GetFlagsNames());
				m_LabelEditor->SetEditable(true);

				value = flag->GetName();
				break;
			}
			case ColumnID::Value:
			{
				m_ValueEditor->SetItems(m_Project.GetComponents().GetFlagsValues());
				m_ValueEditor->SetEditable(true);

				value = flag->GetValue();
				break;
			}
		};
	}
}
void KPCCConditionGroupEditor::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	if (const KPPCCondition* condition = GetConditionEntry(item))
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = KxString::Format(wxS("%1: %2"), KTr(wxS("Generic.Operator")), KPackageProject::OperatorToSymbolicName(condition->GetOperator()));
				break;
			}
		};
	}
	else if (const KPPCFlagEntry* flag = GetFlagEntry(item))
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = flag->GetName();
				break;
			}
			case ColumnID::Value:
			{
				value = flag->GetValue();
				break;
			}
		};
	}
}
bool KPCCConditionGroupEditor::SetValue(const wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column)
{
	if (KPPCCondition* condition = GetConditionEntry(item))
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				condition->SetOperator(value.As<int>() == 0 ? KPP_OPERATOR_AND : KPP_OPERATOR_OR);
				return true;
			}
		};
	}
	else if (KPPCFlagEntry* flag = GetFlagEntry(item))
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				wxString newName = value.As<wxString>();
				TrackChangeID(flag->GetName(), newName);
				flag->SetName(newName);
				ChangeNotify();
				return true;
			}
			case ColumnID::Value:
			{
				flag->SetValue(value.As<wxString>());
				ChangeNotify();
				return true;
			}
		};
	}
	return false;
}
bool KPCCConditionGroupEditor::GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
{
	if (const KPPCCondition* condition = GetConditionEntry(item))
	{
		attributes.SetForegroundColor(m_ConditionColor);
		return true;
	}
	return false;
}

void KPCCConditionGroupEditor::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewColumn* column = event.GetColumn();
	if (column)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			case ColumnID::Value:
			{
				GetView()->EditItem(event.GetItem(), column);
			}
		};
	}
}
void KPCCConditionGroupEditor::OnContextMenu(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KPPCFlagEntry* flag = GetFlagEntry(item);
	KPPCCondition* condition = GetConditionEntry(item);
	if (flag && !condition)
	{
		condition = GetConditionEntry(GetParent(item));
	}

	KxMenu menu;
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddToCondition, KTr("PackageCreator.Conditions.AddToCondition")));
		item->SetBitmap(KGetBitmap(KIMG_FLAG_PLUS));
		item->Enable(condition);
	}
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddNewCondition, KTr("PackageCreator.Conditions.AddNewCondition")));
		item->SetBitmap(KGetBitmap(KIMG_FOLDER_PLUS));
	}
	menu.AddSeparator();
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::RemoveFromCondition, KTr("PackageCreator.Conditions.RemoveFromCondition")));
		item->SetBitmap(KGetBitmap(KIMG_FLAG_MINUS));
		item->Enable(flag);
	}
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::RemoveCondition, KTr("PackageCreator.Conditions.RemoveCondition")));
		item->SetBitmap(KGetBitmap(KIMG_FOLDER_MINUS));
		item->Enable(condition);
	}
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::RemoveAll, KTr("PackageCreator.Conditions.RemoveAll")));
		item->Enable(m_ConditionGroup.HasConditions());
	}

	switch (menu.Show(GetView()))
	{
		case MenuID::AddToCondition:
		{
			OnAddFlag(*condition);
			break;
		}
		case MenuID::AddNewCondition:
		{
			OnAddCondition();
			break;
		}

		case MenuID::RemoveFromCondition:
		{
			OnRemoveFlag(*condition, *flag);
			break;
		}
		case MenuID::RemoveCondition:
		{
			OnRemoveCondition(*condition);
			break;
		}
		case MenuID::RemoveAll:
		{
			OnRemoveAllConditions();
			break;
		}
	};
};

void KPCCConditionGroupEditor::OnAddFlag(KPPCCondition& condition)
{
	KPPCFlagEntry& flag = condition.GetFlags().emplace_back(wxEmptyString, wxEmptyString);
	ChangeNotify();
	RefreshItems();

	KxDataViewItem item = MakeItem(flag);
	SelectItem(item);
	GetView()->EditItem(item, GetView()->GetColumn(ColumnID::Name));
}
void KPCCConditionGroupEditor::OnAddCondition()
{
	KPPCCondition& condition = m_ConditionGroup.GetConditions().emplace_back();
	ChangeNotify();
	RefreshItems();

	KxDataViewItem item = MakeItem(condition);
	SelectItem(item);
	GetView()->EditItem(item, GetView()->GetColumn(ColumnID::Name));
}
void KPCCConditionGroupEditor::OnRemoveFlag(KPPCCondition& condition, KPPCFlagEntry& flag)
{
	switch (AskRemoveOption())
	{
		case RemoveMode::Remove:
		{
			DoRemoveFlag(condition, flag);
			ChangeNotify();
			RefreshItems();
			break;
		}
		case RemoveMode::RemoveTrack:
		{
			TrackRemoveID(flag.GetName());
			ChangeNotify();
			RefreshItems();
			break;
		}
		case RemoveMode::RemoveRename:
		{
			wxString name = flag.GetName();
			wxString newName = flag.GetDeletedName();
			DoRemoveFlag(condition, flag);
			TrackChangeID(name, newName);

			ChangeNotify();
			RefreshItems();
			break;
		}
	};
}
void KPCCConditionGroupEditor::OnRemoveCondition(KPPCCondition& condition)
{
	switch (AskRemoveOption())
	{
		case RemoveMode::Remove:
		{
			DoRemoveCondition(condition);
			ChangeNotify();
			RefreshItems();
			break;
		}
		case RemoveMode::RemoveTrack:
		{
			for (KPPCFlagEntry& flag: condition.GetFlags())
			{
				TrackRemoveID(flag.GetName());
			}
			ChangeNotify();
			RefreshItems();
			break;
		}
		case RemoveMode::RemoveRename:
		{
			for (KPPCFlagEntry& flag: condition.GetFlags())
			{
				wxString name = flag.GetName();
				wxString newName = flag.GetDeletedName();
				DoRemoveFlag(condition, flag);
				TrackChangeID(name, newName);
			}
			ChangeNotify();
			RefreshItems();
			break;
		}
	};
}
void KPCCConditionGroupEditor::OnRemoveAllConditions()
{
	switch (AskRemoveOption())
	{
		case RemoveMode::Remove:
		{
			m_ConditionGroup.GetConditions().clear();
			ChangeNotify();
			RefreshItems();
			break;
		}
		case RemoveMode::RemoveTrack:
		{
			for (KPPCCondition& condition: m_ConditionGroup.GetConditions())
			{
				for (const KPPCFlagEntry& flag: condition.GetFlags())
				{
					TrackRemoveID(flag.GetName());
				}
			}

			m_ConditionGroup.GetConditions().clear();
			ChangeNotify();
			RefreshItems();
			break;
		}
		case RemoveMode::RemoveRename:
		{
			for (KPPCCondition& condition: m_ConditionGroup.GetConditions())
			{
				for (const KPPCFlagEntry& flag: condition.GetFlags())
				{
					TrackChangeID(flag.GetName(), flag.GetDeletedName());
				}
			}

			m_ConditionGroup.GetConditions().clear();
			ChangeNotify();
			RefreshItems();
			break;
		}
	};
}

int KPCCConditionGroupEditor::AskRemoveOption() const
{
	KxTaskDialog dialog(GetViewTLW(), KxID_NONE, KTr("PackageCreator.Conditions.RemoveFlagDialog.Caption"), KTrf("PackageCreator.Conditions.RemoveFlagDialog.Message", KPPCFlagEntry::GetDeletedFlagPrefix()), KxBTN_CANCEL, KxICON_WARNING);
	dialog.AddButton(RemoveMode::Remove, KTr("PackageCreator.Conditions.RemoveFlagDialog.Remove"));
	dialog.AddButton(RemoveMode::RemoveTrack, KTr("PackageCreator.Conditions.RemoveFlagDialog.RemoveTrack"));
	dialog.AddButton(RemoveMode::RemoveRename, KTr("PackageCreator.Conditions.RemoveFlagDialog.RemoveRename"));

	return dialog.ShowModal();
}
bool KPCCConditionGroupEditor::DoTrackID(wxString trackedID, const wxString& newID, bool remove) const
{
	// Manual components
	for (auto& step: m_Project.GetComponents().GetSteps())
	{
		for (auto& group: step->GetGroups())
		{
			for (auto& entry: group->GetEntries())
			{
				TrackID_ReplaceOrRemove(trackedID, newID, entry->GetConditionalFlags().GetFlags(), remove);
				for (KPPCCondition& condition: entry->GetTDConditionGroup().GetConditions())
				{
					TrackID_ReplaceOrRemove(trackedID, newID, condition.GetFlags(), remove);
				}
			}
		}
	}

	// Conditional steps flags
	for (auto& step: m_Project.GetComponents().GetConditionalSteps())
	{
		for (KPPCCondition& condition: step->GetConditionGroup().GetConditions())
		{
			TrackID_ReplaceOrRemove(trackedID, newID, condition.GetFlags(), remove);
		}
	}

	return true;
}
void KPCCConditionGroupEditor::DoRemoveFlag(KPPCCondition& condition, KPPCFlagEntry& flag)
{
	KPPCFlagEntry::Vector& flags = condition.GetFlags();
	auto it = std::remove_if(flags.begin(), flags.end(), [&flag](const KPPCFlagEntry& thisFlag)
	{
		return &thisFlag == &flag;
	});
	flags.erase(it, flags.end());
}
void KPCCConditionGroupEditor::DoRemoveCondition(KPPCCondition& condition)
{
	KPPCCondition::Vector& conditions = m_ConditionGroup.GetConditions();
	auto it = std::remove_if(conditions.begin(), conditions.end(), [&condition](const KPPCCondition& thisCondition)
	{
		return &thisCondition == &condition;
	});
	conditions.erase(it, conditions.end());
}

void KPCCConditionGroupEditor::ChangeNotify()
{
	m_Controller->ChangeNotify();
}
void KPCCConditionGroupEditor::RemoveEmptyConditions()
{
	KPPCCondition::Vector& conditions = m_ConditionGroup.GetConditions();
	for (size_t i = conditions.size(); i != 0; i--)
	{
		if (!conditions[i].HasFlags())
		{
			conditions.erase(conditions.begin() + i);
		}
	}
}

KPCCConditionGroupEditor::KPCCConditionGroupEditor(KPackageCreatorController* controller, KPPCConditionGroup& conditionGroup)
	:m_Controller(controller), m_Project(*controller->GetProject()), m_ConditionGroup(conditionGroup)
{
}

KPPCFlagEntry* KPCCConditionGroupEditor::GetFlagEntry(const KxDataViewItem& item) const
{
	return dynamic_cast<KPPCFlagEntry*>(item.GetValuePtr<wxObject>());
}
KPPCCondition* KPCCConditionGroupEditor::GetConditionEntry(const KxDataViewItem& item) const
{
	return dynamic_cast<KPPCCondition*>(item.GetValuePtr<wxObject>());
}

void KPCCConditionGroupEditor::RefreshItems()
{
	KxDataViewModelExBase::RefreshItems();
	for (KPPCCondition& condition: m_ConditionGroup.GetConditions())
	{
		ItemAdded(MakeItem(condition));
		for (const KPPCFlagEntry& flag: condition.GetFlags())
		{
			ItemAdded(MakeItem(condition), MakeItem(flag));
		}
		GetView()->Expand(MakeItem(condition));
	}
}

//////////////////////////////////////////////////////////////////////////
void KPCCConditionGroupEditorDialog::OnSelectGlobalOperator(wxCommandEvent& event)
{
	int index = event.GetInt();
	if (index != -1)
	{
		m_ConditionGroup.SetOperator((KPPOperator)reinterpret_cast<size_t>(m_GlobalOperatorCB->GetClientData(index)));
		ChangeNotify();
	}
}
void KPCCConditionGroupEditorDialog::LoadWindowSizes()
{
	KProgramOptionSerializer::LoadDataViewLayout(GetView(), m_ViewOptions);
	KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
}

KPCCConditionGroupEditorDialog::KPCCConditionGroupEditorDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller, KPPCConditionGroup& conditionGroup)
	:KPCCConditionGroupEditor(controller, conditionGroup),
	m_WindowOptions("KPCCConditionGroupEditorDialog", "Window"), m_ViewOptions("KPCCConditionGroupEditorDialog", "View")
{
	if (KxStdDialog::Create(parent, KxID_NONE, caption, wxDefaultPosition, wxDefaultSize, KxBTN_OK))
	{
		SetMainIcon(KxICON_NONE);
		SetWindowResizeSide(wxBOTH);

		m_Sizer = new wxBoxSizer(wxVERTICAL);
		m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
		m_ViewPane->SetSizer(m_Sizer);
		PostCreate();

		// List
		KPCCConditionGroupEditor::Create(m_ViewPane, m_Sizer);

		// General operator
		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		m_Sizer->Add(sizer, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);

		m_GlobalOperatorCB = KPackageCreatorPageBase::AddControlsRow(sizer, KTr("Generic.Operator"), new KxComboBox(m_ViewPane, KxID_NONE), 1);
		m_GlobalOperatorCB->Bind(wxEVT_COMBOBOX, &KPCCConditionGroupEditorDialogTD::OnSelectGlobalOperator, this);

		auto AddItem = [this](KPPOperator value)
		{
			int index = m_GlobalOperatorCB->Append(KPackageProject::OperatorToSymbolicName(value), reinterpret_cast<void*>(value));
			if (value == m_ConditionGroup.GetOperator())
			{
				m_GlobalOperatorCB->SetSelection(index);
			}
		};
		AddItem(KPP_OPERATOR_AND);
		AddItem(KPP_OPERATOR_OR);

		// Prepare
		AdjustWindow(wxDefaultPosition, wxSize(700, 400));
		LoadWindowSizes();
	}
}
KPCCConditionGroupEditorDialog::~KPCCConditionGroupEditorDialog()
{
	IncRef();

	KProgramOptionSerializer::SaveDataViewLayout(GetView(), m_ViewOptions);
	KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
}

int KPCCConditionGroupEditorDialog::ShowModal()
{
	int ret = KxStdDialog::ShowModal();
	RemoveEmptyConditions();
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void KPCCConditionGroupEditorDialogTD::OnSelectNewTypeDescriptor(wxCommandEvent& event)
{
	int index = event.GetInt();
	if (index != -1)
	{
		m_Entry.SetTDConditionalValue((KPPCTypeDescriptor)reinterpret_cast<size_t>(m_NewTypeDescriptorCB->GetClientData(index)));
		ChangeNotify();
	}
}

KPCCConditionGroupEditorDialogTD::KPCCConditionGroupEditorDialogTD(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller, KPPCConditionGroup& conditionGroup, KPPCEntry& entry)
	:KPCCConditionGroupEditorDialog(parent, caption, controller, conditionGroup), m_Entry(entry)
{
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	m_Sizer->Add(sizer, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);

	// New type descriptor
	m_NewTypeDescriptorCB = KPackageCreatorPageBase::AddControlsRow(sizer, KTr("PackageCreator.PageComponents.TypeDescriptorConditional"), new KxComboBox(m_ViewPane, KxID_NONE), 1);
	m_NewTypeDescriptorCB->Bind(wxEVT_COMBOBOX, &KPCCConditionGroupEditorDialogTD::OnSelectNewTypeDescriptor, this);

	auto AddItem = [this](const wxString& id, KPPCTypeDescriptor value)
	{
		int index = m_NewTypeDescriptorCB->Append(id, (void*)value);
		if (value == m_Entry.GetTDConditionalValue())
		{
			m_NewTypeDescriptorCB->SetSelection(index);
		}
	};
	AddItem(KAux::MakeNoneLabel(), KPPC_DESCRIPTOR_INVALID);
	AddItem(KTr("PackageCreator.PageComponents.TypeDescriptor.Optional"), KPPC_DESCRIPTOR_OPTIONAL);
	AddItem(KTr("PackageCreator.PageComponents.TypeDescriptor.Required"), KPPC_DESCRIPTOR_REQUIRED);
	AddItem(KTr("PackageCreator.PageComponents.TypeDescriptor.Recommended"), KPPC_DESCRIPTOR_RECOMMENDED);
	AddItem(KTr("PackageCreator.PageComponents.TypeDescriptor.CouldBeUsable"), KPPC_DESCRIPTOR_COULD_BE_USABLE);
	AddItem(KTr("PackageCreator.PageComponents.TypeDescriptor.NotUsable"), KPPC_DESCRIPTOR_NOT_USABLE);

	// Prepare
	AdjustWindow(wxDefaultPosition, wxSize(700, 400));
	LoadWindowSizes();
}
KPCCConditionGroupEditorDialogTD::~KPCCConditionGroupEditorDialogTD()
{
}
