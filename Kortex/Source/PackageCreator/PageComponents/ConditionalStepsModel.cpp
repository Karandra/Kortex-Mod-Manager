#include "stdafx.h"
#include "ConditionalStepsModel.h"
#include "FileDataSelectorModel.h"
#include "ConditionGroupModel.h"
#include "PackageProject/KPackageProject.h"
#include "PackageCreator/PageBase.h"
#include "PackageCreator/PageComponents.h"
#include "PackageCreator/WorkspaceDocument.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxDataViewComboBox.h>

namespace
{
	enum ColumnID
	{
		Conditions,
		StepData
	};
	enum MenuID
	{
		AddStep,
		AddEntry,
	};
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	void ConditionalStepsModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &ConditionalStepsModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &ConditionalStepsModel::OnContextMenu, this);
	
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("PackageCreator.PageComponents.Conditions"), ColumnID::Conditions, KxDATAVIEW_CELL_INERT, 300);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("PackageCreator.PageComponents.FileData"), ColumnID::StepData, KxDATAVIEW_CELL_INERT, 300);
	}
	
	void ConditionalStepsModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		auto entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Conditions:
				{
					value = PageComponents::ConditionGroupToString(entry->GetConditionGroup());
					break;
				}
				case ColumnID::StepData:
				{
					value = PageComponents::FormatArrayToText(entry->GetEntries());
					break;
				}
			};
		}
	}
	bool ConditionalStepsModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
	{
		return false;
	}
	
	void ConditionalStepsModel::OnActivateItem(KxDataViewEvent& event)
	{
		KxDataViewColumn* column = event.GetColumn();
		if (column)
		{
			PackageProject::ConditionalComponentStep* step = GetDataEntry(GetRow(event.GetItem()));
			switch (column->GetID())
			{
				case ColumnID::Conditions:
				{
					if (step)
					{
						ConditionGroupDialog dialog(GetView(), column->GetTitle(), m_Controller, step->GetConditionGroup());
						dialog.ShowModal();
						NotifyChangedItem(event.GetItem());
					}
					break;
				}
				case ColumnID::StepData:
				{
					if (step)
					{
						FileDataSelectorDialog dialog(GetView(), column->GetTitle(), m_Controller);
						dialog.SetDataVector(step->GetEntries(), &m_Controller->GetProject()->GetFileData());
						if (dialog.ShowModal() == KxID_OK)
						{
							step->GetEntries() = dialog.GetSelectedItems();
							NotifyChangedItem(event.GetItem());
						}
					}
					break;
				}
			};
		}
	}
	void ConditionalStepsModel::OnContextMenu(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		const PackageProject::ConditionalComponentStep* entry = GetDataEntry(GetRow(item));
	
		KxMenu menu;
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddStep, KTr(KxID_ADD)));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::DirectionPlus));
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
			case MenuID::AddStep:
			{
				OnAddStep();
				break;
			}
			case KxID_REMOVE:
			{
				OnRemoveStep(item);
				break;
			}
			case KxID_CLEAR:
			{
				OnClearList();
				break;
			}
		};
	};
	
	void ConditionalStepsModel::OnAddStep()
	{
		GetDataVector()->emplace_back(new PackageProject::ConditionalComponentStep());
	
		KxDataViewItem item = GetItem(GetItemCount() - 1);
		NotifyAddedItem(item);
		SelectItem(item);
		GetView()->EditItem(item, GetView()->GetColumn(ColumnID::Conditions));
	}
	void ConditionalStepsModel::OnRemoveStep(const KxDataViewItem& item)
	{
		RemoveItemAndNotify(*GetDataVector(), item);
	}
	void ConditionalStepsModel::OnClearList()
	{
		ClearItemsAndNotify(*GetDataVector());
	}
	
	void ConditionalStepsModel::SetDataVector()
	{
		VectorModel::SetDataVector();
	}
	void ConditionalStepsModel::SetDataVector(VectorType& data)
	{
		VectorModel::SetDataVector(&data);
	}
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	ConditionalStepsDialog::ConditionalStepsDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller, bool isAutomatic)
		:ConditionalStepsModel(controller)
		//m_WindowOptions("ConditionalStepsDialog", "Window"), m_ViewOptions("ConditionalStepsDialog", "View")
	{
		if (KxStdDialog::Create(parent, KxID_NONE, caption, wxDefaultPosition, wxDefaultSize, KxBTN_OK))
		{
			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide(wxBOTH);
	
			wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
			m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
			m_ViewPane->SetSizer(sizer);
			PostCreate();
	
			// List
			ConditionalStepsModel::Create(controller, m_ViewPane, sizer);
	
			AdjustWindow(wxDefaultPosition, FromDIP(wxSize(900, 500)));
			//KProgramOptionSerializer::LoadDataViewLayout(GetView(), m_ViewOptions);
			//KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
		}
	}
	ConditionalStepsDialog::~ConditionalStepsDialog()
	{
		IncRef();
	
		//KProgramOptionSerializer::SaveDataViewLayout(GetView(), m_ViewOptions);
		//KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
	}
}
