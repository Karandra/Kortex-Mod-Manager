#include "stdafx.h"
#include "KPCCRequirementsSelectorModel.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "UI/KMainWindow.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"

enum ColumnID
{
	Checked,
	ID,
};

void KPCCRequirementsSelectorModel::OnInitControl()
{
	GetView()->AppendColumn<KxDataViewToggleRenderer>(wxEmptyString, ColumnID::Checked, KxDATAVIEW_CELL_ACTIVATABLE, KxCOL_WIDTH_AUTOSIZE);
	GetView()->AppendColumn<KxDataViewTextRenderer>(wxEmptyString, ColumnID::ID, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE);
}

void KPCCRequirementsSelectorModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
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
				value = entry->first->GetID();
				break;
			}
		};
	}
}
bool KPCCRequirementsSelectorModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
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

void KPCCRequirementsSelectorModel::SetDataVector()
{
	m_ReqData = nullptr;
	m_DataVector.clear();
	KPackageCreatorVectorModel::SetDataVector();
}
void KPCCRequirementsSelectorModel::SetDataVector(const KxStringVector& data, KPackageProjectRequirements* reqData)
{
	SetDataVector();
	m_ReqData = reqData;

	if (reqData)
	{
		// Add selected first
		for (const wxString& id: data)
		{
			if (KPPRRequirementsGroup* entry = m_ReqData->FindGroupWithID(id))
			{
				m_DataVector.emplace_back(std::make_pair(entry, true));
			}
		}

		// Add others
		for (const auto& entry: m_ReqData->GetGroups())
		{
			if (std::find(data.cbegin(), data.cend(), entry->GetID()) == data.cend())
			{
				m_DataVector.emplace_back(std::make_pair(entry.get(), false));
			}
		}
		KPackageCreatorVectorModel::SetDataVector(&m_DataVector);
	}
}
KxStringVector KPCCRequirementsSelectorModel::GetSelectedItems() const
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
KPCCRequirementsSelectorModelDialog::KPCCRequirementsSelectorModelDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller)
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
		SetDataViewFlags(KxDV_NO_HEADER);
		KPCCRequirementsSelectorModel::Create(controller, m_ViewPane, sizer);

		AdjustWindow(wxDefaultPosition, wxSize(400, 320));
	}
}
KPCCRequirementsSelectorModelDialog::~KPCCRequirementsSelectorModelDialog()
{
	IncRef();
}
