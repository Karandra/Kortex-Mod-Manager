#include "stdafx.h"
#include "RequirementsSelectorModel.h"
#include "PackageCreator/PageBase.h"
#include <Kortex/Application.hpp>

namespace
{
	enum ColumnID
	{
		Checked,
		ID,
	};
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	void RequirementsSelectorModel::OnInitControl()
	{
		GetView()->AppendColumn<KxDataViewToggleRenderer>(wxEmptyString, ColumnID::Checked, KxDATAVIEW_CELL_ACTIVATABLE, KxCOL_WIDTH_AUTOSIZE);
		GetView()->AppendColumn<KxDataViewTextRenderer>(wxEmptyString, ColumnID::ID, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE);
	}
	
	void RequirementsSelectorModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
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
	bool RequirementsSelectorModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
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
	
	void RequirementsSelectorModel::SetDataVector()
	{
		m_ReqData = nullptr;
		m_DataVector.clear();
		VectorModel::SetDataVector();
	}
	void RequirementsSelectorModel::SetDataVector(const KxStringVector& data, PackageProject::RequirementsSection* reqData)
	{
		SetDataVector();
		m_ReqData = reqData;
	
		if (reqData)
		{
			// Add selected first
			for (const wxString& id: data)
			{
				if (PackageProject::RequirementGroup* entry = m_ReqData->FindGroupWithID(id))
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
			VectorModel::SetDataVector(&m_DataVector);
		}
	}
	KxStringVector RequirementsSelectorModel::GetSelectedItems() const
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
	RequirementsSelectorDialog::RequirementsSelectorDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller)
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
			RequirementsSelectorModel::Create(controller, m_ViewPane, sizer);
			
			AdjustWindow(wxDefaultPosition, FromDIP(wxSize(400, 320)));
		}
	}
	RequirementsSelectorDialog::~RequirementsSelectorDialog()
	{
		IncRef();
	}
}
