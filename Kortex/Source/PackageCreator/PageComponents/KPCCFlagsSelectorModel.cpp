#include "stdafx.h"
#include "KPCCFlagsSelectorModel.h"
#include "PackageCreator/KPackageCreatorController.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "UI/KMainWindow.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxComboBox.h>
#include <KxFramework/KxDataViewComboBox.h>
#include <KxFramework/KxTaskDialog.h>

enum ColumnID
{
	Name,
	Value,
	Operator
};
enum MenuID
{
	AddFlag,
};

void KPCCFlagsSelectorModel::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KPCCFlagsSelectorModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KPCCFlagsSelectorModel::OnContextMenu, this);

	// Label
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewComboBoxEditor>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE);
		m_LabelEditor = info.GetEditor();
		m_LabelEditor->SetEditable(true);
	}

	// Value
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewComboBoxEditor>(T("Generic.Value"), ColumnID::Value, KxDATAVIEW_CELL_EDITABLE);
		m_ValueEditor = info.GetEditor();
		m_ValueEditor->SetEditable(true);
	}

	// Operator
	if (!m_IsAssign)
	{
		KxStringVector tChoices;
		tChoices.push_back(KPackageProjectRequirements::OperatorToSymbolicName(KPP_OPERATOR_AND));
		tChoices.push_back(KPackageProjectRequirements::OperatorToSymbolicName(KPP_OPERATOR_OR));

		auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewComboBoxEditor>(T("PackageCreator.Operator"), ColumnID::Operator, KxDATAVIEW_CELL_EDITABLE);
		m_OperatorEditor = info.GetEditor();
		m_OperatorEditor->SetItems
		({
			KPackageProjectRequirements::OperatorToSymbolicName(KPP_OPERATOR_AND),
			KPackageProjectRequirements::OperatorToSymbolicName(KPP_OPERATOR_OR)
		 });
	}
}

void KPCCFlagsSelectorModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	auto entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = entry->GetName();
				break;
			}
			case ColumnID::Value:
			{
				value = entry->GetValue();
				break;
			}
			case ColumnID::Operator:
			{
				value = entry->GetOperator() == KPP_OPERATOR_AND ? 0 : 1;
				break;
			}
		};
	}
}
void KPCCFlagsSelectorModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	auto entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = entry->GetName();
				break;
			}
			case ColumnID::Value:
			{
				value = entry->GetValue();
				break;
			}
			case ColumnID::Operator:
			{
				value = m_OperatorEditor->GetItems()[entry->GetOperator() == KPP_OPERATOR_AND ? 0 : 1];
				break;
			}
		};
	}
}
bool KPCCFlagsSelectorModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	auto entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				wxString newName = value.As<wxString>();
				if (IsAssignMode() && newName.StartsWith(KPPRRequirementsGroup::GetFlagNamePrefix()))
				{
					KPackageCreatorPageBase::ShowTooltipWarning(GetView(), T("PackageCreator.InvalidFlagName"), GetItemRect(GetItem(row), column));
					return false;
				}

				TrackChangeID(entry->GetName(), newName);
				entry->SetName(newName);
				ChangeNotify();
				return true;
			}
			case ColumnID::Value:
			{
				entry->SetValue(value.As<wxString>());
				ChangeNotify();
				return true;
			}
			case ColumnID::Operator:
			{
				entry->SetOperator(value.As<int>() == 0 ? KPP_OPERATOR_AND : KPP_OPERATOR_OR);
				ChangeNotify();
				return true;
			}
		};
	}
	return false;
}

void KPCCFlagsSelectorModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewColumn* column = event.GetColumn();
	if (column)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			case ColumnID::Value:
			case ColumnID::Operator:
			{
				GetView()->EditItem(event.GetItem(), column);
			}
		};
	}
}
void KPCCFlagsSelectorModel::OnContextMenu(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	const KPPCFlagEntry* entry = GetDataEntry(GetRow(item));

	KxMenu menu;
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddFlag, T(KxID_ADD)));
		item->SetBitmap(KGetBitmap(KIMG_FLAG_PLUS));
	}
	menu.AddSeparator();
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(KxID_REMOVE, T(KxID_REMOVE)));
		item->Enable(entry != NULL);
	}
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(KxID_CLEAR, T(KxID_CLEAR)));
		item->Enable(!IsEmpty());
	}

	switch (menu.Show(GetView()))
	{
		case MenuID::AddFlag:
		{
			OnAddFlag();
			break;
		}
		case KxID_REMOVE:
		{
			OnRemoveFlag(item);
			break;
		}
		case KxID_CLEAR:
		{
			OnClearList();
			break;
		}
	};
};

void KPCCFlagsSelectorModel::OnAddFlag()
{
	GetDataVector()->emplace_back(KPPCFlagEntry(wxEmptyString, wxEmptyString, KPP_OPERATOR_AND));

	KxDataViewItem item = GetItem(GetItemCount() - 1);
	NotifyAddedItem(item);
	SelectItem(item);
	GetView()->EditItem(item, GetView()->GetColumn(ColumnID::Name));
}
void KPCCFlagsSelectorModel::OnRemoveFlag(const KxDataViewItem& item)
{
	if (KPPCFlagEntry* entry = GetDataEntry(GetRow(item)))
	{
		wxString sDeletedName = entry->GetDeletedName();
		KxTaskDialog dialog(GetViewTLW(), KxID_NONE, T("PackageCreator.RemoveFlagDialog.Caption", entry->GetName()), T("PackageCreator.RemoveFlagDialog.Message", sDeletedName), KxBTN_CANCEL, KxICON_WARNING);
		dialog.AddButton(KxID_REMOVE, T("PackageCreator.RemoveFlagDialog.Remove"));
		dialog.AddButton(KxID_RENAME, T("PackageCreator.RemoveFlagDialog.Rename"));

		switch (dialog.ShowModal())
		{
			case KxID_REMOVE:
			{
				TrackRemoveID(entry->GetName());
				RemoveItemAndNotify(*GetDataVector(), item);
				return;
			}
			case KxID_RENAME:
			{
				TrackChangeID(entry->GetName(), entry->GetDeletedName());
				entry->SetName(sDeletedName);
				NotifyChangedItem(item);
				return;
			}
			default:
			{
				NotifyChangedItem(item);
				return;
			}
		};
	}
}
void KPCCFlagsSelectorModel::OnClearList()
{
	for (size_t i = 0; i < GetItemCount(); i++)
	{
		TrackRemoveID(GetDataEntry(i)->GetName());
	}
	NotifyCleared();
}

bool KPCCFlagsSelectorModel::DoTrackID(wxString trackedID, const wxString& newID, bool remove) const
{
	if (IsAssignMode())
	{
		// Manual components
		for (auto& step: GetProject().GetComponents().GetSteps())
		{
			for (auto& group: step->GetGroups())
			{
				for (auto& entry: group->GetEntries())
				{
					TrackID_ReplaceOrRemove(trackedID, newID, entry->GetAssignedFlags(), remove);
					TrackID_ReplaceOrRemove(trackedID, newID, entry->GetTDConditions(), remove);
				}
			}
		}

		// Conditional steps flags
		for (auto& step: GetProject().GetComponents().GetConditionalSteps())
		{
			TrackID_ReplaceOrRemove(trackedID, newID, step->GetConditions(), remove);
		}

		return true;
	}
	return false;
}

void KPCCFlagsSelectorModel::SetDataVector()
{
	KPackageCreatorVectorModel::SetDataVector();
}
void KPCCFlagsSelectorModel::SetDataVector(VectorType& data)
{
	KPackageProjectComponents& components = m_Controller->GetProject()->GetComponents();
	KPackageCreatorVectorModel::SetDataVector(&data);

	m_LabelEditor->SetItems(components.GetFlagsNames());

	KxStringVector flagValues = {"true", "false"};
	for (wxString& value: components.GetFlagsValues())
	{
		flagValues.push_back(std::move(value));
	}
	m_ValueEditor->SetItems(flagValues);
}

