#include "stdafx.h"
#include "KPCCConditionalStepsModel.h"
#include "KPCCFileDataSelectorModel.h"
#include "KPCCConditionGroupEditor.h"
#include "PackageProject/KPackageProject.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "PackageCreator/KPackageCreatorPageComponents.h"
#include "PackageCreator/KPackageCreatorController.h"
#include "UI/KMainWindow.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxDataViewComboBox.h>

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

void KPCCConditionalStepsModel::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KPCCConditionalStepsModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KPCCConditionalStepsModel::OnContextMenu, this);

	GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("PackageCreator.PageComponents.Conditions"), ColumnID::Conditions, KxDATAVIEW_CELL_INERT, 300);
	GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("PackageCreator.PageComponents.FileData"), ColumnID::StepData, KxDATAVIEW_CELL_INERT, 300);
}

void KPCCConditionalStepsModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	auto entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Conditions:
			{
				value = KPackageCreatorPageComponents::ConditionGroupToString(entry->GetConditionGroup());
				break;
			}
			case ColumnID::StepData:
			{
				value = KPackageCreatorPageComponents::FormatArrayToText(entry->GetEntries());
				break;
			}
		};
	}
}
bool KPCCConditionalStepsModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	return false;
}

void KPCCConditionalStepsModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewColumn* column = event.GetColumn();
	if (column)
	{
		KPPCConditionalStep* step = GetDataEntry(GetRow(event.GetItem()));
		switch (column->GetID())
		{
			case ColumnID::Conditions:
			{
				if (step)
				{
					KPCCConditionGroupEditorDialog dialog(KMainWindow::GetInstance(), column->GetTitle(), m_Controller, step->GetConditionGroup());
					dialog.ShowModal();
					NotifyChangedItem(event.GetItem());
				}
				break;
			}
			case ColumnID::StepData:
			{
				if (step)
				{
					KPCCFileDataSelectorModelDialog dialog(KMainWindow::GetInstance(), column->GetTitle(), m_Controller);
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
void KPCCConditionalStepsModel::OnContextMenu(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	const KPPCConditionalStep* entry = GetDataEntry(GetRow(item));

	KxMenu menu;
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddStep, KTr(KxID_ADD)));
		item->SetBitmap(KGetBitmap(KIMG_DIRECTION_PLUS));
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

void KPCCConditionalStepsModel::OnAddStep()
{
	GetDataVector()->emplace_back(new KPPCConditionalStep());

	KxDataViewItem item = GetItem(GetItemCount() - 1);
	NotifyAddedItem(item);
	SelectItem(item);
	GetView()->EditItem(item, GetView()->GetColumn(ColumnID::Conditions));
}
void KPCCConditionalStepsModel::OnRemoveStep(const KxDataViewItem& item)
{
	RemoveItemAndNotify(*GetDataVector(), item);
}
void KPCCConditionalStepsModel::OnClearList()
{
	ClearItemsAndNotify(*GetDataVector());
}

void KPCCConditionalStepsModel::SetDataVector()
{
	KPackageCreatorVectorModel::SetDataVector();
}
void KPCCConditionalStepsModel::SetDataVector(VectorType& data)
{
	KPackageCreatorVectorModel::SetDataVector(&data);
}

//////////////////////////////////////////////////////////////////////////
KPCCConditionalStepsModelDialog::KPCCConditionalStepsModelDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller, bool isAutomatic)
	:KPCCConditionalStepsModel(controller),
	m_WindowOptions("KPCCConditionalStepsModelDialog", "Window"), m_ViewOptions("KPCCConditionalStepsModelDialog", "View")
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
		KPCCConditionalStepsModel::Create(controller, m_ViewPane, sizer);

		AdjustWindow(wxDefaultPosition, wxSize(900, 500));
		KProgramOptionSerializer::LoadDataViewLayout(GetView(), m_ViewOptions);
		KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
	}
}
KPCCConditionalStepsModelDialog::~KPCCConditionalStepsModelDialog()
{
	IncRef();

	KProgramOptionSerializer::SaveDataViewLayout(GetView(), m_ViewOptions);
	KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
}
