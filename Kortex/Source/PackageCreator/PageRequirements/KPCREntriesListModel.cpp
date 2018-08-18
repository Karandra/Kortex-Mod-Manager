#include "stdafx.h"
#include "KPCREntriesListModel.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "PackageProject/KPackageProject.h"
#include "PackageManager/KPackageManager.h"
#include "PluginManager/KPluginManager.h"
#include "UI/KMainWindow.h"
#include "UI/KTextEditorDialog.h"
#include "UI/KImageViewerDialog.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxTextBoxDialog.h>

enum ColumnID
{
	Type,
	Operator,
	ID,
	Name,
	RequiredVersion,
	CurrentVersion,
	RequiredState,
	Object,
	Description,
};
enum MenuID
{
	AddEntry,
};

void KPCREntriesListModel::OnInitControl()
{
	/* View */
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KPCREntriesListModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_EDIT_STARTING, &KPCREntriesListModel::OnStartEditItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KPCREntriesListModel::OnContextMenuItem, this);

	/* Columns */
	// Type
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewComboBoxEditor>(T("PackageCreator.PageRequirements.Type"), ColumnID::Type, KxDATAVIEW_CELL_EDITABLE, 115);
		m_TypeEditor = info.GetEditor();
		m_TypeEditor->SetItems
		({
			T("PackageCreator.PageRequirements.Type.User"),
			T("PackageCreator.PageRequirements.Type.System"),
			T("PackageCreator.PageRequirements.Type.Auto")
		 });
		
	}

	// Operator
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewComboBoxEditor>(T("PackageCreator.Operator"), ColumnID::Operator, KxDATAVIEW_CELL_EDITABLE);
		m_OperatorEditor = info.GetEditor();
		m_OperatorEditor->SetItems
		({
			KPackageProjectRequirements::OperatorToSymbolicName(KPP_OPERATOR_AND),
			KPackageProjectRequirements::OperatorToSymbolicName(KPP_OPERATOR_OR)
		 });
	}

	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("Generic.ID"), ColumnID::ID, KxDATAVIEW_CELL_EDITABLE, 175);
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 235);
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("PackageCreator.PageRequirements.RequiredVersion") + " (RV)", ColumnID::RequiredVersion, KxDATAVIEW_CELL_EDITABLE, 150);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("PackageCreator.PageRequirements.CurrentVersion") + " (CV)", ColumnID::CurrentVersion, KxDATAVIEW_CELL_INERT, 200);

	// Object function
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewComboBoxEditor>(T("PackageCreator.PageRequirements.RequiredState"), ColumnID::RequiredState, KxDATAVIEW_CELL_EDITABLE, 150);
		m_ObjectFunctionEditor = info.GetEditor();

		KxStringVector tChoices;
		tChoices.push_back(KAux::MakeNoneLabel());
		tChoices.push_back(T("PackageCreator.PageRequirements.RequiredState.ModActive"));
		tChoices.push_back(T("PackageCreator.PageRequirements.RequiredState.ModInactive"));
		tChoices.push_back(T("PackageCreator.PageRequirements.RequiredState.FileExist"));
		tChoices.push_back(T("PackageCreator.PageRequirements.RequiredState.FileNotExist"));

		if (KPluginManager::HasInstance())
		{
			tChoices.push_back(T("PackageCreator.PageRequirements.RequiredState.PluginActive"));
			tChoices.push_back(T("PackageCreator.PageRequirements.RequiredState.PluginInactive"));
		}
		m_ObjectFunctionEditor->SetItems(tChoices);
	}

	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("PackageCreator.PageRequirements.Object"), ColumnID::Object, KxDATAVIEW_CELL_EDITABLE, 400);
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("Generic.Description"), ColumnID::Description, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE);
}

void KPCREntriesListModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KPPRRequirementEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Type:
			{
				value = entry->GetTypeDescriptor();
				return;
			}
			case ColumnID::Operator:
			{
				value = entry->GetOperator() == KPP_OPERATOR_AND ? 0 : 1;
				return;
			}
			case ColumnID::ID:
			{
				value = entry->RawGetID();
				return;
			}
			case ColumnID::Name:
			{
				value = entry->RawGetName();
				return;
			}
			case ColumnID::RequiredVersion:
			{
				value = entry->GetRequiredVersion().ToString();
				return;
			}
			case ColumnID::RequiredState:
			{
				value = entry->GetObjectFunction();
				return;
			}
			case ColumnID::Object:
			{
				value = entry->GetObject();
				return;
			}
			case ColumnID::Description:
			{
				value = entry->GetDescription();
				return;
			}
		};
	}
}
void KPCREntriesListModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KPPRRequirementEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Type:
			{
				value = m_TypeEditor->GetItems()[entry->GetTypeDescriptor()];
				break;
			}
			case ColumnID::Operator:
			{
				value = m_OperatorEditor->GetItems()[entry->GetOperator() == KPP_OPERATOR_AND ? 0 : 1];
				break;
			}
			case ColumnID::ID:
			{
				value = entry->GetID();
				break;
			}
			case ColumnID::Name:
			{
				value = entry->GetName();
				break;
			}
			case ColumnID::RequiredVersion:
			{
				value = entry->GetRequiredVersion().IsOK() ? entry->GetRequiredVersion().ToString() : KAux::MakeNoneLabel();
				break;
			}
			case ColumnID::CurrentVersion:
			{
				wxString operatorName = KPackageProjectRequirements::OperatorToSymbolicName(entry->GetRVFunction());
				wxString CV = entry->GetCurrentVersion().ToString();
				wxString RV = entry->GetRequiredVersion().ToString();

				value = wxString::Format("CV(%s) %s RV(%s) %c %s", CV, operatorName, RV, KAux::GetUnicodeChar(KAUX_CHAR_ARROW_RIGHT), entry->CheckVersion() ? "true" : "false");
				break;
			}
			case ColumnID::RequiredState:
			{
				value = wxString::Format("%s %c %s", m_ObjectFunctionEditor->GetItems()[entry->GetObjectFunction()], KAux::GetUnicodeChar(KAUX_CHAR_ARROW_RIGHT), entry->GetObjectFunctionResult() ? "true" : "false");
				break;
			}
			case ColumnID::Object:
			{
				value = entry->GetObject().IsEmpty() ? KAux::MakeNoneLabel() : entry->GetObject();
				break;
			}
			case ColumnID::Description:
			{
				value = entry->GetDescription().IsEmpty() ? KAux::MakeNoneLabel() : entry->GetDescription();
				break;
			}
		};
	}
}
bool KPCREntriesListModel::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
{
	KPPRRequirementEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Type:
			{
				entry->TrySetTypeDescriptor(data.As<KPPRTypeDescriptor>());
				ChangeNotify();
				break;
			}
			case ColumnID::Operator:
			{
				entry->SetOperator(data.As<int>() == 0 ? KPP_OPERATOR_AND : KPP_OPERATOR_OR);
				ChangeNotify();
				break;
			}
			case ColumnID::ID:
			{
				wxString newID = data.As<wxString>();
				if (newID != entry->GetID())
				{
					if (newID.IsEmpty() || !m_Group->HasEntryWithID(newID))
					{
						entry->SetID(newID);
						entry->ConformToTypeDescriptor();
						ChangeNotify();
					}
					else
					{
						KPackageCreatorPageBase::WarnIDCollision(GetView(), GetView()->GetAdjustedItemRect(GetItem(row), column));
						return false;
					}
				}
				break;
			}
			case ColumnID::Name:
			{
				entry->SetName(data.As<wxString>());
				entry->ConformToTypeDescriptor();
				ChangeNotify();
				break;
			}
			case ColumnID::RequiredVersion:
			{
				entry->SetRequiredVersion(data.As<wxString>());
				ChangeNotify();
				break;
			}
			case ColumnID::RequiredState:
			{
				entry->ResetObjectFunctionResult();
				entry->SetObjectFunction(data.As<KPPRObjectFunction>());
				ChangeNotify();
				break;
			}
			case ColumnID::Object:
			{
				entry->SetObject(data.As<wxString>());
				entry->ResetCurrentVersion();
				entry->ResetObjectFunctionResult();
				ChangeNotify();
				break;
			}
		};
	}
	return true;
}
bool KPCREntriesListModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	switch (column->GetID())
	{
		case ColumnID::ID:
		case ColumnID::Name:
		case ColumnID::RequiredState:
		case ColumnID::Object:
		{
			if (KPPRRequirementEntry* entry = GetDataEntry(row))
			{
				return entry->IsUserEditable();
			}
			break;
		}
	};
	return true;
}
bool KPCREntriesListModel::GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
{
	const KPPRRequirementEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::CurrentVersion:
			{
				attributes.SetFontFace("Consolas");
				break;
			}
			case ColumnID::RequiredVersion:
			{
				if (entry->GetRequiredVersion().IsOK())
				{
					attributes.SetFontFace("Consolas");
				}
				break;
			}
		}
	};

	if (!IsEnabledByRow(row, column))
	{
		attributes.SetEnabled(false);
	}
	return !attributes.IsDefault();
}

void KPCREntriesListModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	KPPRRequirementEntry* entry = GetDataEntry(GetRow(event.GetItem()));

	if (column)
	{
		switch (column->GetID())
		{
			case ColumnID::Type:
			case ColumnID::Operator:
			case ColumnID::RequiredVersion:
			{
				GetView()->EditItem(item, column);
				break;
			}
			case ColumnID::CurrentVersion:
			{
				if (entry)
				{
					KxMenu menu;
					for (int i = KPP_OPERATOR_MIN; i < KPP_OPERATOR_COUNT_COMPARISON; i++)
					{
						KxMenuItem* item = menu.Add(new KxMenuItem(i, KPackageProjectRequirements::OperatorToSymbolicName((KPPOperator)i), wxEmptyString, wxITEM_CHECK));
						item->Check(i == entry->GetRVFunction());
					}

					wxWindowID id = menu.Show(GetView(), GetView()->GetDropdownMenuPosition(item, column) + wxPoint(0, 1));
					if (id != KxID_NONE)
					{
						entry->SetRVFunction((KPPOperator)id);
						NotifyChangedItem(item);
					}
				}
				break;
			}
			case ColumnID::ID:
			case ColumnID::Name:
			case ColumnID::RequiredState:
			case ColumnID::Object:
			{
				if (entry && entry->IsUserEditable())
				{
					GetView()->EditItem(item, column);
				}
				else
				{
					wxBell();
				}
				break;
			}
			case ColumnID::Description:
			{
				if (entry)
				{
					KTextEditorDialog dialog(GetView());
					dialog.SetText(entry->GetDescription());
					if (dialog.ShowModal() == KxID_OK && dialog.IsModified())
					{
						entry->SetDescription(dialog.GetText());
						NotifyChangedItem(item);
					}
				}
				break;
			}
		};
	}
}
void KPCREntriesListModel::OnStartEditItem(KxDataViewEvent& event)
{
	switch (event.GetColumn()->GetID())
	{
		case ColumnID::ID:
		case ColumnID::Name:
		case ColumnID::RequiredState:
		case ColumnID::Object:
		{
			KPPRRequirementEntry* entry = GetDataEntry(GetRow(event.GetItem()));
			if (entry && !entry->IsUserEditable())
			{
				event.Veto();
				wxBell();
			}
			break;
		}
	};
}
void KPCREntriesListModel::OnContextMenuItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	const KPPRRequirementEntry* entry = GetDataEntry(GetRow(item));

	KxMenu menu;
	{
		KxMenu* allItems = CreateAllItemsMenu(menu);
		CreateAllItemsMenuEntry(allItems, ColumnID::Name);
		CreateAllItemsMenuEntry(allItems, ColumnID::RequiredVersion);
		CreateAllItemsMenuEntry(allItems, ColumnID::Object);
		CreateAllItemsMenuEntry(allItems, ColumnID::Description);
		menu.AddSeparator();
	}

	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddEntry, T(KxID_ADD)));
		item->SetBitmap(KGetBitmap(KIMG_CHEQUE_PLUS));
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
		case MenuID::AddEntry:
		{
			OnAddEntry();
			break;
		}
		case KxID_REMOVE:
		{
			OnRemoveEntry(item);
			break;
		}
		case KxID_CLEAR:
		{
			OnClearList();
			break;
		}
	};
}
void KPCREntriesListModel::OnAllItemsMenuSelect(KxDataViewColumn* column)
{
	switch (column->GetID())
	{
		case ColumnID::Name:
		case ColumnID::RequiredVersion:
		case ColumnID::Object:
		{
			KxTextBoxDialog dialog(GetView(), KxID_NONE, column->GetTitle());
			if (dialog.ShowModal() == KxID_OK)
			{
				for (auto& entry: *GetDataVector())
				{
					if (column->GetID() == ColumnID::Name)
					{
						if (entry->IsUserEditable())
						{
							entry->SetName(dialog.GetValue());
						}
					}
					else if (column->GetID() == ColumnID::RequiredVersion)
					{
						entry->SetRequiredVersion(dialog.GetValue());
					}
					else if (column->GetID() == ColumnID::Object)
					{
						if (entry->IsUserEditable())
						{
							entry->SetObject(dialog.GetValue());
						}
					}
				}
				NotifyAllItemsChanged();
			}
			break;
		}
		case ColumnID::Description:
		{
			KTextEditorDialog dialog(GetView());
			if (dialog.ShowModal() == KxID_OK)
			{
				for (auto& entry: *GetDataVector())
				{
					entry->SetDescription(dialog.GetText());
				}
				NotifyAllItemsChanged();
			}
			break;
		}
	};
}

void KPCREntriesListModel::OnAddEntry()
{
	GetDataVector()->emplace_back(new KPPRRequirementEntry(KPPR_TYPE_USER));

	KxDataViewItem item = GetItem(GetItemCount() - 1);
	NotifyAddedItem(item);
	SelectItem(item);
	GetView()->EditItem(item, GetView()->GetColumn(ColumnID::ID));
}
void KPCREntriesListModel::OnRemoveEntry(const KxDataViewItem& item)
{
	RemoveItemAndNotify(*GetDataVector(), item);
}
void KPCREntriesListModel::OnClearList()
{
	ClearItemsAndNotify(*GetDataVector());
}

void KPCREntriesListModel::SetProject(KPackageProject& projectData)
{
	m_Requirements = &projectData.GetRequirements();
}
