#include "stdafx.h"
#include "ConditionFlagsModel.h"
#include "PackageCreator/WorkspaceDocument.h"
#include "PackageCreator/PageBase.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxString.h>
#include <KxFramework/KxComboBox.h>
#include <KxFramework/KxDataViewComboBox.h>
#include <KxFramework/KxTaskDialog.h>

namespace
{
	enum ColumnID
	{
		Name,
		Value,
	};
	enum MenuID
	{
		AddFlag,
	};
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	void ConditionFlagsModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &ConditionFlagsModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &ConditionFlagsModel::OnContextMenu, this);

		// Label
		{
			auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewComboBoxEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE);
			m_LabelEditor = info.GetEditor();
			m_LabelEditor->SetEditable(true);
		}

		// Value
		{
			auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewComboBoxEditor>(KTr("Generic.Value"), ColumnID::Value, KxDATAVIEW_CELL_EDITABLE);
			m_ValueEditor = info.GetEditor();
			m_ValueEditor->SetEditable(true);
		}
	}

	void ConditionFlagsModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
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
			};
		}
	}
	void ConditionFlagsModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
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
			};
		}
	}
	bool ConditionFlagsModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
	{
		auto entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					wxString newName = value.As<wxString>();
					if (newName.StartsWith(PackageProject::RequirementGroup::GetFlagNamePrefix()))
					{
						wxRect rect = GetView()->GetAdjustedItemRect(GetItem(row), column);
						PageBase::ShowTooltipWarning(GetView(), KTr("PackageCreator.InvalidFlagName"), rect);
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
			};
		}
		return false;
	}

	void ConditionFlagsModel::OnActivateItem(KxDataViewEvent& event)
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
	void ConditionFlagsModel::OnContextMenu(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		const PackageProject::FlagItem* entry = GetDataEntry(GetRow(item));

		KxMenu menu;
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddFlag, KTr(KxID_ADD)));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FlagPlus));
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

	void ConditionFlagsModel::OnAddFlag()
	{
		GetDataVector()->emplace_back(wxEmptyString, wxEmptyString);

		KxDataViewItem item = GetItem(GetItemCount() - 1);
		NotifyAddedItem(item);
		SelectItem(item);
		GetView()->EditItem(item, GetView()->GetColumn(ColumnID::Name));
	}
	void ConditionFlagsModel::OnRemoveFlag(const KxDataViewItem& item)
	{
		if (PackageProject::FlagItem* entry = GetDataEntry(GetRow(item)))
		{
			wxString deletedName = entry->GetDeletedName();
			KxTaskDialog dialog(GetViewTLW(), KxID_NONE, KTrf("PackageCreator.Conditions.RemoveFlagDialog.Caption", entry->GetName()), KTrf("PackageCreator.Conditions.RemoveFlagDialog.Message", deletedName), KxBTN_CANCEL, KxICON_WARNING);
			dialog.AddButton(KxID_REMOVE, KTr("PackageCreator.Conditions.RemoveFlagDialog.Remove"));
			dialog.AddButton(KxID_RENAME, KTr("PackageCreator.Conditions.RemoveFlagDialog.RemoveRename"));

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
					entry->SetName(deletedName);
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
	void ConditionFlagsModel::OnClearList()
	{
		for (size_t i = 0; i < GetItemCount(); i++)
		{
			TrackRemoveID(GetDataEntry(i)->GetName());
		}
		NotifyCleared();
	}

	bool ConditionFlagsModel::DoTrackID(wxString trackedID, const wxString& newID, bool remove) const
	{
		// Manual components
		for (auto& step: GetProject().GetComponents().GetSteps())
		{
			for (auto& group: step->GetGroups())
			{
				for (auto& entry: group->GetItems())
				{
					TrackID_ReplaceOrRemove(trackedID, newID, entry->GetConditionFlags().GetFlags(), remove);
					for (PackageProject::Condition& condition : entry->GetTDConditionGroup().GetConditions())
					{
						TrackID_ReplaceOrRemove(trackedID, newID, condition.GetFlags(), remove);
					}
				}
			}
		}

		// Conditional steps flags
		for (auto& step : GetProject().GetComponents().GetConditionalSteps())
		{
			for (PackageProject::Condition& condition: step->GetConditionGroup().GetConditions())
			{
				TrackID_ReplaceOrRemove(trackedID, newID, condition.GetFlags(), remove);
			}
		}

		return true;
	}

	void ConditionFlagsModel::SetDataVector()
	{
		m_Condition = nullptr;
		VectorModel::SetDataVector();
	}
	void ConditionFlagsModel::SetDataVector(PackageProject::Condition& data)
	{
		m_Condition = &data;
		VectorModel::SetDataVector(&m_Condition->GetFlags());

		PackageProject::ComponentsSection& components = m_Controller->GetProject()->GetComponents();
		m_LabelEditor->SetItems(components.GetFlagsNames());
		m_ValueEditor->SetItems(components.GetFlagsValues());
	}
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	ConditionFlagsDialog::ConditionFlagsDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller)
		:ConditionFlagsModel(controller)
		//m_WindowOptions("ConditionFlagsDialog", "Window"), m_ViewOptions("ConditionGroupDialog", "View")
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
			ConditionFlagsModel::Create(controller, m_ViewPane, m_Sizer);

			AdjustWindow(wxDefaultPosition, FromDIP(wxSize(700, 400)));
			//KProgramOptionSerializer::LoadDataViewLayout(GetView(), m_ViewOptions);
			//KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
		}
	}
	ConditionFlagsDialog::~ConditionFlagsDialog()
	{
		IncRef();

		//KProgramOptionSerializer::SaveDataViewLayout(GetView(), m_ViewOptions);
		//KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
	}
}
