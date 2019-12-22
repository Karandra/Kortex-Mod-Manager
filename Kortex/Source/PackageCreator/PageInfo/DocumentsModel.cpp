#include "stdafx.h"
#include "DocumentsModel.h"
#include "PackageCreator/PageBase.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxString.h>
#include <KxFramework/KxFileBrowseDialog.h>

namespace
{
	enum ColumnID
	{
		Name,
		Value,
	};
	enum MenuID
	{
		AddEntry,
	};
}

namespace Kortex::PackageDesigner::PageInfoNS
{
	void DocumentsModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &DocumentsModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &DocumentsModel::OnContextMenu, this);
	
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 200);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr(KxID_FILE), ColumnID::Value, KxDATAVIEW_CELL_INERT, 200);
	}
	
	void DocumentsModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		auto entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = entry->GetRawLabel();
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
	bool DocumentsModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
	{
		auto entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					entry->SetLabel(value.As<wxString>());
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
	
	void DocumentsModel::OnActivateItem(KxDataViewEvent& event)
	{
		KxDataViewColumn* column = event.GetColumn();
		if (column)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					GetView()->EditItem(event.GetItem(), column);
					break;
				}
				case ColumnID::Value:
				{
					auto entry = GetDataEntry(GetRow(event.GetItem()));
					if (entry)
					{
						KxStringVector files = OpenFileDialog(false);
						if (!files.empty())
						{
							entry->SetValue(files.front());
							ChangeNotify();
						}
					}
					break;
				}
			};
		}
	}
	void DocumentsModel::OnContextMenu(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		const Utility::LabeledValue* entry = GetDataEntry(GetRow(item));
	
		KxMenu menu;
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddEntry, KTr(KxID_ADD)));
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
	};
	
	void DocumentsModel::OnAddEntry()
	{
		KxDataViewItem item;
		KxStringVector files = OpenFileDialog();
	
		for (const wxString& value: files)
		{
			if (!value.IsEmpty())
			{
				GetDataVector()->emplace_back(Utility::LabeledValue(value, value.AfterLast('\\')));
				item = GetItem(GetItemCount() - 1);
				NotifyAddedItem(item);
			}
		}
	
		SelectItem(item);
		if (files.size() == 1)
		{
			GetView()->EditItem(item, GetView()->GetColumn(ColumnID::Name));
		}
	}
	void DocumentsModel::OnRemoveEntry(const KxDataViewItem& item)
	{
		RemoveItemAndNotify(*GetDataVector(), item);
	}
	void DocumentsModel::OnClearList()
	{
		ClearItemsAndNotify(*GetDataVector());
	}
	KxStringVector DocumentsModel::OpenFileDialog(bool isMultiple) const
	{
		KxFileBrowseDialog dialog(GetViewTLW(), KxID_NONE, KxFBD_OPEN);
		dialog.AddFilter("*.txt;*.ini;*.xml;*.htm;*.html;*.mht;*.mhtml;*.pdf", KTr("FileFilter.AllSupportedFormats"));
		dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
		dialog.SetOptionEnabled(KxFBD_ALLOW_MULTISELECT, isMultiple);
	
		dialog.ShowModal();
		return dialog.GetResults();
	}
	
	void DocumentsModel::SetDataVector()
	{
		m_InfoData = nullptr;
		VectorModel::SetDataVector();
	}
	void DocumentsModel::SetDataVector(VectorType& data, PackageProject::InfoSection* info)
	{
		m_InfoData = info;
		VectorModel::SetDataVector(&data);
	}
}

namespace Kortex::PackageDesigner::PageInfoNS
{
	DocumentsDialog::DocumentsDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller)
		//:m_WindowOptions("DocumentsDialog", "Window"), m_ViewOptions("DocumentsDialog", "View")
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
			DocumentsModel::Create(controller, m_ViewPane, sizer);
	
			AdjustWindow(wxDefaultPosition, FromDIP(wxSize(700, 400)));
			//KProgramOptionSerializer::LoadDataViewLayout(GetView(), m_ViewOptions);
			//KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
		}
	}
	DocumentsDialog::~DocumentsDialog()
	{
		IncRef();
	
		//KProgramOptionSerializer::SaveDataViewLayout(GetView(), m_ViewOptions);
		//KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
	}
}
