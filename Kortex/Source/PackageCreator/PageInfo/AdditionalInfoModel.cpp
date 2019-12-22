#include "stdafx.h"
#include "AdditionalInfoModel.h"
#include "PackageCreator/PageBase.h"
#include "UI/TextEditDialog.h"
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
	void AdditionalInfoModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &AdditionalInfoModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &AdditionalInfoModel::OnContextMenu, this);
	
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 300);
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("Generic.Value"), ColumnID::Value, m_UseInlineEditor ? KxDATAVIEW_CELL_EDITABLE : KxDATAVIEW_CELL_INERT, 300);
	}
	
	void AdditionalInfoModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
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
	bool AdditionalInfoModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
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
	
	void AdditionalInfoModel::OnActivateItem(KxDataViewEvent& event)
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
					if (m_UseInlineEditor)
					{
						GetView()->EditItem(event.GetItem(), column);
					}
					else
					{
						Utility::LabeledValue* entry = GetDataEntry(GetRow(event.GetItem()));
						if (entry)
						{
							UI::TextEditDialog dialog(GetView());
							dialog.SetText(entry->GetValue());
							if (dialog.ShowModal() == KxID_OK && dialog.IsModified())
							{
								entry->SetValue(dialog.GetText());
								NotifyChangedItem(event.GetItem());
							}
						}
					}
					break;
				}
			};
		}
	}
	void AdditionalInfoModel::OnContextMenu(KxDataViewEvent& event)
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
	
	void AdditionalInfoModel::OnAddEntry()
	{
		GetDataVector()->emplace_back(Utility::LabeledValue(wxEmptyString));
	
		KxDataViewItem item = GetItem(GetItemCount() - 1);
		NotifyAddedItem(item);
		SelectItem(item);
		GetView()->EditItem(item, GetView()->GetColumn(ColumnID::Name));
	}
	void AdditionalInfoModel::OnRemoveEntry(const KxDataViewItem& item)
	{
		RemoveItemAndNotify(*GetDataVector(), item);
	}
	void AdditionalInfoModel::OnClearList()
	{
		ClearItemsAndNotify(*GetDataVector());
	}
	
	void AdditionalInfoModel::SetDataVector()
	{
		m_InfoData = nullptr;
		VectorModel::SetDataVector();
	}
	void AdditionalInfoModel::SetDataVector(VectorType& data, PackageProject::InfoSection* info)
	{
		m_InfoData = info;
		VectorModel::SetDataVector(&data);
	}
}

namespace Kortex::PackageDesigner::PageInfoNS
{
	AdditionalInfoDialog::AdditionalInfoDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller, bool bUseInlineEditor)
		//:m_WindowOptions("KPCInfoAdditionalInfoModelDialog", "Window"), m_ViewOptions("KPCInfoAdditionalInfoModelDialog", "View")
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
			AdditionalInfoModel::Create(controller, m_ViewPane, sizer);
			
			AdjustWindow(wxDefaultPosition, FromDIP(wxSize(700, 400)));
			//KProgramOptionSerializer::LoadDataViewLayout(GetView(), m_ViewOptions);
			//KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
		}
	}
	AdditionalInfoDialog::~AdditionalInfoDialog()
	{
		IncRef();
	
		//KProgramOptionSerializer::SaveDataViewLayout(GetView(), m_ViewOptions);
		//KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
	}
}
