#include "stdafx.h"
#include "ConditionGroupModel.h"
#include "PackageCreator/WorkspaceDocument.h"
#include "PackageCreator/PageBase.h"
#include "PackageProject/KPackageProjectComponents.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"
#include "Utility/KBitmapSize.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxComboBox.h>
#include <KxFramework/KxDataViewComboBox.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxUxTheme.h>

namespace
{
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
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	void ConditionGroupModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &ConditionGroupModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &ConditionGroupModel::OnContextMenu, this);
	
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
		m_ConditionColor = KxUxTheme::GetDialogMainInstructionColor(*GetView());
	}
	
	bool ConditionGroupModel::IsContainer(const KxDataViewItem& item) const
	{
		if (const PackageProject::Condition* condition = GetConditionEntry(item))
		{
			return condition->HasFlags();
		}
		return false;
	}
	void ConditionGroupModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
	{
		if (item.IsTreeRootItem())
		{
			for (const PackageProject::Condition& condition: m_ConditionGroup.GetConditions())
			{
				children.push_back(MakeItem(condition));
			}
		}
		else if (const PackageProject::Condition* condition = GetConditionEntry(item))
		{
			for (const PackageProject::FlagItem& flag: condition->GetFlags())
			{
				children.push_back(MakeItem(flag));
			}
		}
	}
	KxDataViewItem ConditionGroupModel::GetParent(const KxDataViewItem& item) const
	{
		if (const PackageProject::FlagItem* itemFlag = GetFlagEntry(item))
		{
			for (PackageProject::Condition& condition: m_ConditionGroup.GetConditions())
			{
				for (const PackageProject::FlagItem& flag: condition.GetFlags())
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
	
	bool ConditionGroupModel::IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		return true;
	}
	bool ConditionGroupModel::IsEditorEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		if (const PackageProject::Condition* condition = GetConditionEntry(item))
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					return true;
				}
			};
		}
		else if (const PackageProject::FlagItem* flag = GetFlagEntry(item))
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
	void ConditionGroupModel::GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		if (const PackageProject::Condition* condition = GetConditionEntry(item))
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					m_LabelEditor->SetItems(ModPackageProject::CreateOperatorSymNamesList({PackageProject::KPP_OPERATOR_AND, PackageProject::KPP_OPERATOR_OR}));
					m_LabelEditor->SetEditable(false);
	
					value = condition->GetOperator() == PackageProject::KPP_OPERATOR_AND ? 0 : 1;
					break;
				}
			};
		}
		else if (const PackageProject::FlagItem* flag = GetFlagEntry(item))
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
	void ConditionGroupModel::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		if (const PackageProject::Condition* condition = GetConditionEntry(item))
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = KxString::Format(wxS("%1: %2"), KTr(wxS("Generic.Operator")), ModPackageProject::OperatorToSymbolicName(condition->GetOperator()));
					break;
				}
			};
		}
		else if (const PackageProject::FlagItem* flag = GetFlagEntry(item))
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
	bool ConditionGroupModel::SetValue(const wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column)
	{
		if (PackageProject::Condition* condition = GetConditionEntry(item))
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					condition->SetOperator(value.As<int>() == 0 ? PackageProject::KPP_OPERATOR_AND : PackageProject::KPP_OPERATOR_OR);
					return true;
				}
			};
		}
		else if (PackageProject::FlagItem* flag = GetFlagEntry(item))
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
	bool ConditionGroupModel::GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
	{
		if (const PackageProject::Condition* condition = GetConditionEntry(item))
		{
			attributes.SetForegroundColor(m_ConditionColor);
			return true;
		}
		return false;
	}
	
	void ConditionGroupModel::OnActivateItem(KxDataViewEvent& event)
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
	void ConditionGroupModel::OnContextMenu(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		PackageProject::FlagItem* flag = GetFlagEntry(item);
		PackageProject::Condition* condition = GetConditionEntry(item);
		if (flag && !condition)
		{
			condition = GetConditionEntry(GetParent(item));
		}
	
		KxMenu menu;
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddToCondition, KTr("PackageCreator.Conditions.AddToCondition")));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FlagPlus));
			item->Enable(condition);
		}
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddNewCondition, KTr("PackageCreator.Conditions.AddNewCondition")));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderPlus));
		}
		menu.AddSeparator();
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::RemoveFromCondition, KTr("PackageCreator.Conditions.RemoveFromCondition")));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FlagMinus));
			item->Enable(flag);
		}
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::RemoveCondition, KTr("PackageCreator.Conditions.RemoveCondition")));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderMinus));
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
	
	void ConditionGroupModel::OnAddFlag(PackageProject::Condition& condition)
	{
		PackageProject::FlagItem& flag = condition.GetFlags().emplace_back(wxEmptyString, wxEmptyString);
		ChangeNotify();
		RefreshItems();
	
		KxDataViewItem item = MakeItem(flag);
		SelectItem(item);
		GetView()->EditItem(item, GetView()->GetColumn(ColumnID::Name));
	}
	void ConditionGroupModel::OnAddCondition()
	{
		PackageProject::Condition& condition = m_ConditionGroup.GetConditions().emplace_back();
		ChangeNotify();
		RefreshItems();
	
		KxDataViewItem item = MakeItem(condition);
		SelectItem(item);
		GetView()->EditItem(item, GetView()->GetColumn(ColumnID::Name));
	}
	void ConditionGroupModel::OnRemoveFlag(PackageProject::Condition& condition, PackageProject::FlagItem& flag)
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
	void ConditionGroupModel::OnRemoveCondition(PackageProject::Condition& condition)
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
				for (PackageProject::FlagItem& flag: condition.GetFlags())
				{
					TrackRemoveID(flag.GetName());
				}
				ChangeNotify();
				RefreshItems();
				break;
			}
			case RemoveMode::RemoveRename:
			{
				for (PackageProject::FlagItem& flag: condition.GetFlags())
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
	void ConditionGroupModel::OnRemoveAllConditions()
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
				for (PackageProject::Condition& condition: m_ConditionGroup.GetConditions())
				{
					for (const PackageProject::FlagItem& flag: condition.GetFlags())
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
				for (PackageProject::Condition& condition: m_ConditionGroup.GetConditions())
				{
					for (const PackageProject::FlagItem& flag: condition.GetFlags())
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
	
	int ConditionGroupModel::AskRemoveOption() const
	{
		KxTaskDialog dialog(GetViewTLW(), KxID_NONE, KTr("PackageCreator.Conditions.RemoveFlagDialog.Caption"), KTrf("PackageCreator.Conditions.RemoveFlagDialog.Message", PackageProject::FlagItem::GetDeletedFlagPrefix()), KxBTN_CANCEL, KxICON_WARNING);
		dialog.AddButton(RemoveMode::Remove, KTr("PackageCreator.Conditions.RemoveFlagDialog.Remove"));
		dialog.AddButton(RemoveMode::RemoveTrack, KTr("PackageCreator.Conditions.RemoveFlagDialog.RemoveTrack"));
		dialog.AddButton(RemoveMode::RemoveRename, KTr("PackageCreator.Conditions.RemoveFlagDialog.RemoveRename"));
	
		return dialog.ShowModal();
	}
	bool ConditionGroupModel::DoTrackID(wxString trackedID, const wxString& newID, bool remove) const
	{
		// Manual components
		for (auto& step: m_Project.GetComponents().GetSteps())
		{
			for (auto& group: step->GetGroups())
			{
				for (auto& entry: group->GetEntries())
				{
					TrackID_ReplaceOrRemove(trackedID, newID, entry->GetConditionalFlags().GetFlags(), remove);
					for (PackageProject::Condition& condition: entry->GetTDConditionGroup().GetConditions())
					{
						TrackID_ReplaceOrRemove(trackedID, newID, condition.GetFlags(), remove);
					}
				}
			}
		}
	
		// Conditional steps flags
		for (auto& step: m_Project.GetComponents().GetConditionalSteps())
		{
			for (PackageProject::Condition& condition: step->GetConditionGroup().GetConditions())
			{
				TrackID_ReplaceOrRemove(trackedID, newID, condition.GetFlags(), remove);
			}
		}
	
		return true;
	}
	void ConditionGroupModel::DoRemoveFlag(PackageProject::Condition& condition, PackageProject::FlagItem& flag)
	{
		PackageProject::FlagItem::Vector& flags = condition.GetFlags();
		auto it = std::remove_if(flags.begin(), flags.end(), [&flag](const PackageProject::FlagItem& thisFlag)
		{
			return &thisFlag == &flag;
		});
		flags.erase(it, flags.end());
	}
	void ConditionGroupModel::DoRemoveCondition(PackageProject::Condition& condition)
	{
		PackageProject::Condition::Vector& conditions = m_ConditionGroup.GetConditions();
		auto it = std::remove_if(conditions.begin(), conditions.end(), [&condition](const PackageProject::Condition& thisCondition)
		{
			return &thisCondition == &condition;
		});
		conditions.erase(it, conditions.end());
	}
	
	void ConditionGroupModel::ChangeNotify()
	{
		m_Controller->ChangeNotify();
	}
	void ConditionGroupModel::RemoveEmptyConditions()
	{
		PackageProject::Condition::Vector& conditions = m_ConditionGroup.GetConditions();
		for (size_t i = conditions.size(); i != 0; i--)
		{
			if (!conditions[i].HasFlags())
			{
				conditions.erase(conditions.begin() + i);
			}
		}
	}
	
	ConditionGroupModel::ConditionGroupModel(WorkspaceDocument* controller, PackageProject::ConditionGroup& conditionGroup)
		:m_Controller(controller), m_Project(*controller->GetProject()), m_ConditionGroup(conditionGroup)
	{
	}
	
	PackageProject::FlagItem* ConditionGroupModel::GetFlagEntry(const KxDataViewItem& item) const
	{
		return dynamic_cast<PackageProject::FlagItem*>(item.GetValuePtr<wxObject>());
	}
	PackageProject::Condition* ConditionGroupModel::GetConditionEntry(const KxDataViewItem& item) const
	{
		return dynamic_cast<PackageProject::Condition*>(item.GetValuePtr<wxObject>());
	}
	
	void ConditionGroupModel::RefreshItems()
	{
		KxDataViewModelExBase::RefreshItems();
		for (PackageProject::Condition& condition: m_ConditionGroup.GetConditions())
		{
			ItemAdded(MakeItem(condition));
			for (const PackageProject::FlagItem& flag: condition.GetFlags())
			{
				ItemAdded(MakeItem(condition), MakeItem(flag));
			}
			GetView()->Expand(MakeItem(condition));
		}
	}
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	void ConditionGroupDialog::OnSelectGlobalOperator(wxCommandEvent& event)
	{
		int index = event.GetInt();
		if (index != -1)
		{
			m_ConditionGroup.SetOperator((PackageProject::Operator)reinterpret_cast<size_t>(m_GlobalOperatorCB->GetClientData(index)));
			ChangeNotify();
		}
	}
	void ConditionGroupDialog::LoadWindowSizes()
	{
		//KProgramOptionSerializer::LoadDataViewLayout(GetView(), m_ViewOptions);
		//KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
	}
	
	ConditionGroupDialog::ConditionGroupDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller, PackageProject::ConditionGroup& conditionGroup)
		:ConditionGroupModel(controller, conditionGroup)
		//m_WindowOptions("ConditionGroupDialog", "Window"), m_ViewOptions("ConditionGroupDialog", "View")
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
			ConditionGroupModel::Create(m_ViewPane, m_Sizer);
	
			// General operator
			wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
			m_Sizer->Add(sizer, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);
	
			m_GlobalOperatorCB = PageBase::AddControlsRow(sizer, KTr("Generic.Operator"), new KxComboBox(m_ViewPane, KxID_NONE), 1);
			m_GlobalOperatorCB->Bind(wxEVT_COMBOBOX, &ConditionGroupDialogWithTypeDescriptor::OnSelectGlobalOperator, this);
	
			auto AddItem = [this](PackageProject::Operator value)
			{
				int index = m_GlobalOperatorCB->Append(ModPackageProject::OperatorToSymbolicName(value), reinterpret_cast<void*>(value));
				if (value == m_ConditionGroup.GetOperator())
				{
					m_GlobalOperatorCB->SetSelection(index);
				}
			};
			AddItem(PackageProject::KPP_OPERATOR_AND);
			AddItem(PackageProject::KPP_OPERATOR_OR);
	
			// Prepare
			AdjustWindow(wxDefaultPosition, FromDIP(wxSize(700, 400)));
			LoadWindowSizes();
		}
	}
	ConditionGroupDialog::~ConditionGroupDialog()
	{
		IncRef();
	
		//KProgramOptionSerializer::SaveDataViewLayout(GetView(), m_ViewOptions);
		//KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
	}
	
	int ConditionGroupDialog::ShowModal()
	{
		int ret = KxStdDialog::ShowModal();
		RemoveEmptyConditions();
		return ret;
	}
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	void ConditionGroupDialogWithTypeDescriptor::OnSelectNewTypeDescriptor(wxCommandEvent& event)
	{
		int index = event.GetInt();
		if (index != -1)
		{
			m_Entry.SetTDConditionalValue((PackageProject::TypeDescriptor)reinterpret_cast<size_t>(m_NewTypeDescriptorCB->GetClientData(index)));
			ChangeNotify();
		}
	}
	
	ConditionGroupDialogWithTypeDescriptor::ConditionGroupDialogWithTypeDescriptor(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller, PackageProject::ConditionGroup& conditionGroup, PackageProject::ComponentItem& entry)
		:ConditionGroupDialog(parent, caption, controller, conditionGroup), m_Entry(entry)
	{
		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		m_Sizer->Add(sizer, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);
	
		// New type descriptor
		m_NewTypeDescriptorCB = PageBase::AddControlsRow(sizer, KTr("PackageCreator.PageComponents.TypeDescriptorConditional"), new KxComboBox(m_ViewPane, KxID_NONE), 1);
		m_NewTypeDescriptorCB->Bind(wxEVT_COMBOBOX, &ConditionGroupDialogWithTypeDescriptor::OnSelectNewTypeDescriptor, this);
	
		auto AddItem = [this](const wxString& id, PackageProject::TypeDescriptor value)
		{
			int index = m_NewTypeDescriptorCB->Append(id, (void*)value);
			if (value == m_Entry.GetTDConditionalValue())
			{
				m_NewTypeDescriptorCB->SetSelection(index);
			}
		};
		AddItem(KAux::MakeNoneLabel(), PackageProject::KPPC_DESCRIPTOR_INVALID);
		AddItem(KTr("PackageCreator.PageComponents.TypeDescriptor.Optional"), PackageProject::KPPC_DESCRIPTOR_OPTIONAL);
		AddItem(KTr("PackageCreator.PageComponents.TypeDescriptor.Required"), PackageProject::KPPC_DESCRIPTOR_REQUIRED);
		AddItem(KTr("PackageCreator.PageComponents.TypeDescriptor.Recommended"), PackageProject::KPPC_DESCRIPTOR_RECOMMENDED);
		AddItem(KTr("PackageCreator.PageComponents.TypeDescriptor.CouldBeUsable"), PackageProject::KPPC_DESCRIPTOR_COULD_BE_USABLE);
		AddItem(KTr("PackageCreator.PageComponents.TypeDescriptor.NotUsable"), PackageProject::KPPC_DESCRIPTOR_NOT_USABLE);
	
		// Prepare
		AdjustWindow(wxDefaultPosition, FromDIP(wxSize(700, 400)));
		LoadWindowSizes();
	}
	ConditionGroupDialogWithTypeDescriptor::~ConditionGroupDialogWithTypeDescriptor()
	{
	}
}
