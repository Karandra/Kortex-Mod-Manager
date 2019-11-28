#include "stdafx.h"
#include "SitesModel.h"
#include "PackageCreator/PageBase.h"
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
		AddSite,
	};
}

namespace Kortex::PackageDesigner::PageInfoNS
{
	void SitesModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &SitesModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &SitesModel::OnContextMenu, this);
	
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 200);
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("Generic.Address"), ColumnID::Value, m_UseInlineEditor ? KxDATAVIEW_CELL_EDITABLE : KxDATAVIEW_CELL_INERT, 300);
	}
	
	void SitesModel::GetEditorValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
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
	void SitesModel::GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
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
	bool SitesModel::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
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
	
	void SitesModel::OnActivateItem(KxDataViewEvent& event)
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
							UI::TextEditDialog dialog(GetView());
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
	void SitesModel::OnContextMenu(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		const KLabeledValue* entry = GetDataEntry(GetRow(item));
	
		KxMenu menu;
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddSite, KTr(KxID_ADD)));
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
	
	void SitesModel::OnAddSite()
	{
		GetDataVector()->emplace_back(KLabeledValue(wxEmptyString));
	
		KxDataViewItem item = GetItem(GetItemCount() - 1);
		NotifyAddedItem(item);
		SelectItem(item);
		GetView()->EditItem(item, GetView()->GetColumn(ColumnID::Name));
	}
	void SitesModel::OnRemoveSite(const KxDataViewItem& item)
	{
		RemoveItemAndNotify(*GetDataVector(), item);
	}
	void SitesModel::OnClearList()
	{
		ClearItemsAndNotify(*GetDataVector());
	}
	
	void SitesModel::SetDataVector()
	{
		m_InfoData = nullptr;
		VectorModel::SetDataVector();
	}
	void SitesModel::SetDataVector(VectorType& data, PackageProject::InfoSection* info)
	{
		m_InfoData = info;
		VectorModel::SetDataVector(&data);
	}
}

namespace Kortex::PackageDesigner::PageInfoNS
{
	SitesDialog::SitesDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller, bool bUseInlineEditor)
		//:m_WindowOptions("SitesDialog", "Window"), m_ViewOptions("SitesDialog", "View")
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
	
			// List2
			IncRef();
			SitesModel::Create(controller, m_ViewPane, sizer);
	
			AdjustWindow(wxDefaultPosition, FromDIP(wxSize(700, 400)));
			//KProgramOptionSerializer::LoadDataViewLayout(GetView(), m_ViewOptions);
			//KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
		}
	}
	SitesDialog::~SitesDialog()
	{
		IncRef();
	
		//KProgramOptionSerializer::SaveDataViewLayout(GetView(), m_ViewOptions);
		//KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
	}
}
