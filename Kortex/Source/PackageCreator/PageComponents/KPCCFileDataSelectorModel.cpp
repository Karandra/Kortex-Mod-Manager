#include "stdafx.h"
#include "KPCCFileDataSelectorModel.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "UI/KMainWindow.h"
#include <Kortex/Application.hpp>
#include "KAux.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxDataViewComboBox.h>

enum ColumnID
{
	Checked,
	ID,
	Source,
};

void KPCCFileDataSelectorModel::OnInitControl()
{
	GetView()->AppendColumn<KxDataViewToggleRenderer>(wxEmptyString, ColumnID::Checked, KxDATAVIEW_CELL_ACTIVATABLE, KxCOL_WIDTH_AUTOSIZE, KxDV_COL_NONE);
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(KTr("PackageCreator.PageFileData.MainList.InPackagePath"), ColumnID::ID, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE);
	GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("PackageCreator.PageFileData.MainList.Source"), ColumnID::Source, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE);
}

void KPCCFileDataSelectorModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
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
				value = KxDataViewBitmapTextValue(entry->first->GetID(), KGetBitmap(entry->first->ToFolderEntry() ? KIMG_FOLDER : KIMG_DOCUMENT));
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
bool KPCCFileDataSelectorModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
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

void KPCCFileDataSelectorModel::SetDataVector()
{
	m_FileData = nullptr;
	m_DataVector.clear();
	KPackageCreatorVectorModel::SetDataVector();
}
void KPCCFileDataSelectorModel::SetDataVector(const KxStringVector& data, KPackageProjectFileData* fileData)
{
	SetDataVector();
	m_FileData = fileData;

	if (fileData)
	{
		// Add selected first
		for (const wxString& id: data)
		{
			if (KPPFFileEntry* entry = m_FileData->FindEntryWithID(id))
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
		KPackageCreatorVectorModel::SetDataVector(&m_DataVector);
	}
}
KxStringVector KPCCFileDataSelectorModel::GetSelectedItems() const
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

//////////////////////////////////////////////////////////////////////////
KxDataViewCtrl* KPCCFileDataSelectorModelCB::OnCreateDataView(wxWindow* window)
{
	m_ComboView = new KxDataViewComboBox();
	m_ComboView->SetDataViewFlags(KxDV_NO_HEADER);
	m_ComboView->SetOptionEnabled(KxDVCB_OPTION_ALT_POPUP_WINDOW);
	m_ComboView->SetOptionEnabled(KxDVCB_OPTION_HORIZONTAL_SIZER);
	m_ComboView->SetOptionEnabled(KxDVCB_OPTION_FORCE_GET_STRING_VALUE_ON_DISMISS);
	m_ComboView->Create(window, KxID_NONE);
	m_ComboView->ComboSetMaxVisibleItems(16);

	m_ComboView->Bind(KxEVT_DVCB_GET_STRING_VALUE, &KPCCFileDataSelectorModelCB::OnGetStringValue, this);
	return m_ComboView;
}
wxWindow* KPCCFileDataSelectorModelCB::OnGetDataViewWindow()
{
	return m_ComboView->GetComboControl();
}
void KPCCFileDataSelectorModelCB::OnSetDataVector()
{
	if (m_ComboView)
	{
		m_ComboView->ComboRefreshLabel();
	}
}
void KPCCFileDataSelectorModelCB::OnGetStringValue(KxDataViewEvent& event)
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

void KPCCFileDataSelectorModelCB::SetDataVector()
{
	KPCCFileDataSelectorModel::SetDataVector();
}
void KPCCFileDataSelectorModelCB::SetDataVector(KxStringVector& data, KPackageProjectFileData* fileData)
{
	m_RequiredFiles = &data;
	KPCCFileDataSelectorModel::SetDataVector(data, fileData);
}

//////////////////////////////////////////////////////////////////////////
KPCCFileDataSelectorModelDialog::KPCCFileDataSelectorModelDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller)
	//:m_WindowOptions("KPCCFileDataSelectorModelDialog", "Window"), m_ViewOptions("KPCCFileDataSelectorModelDialog", "View")
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
		KPCCFileDataSelectorModel::Create(controller, m_ViewPane, sizer);

		AdjustWindow(wxDefaultPosition, wxSize(700, 500));
		//KProgramOptionSerializer::LoadDataViewLayout(GetView(), m_ViewOptions);
		//KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
	}
}
KPCCFileDataSelectorModelDialog::~KPCCFileDataSelectorModelDialog()
{
	IncRef();

	//KProgramOptionSerializer::SaveDataViewLayout(GetView(), m_ViewOptions);
	//KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
}
