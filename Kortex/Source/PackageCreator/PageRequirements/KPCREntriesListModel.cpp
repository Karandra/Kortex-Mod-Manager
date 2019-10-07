#include "stdafx.h"
#include <Kortex/PluginManager.hpp>
#include "KPCREntriesListModel.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "PackageProject/KPackageProject.h"
#include "ModPackages/IPackageManager.h"
#include "UI/TextEditDialog.h"
#include "UI/ImageViewerDialog.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxTextBoxDialog.h>

namespace
{
	enum ColumnID
	{
		Type,
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
}

namespace Kortex::PackageDesigner
{
	void KPCREntriesListModel::OnInitControl()
	{
		/* View */
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KPCREntriesListModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_EDIT_STARTING, &KPCREntriesListModel::OnStartEditItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KPCREntriesListModel::OnContextMenuItem, this);
	
		/* Columns */
		// Type
		{
			auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewComboBoxEditor>(KTr("PackageCreator.PageRequirements.Type"), ColumnID::Type, KxDATAVIEW_CELL_EDITABLE, 115);
			m_TypeEditor = info.GetEditor();
			m_TypeEditor->SetItems
			({
				KTr("PackageCreator.PageRequirements.Type.User"),
				KTr("PackageCreator.PageRequirements.Type.System"),
				KTr("PackageCreator.PageRequirements.Type.Auto")
			 });
			
		}
	
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("Generic.ID"), ColumnID::ID, KxDATAVIEW_CELL_EDITABLE, 175);
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 235);
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("PackageCreator.PageRequirements.RequiredVersion") + " (RV)", ColumnID::RequiredVersion, KxDATAVIEW_CELL_EDITABLE, 150);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("PackageCreator.PageRequirements.CurrentVersion") + " (CV)", ColumnID::CurrentVersion, KxDATAVIEW_CELL_INERT, 200);
	
		// Object function
		{
			auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewComboBoxEditor>(KTr("PackageCreator.PageRequirements.RequiredState"), ColumnID::RequiredState, KxDATAVIEW_CELL_EDITABLE, 150);
			m_ObjectFunctionEditor = info.GetEditor();
	
			KxStringVector choices;
			choices.push_back(KAux::MakeNoneLabel());
			choices.push_back(KTr("PackageCreator.PageRequirements.RequiredState.ModActive"));
			choices.push_back(KTr("PackageCreator.PageRequirements.RequiredState.ModInactive"));
			choices.push_back(KTr("PackageCreator.PageRequirements.RequiredState.FileExist"));
			choices.push_back(KTr("PackageCreator.PageRequirements.RequiredState.FileNotExist"));
	
			if (IPluginManager::GetInstance())
			{
				choices.push_back(KTr("PackageCreator.PageRequirements.RequiredState.PluginActive"));
				choices.push_back(KTr("PackageCreator.PageRequirements.RequiredState.PluginInactive"));
			}
			m_ObjectFunctionEditor->SetItems(choices);
		}
	
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("PackageCreator.PageRequirements.Object"), ColumnID::Object, KxDATAVIEW_CELL_EDITABLE, 400);
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("Generic.Description"), ColumnID::Description, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE);
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
				case ColumnID::ID:
				{
					value = entry->IsEmptyID() ? KAux::MakeNoneLabel() : entry->GetID();
					break;
				}
				case ColumnID::Name:
				{
					value = entry->IsEmptyName() ? KAux::MakeNoneLabel() : entry->GetName();
					break;
				}
				case ColumnID::RequiredVersion:
				{
					value = entry->GetRequiredVersion().IsOK() ? entry->GetRequiredVersion().ToString() : KAux::MakeNoneLabel();
					break;
				}
				case ColumnID::CurrentVersion:
				{
					wxString operatorName = KPackageProject::OperatorToSymbolicName(entry->GetRVFunction());
					wxString cv = entry->GetCurrentVersion();
					wxString rv = entry->GetRequiredVersion();
	
					value = KxString::Format("CV(%1) %2 RV(%3) %4 %5", cv, operatorName, rv, KAux::GetUnicodeChar(KAUX_CHAR_ARROW_RIGHT), entry->CheckVersion());
					break;
				}
				case ColumnID::RequiredState:
				{
					KxFormat format("%1 %2 %3");
					format(m_ObjectFunctionEditor->GetItems()[entry->GetObjectFunction()]);
					format(KAux::GetUnicodeChar(KAUX_CHAR_ARROW_RIGHT));
					format(entry->GetObjectFunctionResult() == KPPReqState::True);
	
					value = format.ToString();
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
					entry->ResetCurrentVersion();
					entry->ResetObjectFunctionResult();
	
					entry->TrySetTypeDescriptor(data.As<KPPRTypeDescriptor>());
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
							entry->ResetCurrentVersion();
							entry->ResetObjectFunctionResult();
	
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
					entry->ResetCurrentVersion();
					entry->ResetObjectFunctionResult();
	
					entry->SetName(data.As<wxString>());
					entry->ConformToTypeDescriptor();
					ChangeNotify();
					break;
				}
				case ColumnID::RequiredVersion:
				{
					entry->ResetObjectFunctionResult();
	
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
					entry->ResetCurrentVersion();
					entry->ResetObjectFunctionResult();
	
					entry->SetObject(data.As<wxString>());
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
							KxMenuItem* item = menu.Add(new KxMenuItem(i, KPackageProject::OperatorToSymbolicName((KPPOperator)i), wxEmptyString, wxITEM_CHECK));
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
						UI::TextEditDialog dialog(GetView());
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
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddEntry, KTr(KxID_ADD)));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::ChequePlus));
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
				UI::TextEditDialog dialog(GetView());
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
}
