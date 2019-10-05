#include "stdafx.h"
#include "KPCInfoAdditionalInfoModel.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "UI/TextEditDialog.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"
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

namespace Kortex::PackageDesigner
{
	void KPCInfoAdditionalInfoModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KPCInfoAdditionalInfoModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KPCInfoAdditionalInfoModel::OnContextMenu, this);
	
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 300);
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("Generic.Value"), ColumnID::Value, m_UseInlineEditor ? KxDATAVIEW_CELL_EDITABLE : KxDATAVIEW_CELL_INERT, 300);
	}
	
	void KPCInfoAdditionalInfoModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		auto entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = entry->GetLabelRaw();
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
	bool KPCInfoAdditionalInfoModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
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
	
	void KPCInfoAdditionalInfoModel::OnActivateItem(KxDataViewEvent& event)
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
						KLabeledValue* entry = GetDataEntry(GetRow(event.GetItem()));
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
	void KPCInfoAdditionalInfoModel::OnContextMenu(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		const KLabeledValue* entry = GetDataEntry(GetRow(item));
	
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
	
	void KPCInfoAdditionalInfoModel::OnAddEntry()
	{
		GetDataVector()->emplace_back(KLabeledValue(wxEmptyString));
	
		KxDataViewItem item = GetItem(GetItemCount() - 1);
		NotifyAddedItem(item);
		SelectItem(item);
		GetView()->EditItem(item, GetView()->GetColumn(ColumnID::Name));
	}
	void KPCInfoAdditionalInfoModel::OnRemoveEntry(const KxDataViewItem& item)
	{
		RemoveItemAndNotify(*GetDataVector(), item);
	}
	void KPCInfoAdditionalInfoModel::OnClearList()
	{
		ClearItemsAndNotify(*GetDataVector());
	}
	
	void KPCInfoAdditionalInfoModel::SetDataVector()
	{
		m_InfoData = nullptr;
		KPackageCreatorVectorModel::SetDataVector();
	}
	void KPCInfoAdditionalInfoModel::SetDataVector(VectorType& data, KPackageProjectInfo* info)
	{
		m_InfoData = info;
		KPackageCreatorVectorModel::SetDataVector(&data);
	}
}

namespace Kortex::PackageDesigner
{
	KPCInfoAdditionalInfoModelDialog::KPCInfoAdditionalInfoModelDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller, bool bUseInlineEditor)
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
			KPCInfoAdditionalInfoModel::Create(controller, m_ViewPane, sizer);
	
			AdjustWindow(wxDefaultPosition, wxSize(700, 400));
			//KProgramOptionSerializer::LoadDataViewLayout(GetView(), m_ViewOptions);
			//KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
		}
	}
	KPCInfoAdditionalInfoModelDialog::~KPCInfoAdditionalInfoModelDialog()
	{
		IncRef();
	
		//KProgramOptionSerializer::SaveDataViewLayout(GetView(), m_ViewOptions);
		//KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
	}
}