//////////////////////////////////////////////////////////////////////////
KPCCFlagsSelectorModelDialog::KPCCFlagsSelectorModelDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller, bool isAssign)
	:KPCCFlagsSelectorModel(controller, isAssign),
	m_WindowOptions("KPCCFlagsSelectorModelDialog", "Window"), m_ViewOptions("KPCCFlagsSelectorModelDialog", "View")
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
		KPCCFlagsSelectorModel::Create(controller, m_ViewPane, m_Sizer);

		AdjustWindow(wxDefaultPosition, wxSize(700, 400));
		KProgramOptionSerializer::LoadDataViewLayout(GetView(), m_ViewOptions);
		KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
	}
}
KPCCFlagsSelectorModelDialog::~KPCCFlagsSelectorModelDialog()
{
	IncRef();

	KProgramOptionSerializer::SaveDataViewLayout(GetView(), m_ViewOptions);
	KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
}

//////////////////////////////////////////////////////////////////////////
void KPCCFlagsTDSelectorModelDialog::OnSelectNewTD(wxCommandEvent& event)
{
	int index = event.GetInt();
	if (index != -1)
	{
		m_Entry->SetTDConditionalValue((KPPCTypeDescriptor)reinterpret_cast<size_t>(m_ComboBoxNewTD->GetClientData(index)));
		ChangeNotify();
	}
}

KPCCFlagsTDSelectorModelDialog::KPCCFlagsTDSelectorModelDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller, KPPCEntry* entry)
	:KPCCFlagsSelectorModelDialog(parent, caption, controller, false), m_Entry(entry),
	m_WindowOptions("KPCCFlagsTDSelectorModelDialog", "Window"), m_ViewOptions("KPCCFlagsTDSelectorModelDialog", "View")
{
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	m_Sizer->Add(sizer, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);

	m_ComboBoxNewTD = KPackageCreatorPageBase::AddControlsRow(sizer, T("PackageCreator.PageComponents.TypeDescriptorConditional"), new KxComboBox(m_ViewPane, KxID_NONE), 1);
	m_ComboBoxNewTD->Bind(wxEVT_COMBOBOX, &KPCCFlagsTDSelectorModelDialog::OnSelectNewTD, this);

	auto AddItem = [this](const wxString& id, KPPCTypeDescriptor nTD)
	{
		int index = m_ComboBoxNewTD->Append(id, (void*)nTD);
		if (nTD == m_Entry->GetTDConditionalValue())
		{
			m_ComboBoxNewTD->SetSelection(index);
		}
	};
	AddItem(KAux::MakeNoneLabel(), KPPC_DESCRIPTOR_INVALID);
	AddItem(T("PackageCreator.PageComponents.TypeDescriptor.Optional"), KPPC_DESCRIPTOR_OPTIONAL);
	AddItem(T("PackageCreator.PageComponents.TypeDescriptor.Required"), KPPC_DESCRIPTOR_REQUIRED);
	AddItem(T("PackageCreator.PageComponents.TypeDescriptor.Recommended"), KPPC_DESCRIPTOR_RECOMMENDED);
	AddItem(T("PackageCreator.PageComponents.TypeDescriptor.CouldBeUsable"), KPPC_DESCRIPTOR_COULD_BE_USABLE);
	AddItem(T("PackageCreator.PageComponents.TypeDescriptor.NotUsable"), KPPC_DESCRIPTOR_NOT_USABLE);

	AdjustWindow(wxDefaultPosition, wxSize(700, 400));
	KProgramOptionSerializer::LoadDataViewLayout(GetView(), m_ViewOptions);
	KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
}
KPCCFlagsTDSelectorModelDialog::~KPCCFlagsTDSelectorModelDialog()
{
	KProgramOptionSerializer::SaveDataViewLayout(GetView(), m_ViewOptions);
	KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
}
