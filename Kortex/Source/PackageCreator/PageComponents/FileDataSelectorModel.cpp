#include "stdafx.h"
#include "FileDataSelectorModel.h"
#include "PackageCreator/PageBase.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxDataViewComboBox.h>

namespace
{
	enum ColumnID
	{
		Checked,
		ID,
		Source,
	};
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	void FileDataSelectorModel::OnInitControl()
	{
		GetView()->AppendColumn<KxDataViewToggleRenderer>(wxEmptyString, ColumnID::Checked, KxDATAVIEW_CELL_ACTIVATABLE, KxCOL_WIDTH_AUTOSIZE, KxDV_COL_NONE);
		GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(KTr("PackageCreator.PageFileData.MainList.InPackagePath"), ColumnID::ID, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("PackageCreator.PageFileData.MainList.Source"), ColumnID::Source, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE);
	}
	
	void FileDataSelectorModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		auto entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Checked:
				{
					value = entry->second;
					break;
				}
				case ColumnID::ID:
				{
					value = KxDataViewBitmapTextValue(entry->first->GetID(), ImageProvider::GetBitmap(entry->first->ToFolderItem() ? ImageResourceID::Folder : ImageResourceID::Document));
					break;
				}
				case ColumnID::Source:
				{
					value = entry->first->GetSource();
					break;
				}
			};
		}
	}
	bool FileDataSelectorModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
	{
		auto entry = GetDataEntry(row);
		if (entry && column->GetID() == ColumnID::Checked)
		{
			entry->second = !entry->second;
			ChangeNotify();
			return true;
		}
		return false;
	}
	
	void FileDataSelectorModel::SetDataVector()
	{
		m_FileData = nullptr;
		m_DataVector.clear();
		VectorModel::SetDataVector();
	}
	void FileDataSelectorModel::SetDataVector(const KxStringVector& data, PackageProject::FileDataSection* fileData)
	{
		SetDataVector();
		m_FileData = fileData;
	
		if (fileData)
		{
			// Add selected first
			for (const wxString& id: data)
			{
				if (PackageProject::FileItem* entry = m_FileData->FindEntryWithID(id))
				{
					m_DataVector.emplace_back(std::make_pair(entry, true));
				}
			}
	
			// Add others
			for (const auto& entry: m_FileData->GetData())
			{
				if (std::find(data.cbegin(), data.cend(), entry->GetID()) == data.cend())
				{
					m_DataVector.emplace_back(std::make_pair(entry.get(), false));
				}
			}
			VectorModel::SetDataVector(&m_DataVector);
		}
	}
	KxStringVector FileDataSelectorModel::GetSelectedItems() const
	{
		KxStringVector data;
		for (const auto& v: *GetDataVector())
		{
			if (v.second)
			{
				data.push_back(v.first->GetID());
			}
		}
		return data;
	}
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	KxDataViewCtrl* FileDataSelectorComboBox::OnCreateDataView(wxWindow* window)
	{
		m_ComboView = new KxDataViewComboBox();
		m_ComboView->SetDataViewFlags(KxDV_NO_HEADER);
		m_ComboView->SetOptionEnabled(KxDVCB_OPTION_ALT_POPUP_WINDOW);
		m_ComboView->SetOptionEnabled(KxDVCB_OPTION_HORIZONTAL_SIZER);
		m_ComboView->SetOptionEnabled(KxDVCB_OPTION_FORCE_GET_STRING_VALUE_ON_DISMISS);
		m_ComboView->Create(window, KxID_NONE);
		m_ComboView->ComboSetMaxVisibleItems(16);
	
		m_ComboView->Bind(KxEVT_DVCB_GET_STRING_VALUE, &FileDataSelectorComboBox::OnGetStringValue, this);
		return m_ComboView;
	}
	wxWindow* FileDataSelectorComboBox::OnGetDataViewWindow()
	{
		return m_ComboView->GetComboControl();
	}
	void FileDataSelectorComboBox::OnSetDataVector()
	{
		if (m_ComboView)
		{
			m_ComboView->ComboRefreshLabel();
		}
	}
	void FileDataSelectorComboBox::OnGetStringValue(KxDataViewEvent& event)
	{
		if (HasDataVector())
		{
			*m_RequiredFiles = GetSelectedItems();
			if (!m_RequiredFiles->empty())
			{
				event.SetString(KxString::Join(*m_RequiredFiles, ", "));
				return;
			}
		}
		event.SetString(KAux::MakeNoneLabel());
	}
	
	void FileDataSelectorComboBox::SetDataVector()
	{
		FileDataSelectorModel::SetDataVector();
	}
	void FileDataSelectorComboBox::SetDataVector(KxStringVector& data, PackageProject::FileDataSection* fileData)
	{
		m_RequiredFiles = &data;
		FileDataSelectorModel::SetDataVector(data, fileData);
	}
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	FileDataSelectorDialog::FileDataSelectorDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller)
		//:m_WindowOptions("FileDataSelectorDialog", "Window"), m_ViewOptions("FileDataSelectorDialog", "View")
	{
		if (KxStdDialog::Create(parent, KxID_NONE, caption, wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL))
		{
			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide(wxBOTH);
	
			wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
			m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
			m_ViewPane->SetSizer(sizer);
			PostCreate();
	
			// List
			FileDataSelectorModel::Create(controller, m_ViewPane, sizer);
	
			AdjustWindow(wxDefaultPosition, FromDIP(wxSize(700, 500)));
			//KProgramOptionSerializer::LoadDataViewLayout(GetView(), m_ViewOptions);
			//KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
		}
	}
	FileDataSelectorDialog::~FileDataSelectorDialog()
	{
		IncRef();
	
		//KProgramOptionSerializer::SaveDataViewLayout(GetView(), m_ViewOptions);
		//KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
	}
}
