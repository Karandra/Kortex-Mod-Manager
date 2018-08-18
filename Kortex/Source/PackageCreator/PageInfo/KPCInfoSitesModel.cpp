#include "stdafx.h"
#include "KPCInfoSitesModel.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "UI/KMainWindow.h"
#include "UI/KTextEditorDialog.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxFileBrowseDialog.h>

enum ColumnID
{
	Name,
	Value,
};
enum MenuID
{
	AddSite,
};

void KPCInfoSitesModel::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KPCInfoSitesModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KPCInfoSitesModel::OnContextMenu, this);

	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 200);
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("Generic.Address"), ColumnID::Value, m_UseInlineEditor ? KxDATAVIEW_CELL_EDITABLE : KxDATAVIEW_CELL_INERT, 300);
}

void KPCInfoSitesModel::GetEditorValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
{
	const KLabeledValue* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				data = entry->GetLabelRaw();
				return;
			}
			case ColumnID::Value:
			{
				data = entry->GetValue();
				return;
			}
		};
	}
}
void KPCInfoSitesModel::GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
{
	const KLabeledValue* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				data = entry->HasLabel() ? entry->GetLabelRaw() : KAux::ExtractDomainName(entry->GetValue());
				break;
			}
			case ColumnID::Value:
			{
				data = entry->GetValue();
				break;
			}
		};
	}
}
bool KPCInfoSitesModel::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
{
	KLabeledValue* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				entry->SetLabel(data.As<wxString>());
				ChangeNotify();
				return true;
			}
			case ColumnID::Value:
			{
				entry->SetValue(data.As<wxString>());
				ChangeNotify();
				return true;
			}
		};
	}
	return false;
}

void KPCInfoSitesModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	if (column)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				GetView()->EditItem(item, column);
				break;
			}
			case ColumnID::Value:
			{
				if (m_UseInlineEditor)
				{
					GetView()->EditItem(item, column);
				}
				else
				{
					if (KLabeledValue* entry = GetDataEntry(GetRow(item)))
					{
						KTextEditorDialog dialog(GetView());
						dialog.SetText(entry->GetValue());
						if (dialog.ShowModal() == KxID_OK && dialog.IsModified())
						{
							entry->SetValue(dialog.GetText());
							NotifyChangedItem(item);
						}
					}
				}
				break;
			}
		};
	}
}
void KPCInfoSitesModel::OnContextMenu(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	const KLabeledValue* entry = GetDataEntry(GetRow(item));

	KxMenu menu;
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddSite, T(KxID_ADD)));
		item->SetBitmap(KGetBitmap(KIMG_PLUS_SMALL));
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
		case MenuID::AddSite:
		{
			OnAddSite();
			break;
		}
		case KxID_REMOVE:
		{
			OnRemoveSite(item);
			break;
		}
		case KxID_CLEAR:
		{
			OnClearList();
			break;
		}
	};
};

void KPCInfoSitesModel::OnAddSite()
{
	GetDataVector()->emplace_back(KLabeledValue(wxEmptyString));

	KxDataViewItem item = GetItem(GetItemCount() - 1);
	NotifyAddedItem(item);
	SelectItem(item);
	GetView()->EditItem(item, GetView()->GetColumn(ColumnID::Name));
}
void KPCInfoSitesModel::OnRemoveSite(const KxDataViewItem& item)
{
	RemoveItemAndNotify(*GetDataVector(), item);
}
void KPCInfoSitesModel::OnClearList()
{
	ClearItemsAndNotify(*GetDataVector());
}

void KPCInfoSitesModel::SetDataVector()
{
	m_InfoData = NULL;
	KPackageCreatorVectorModel::SetDataVector();
}
void KPCInfoSitesModel::SetDataVector(VectorType& data, KPackageProjectInfo* info)
{
	m_InfoData = info;
	KPackageCreatorVectorModel::SetDataVector(&data);
}

//////////////////////////////////////////////////////////////////////////
KPCInfoSitesModelDialog::KPCInfoSitesModelDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller, bool bUseInlineEditor)
	:m_WindowOptions("KPCInfoSitesModelDialog", "Window"), m_ViewOptions("KPCInfoSitesModelDialog", "View")
{
	UseInlineEditor(bUseInlineEditor);

	if (KxStdDialog::Create(parent, KxID_NONE, caption, wxDefaultPosition, wxDefaultSize, KxBTN_OK))
	{
		SetMainIcon(KxICON_NONE);
		SetWindowResizeSide(wxBOTH);

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
		m_ViewPane->SetSizer(sizer);
		PostCreate();

		// List
		IncRef();
		KPCInfoSitesModel::Create(controller, m_ViewPane, sizer);

		AdjustWindow(wxDefaultPosition, wxSize(700, 400));
		KProgramOptionSerializer::LoadDataViewLayout(GetView(), m_ViewOptions);
		KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
	}
}
KPCInfoSitesModelDialog::~KPCInfoSitesModelDialog()
{
	IncRef();

	KProgramOptionSerializer::SaveDataViewLayout(GetView(), m_ViewOptions);
	KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
}
