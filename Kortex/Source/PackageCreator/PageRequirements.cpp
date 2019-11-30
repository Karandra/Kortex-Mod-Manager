#include "stdafx.h"
#include "PageRequirements.h"
#include "PageBase.h"
#include "Workspace.h"
#include "WorkspaceDocument.h"
#include "PageRequirements/GroupsModel.h"
#include "PageRequirements/EntriesListModel.h"
#include "PageComponents/RequirementsSelectorModel.h"
#include "ModPackages/IPackageManager.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxComboBox.h>
#include <KxFramework/KxTextBoxDialog.h>
#include <KxFramework/KxTaskDialog.h>

namespace Kortex::PackageDesigner
{
	bool PageRequirements::OnCreateWorkspace()
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(m_MainSizer);

		CreateGroupsControls();
		CreateEntriesControls();
		CreateStdReqsControls();

		//KProgramOptionSerializer::LoadDataViewLayout(m_GroupsModel->GetView(), m_GroupsModelOptions);
		//KProgramOptionSerializer::LoadDataViewLayout(m_EntriesModel->GetView(), m_EntriesModelOptions);
		return true;
	}
	bool PageRequirements::OnOpenWorkspace()
	{
		return true;
	}
	bool PageRequirements::OnCloseWorkspace()
	{
		return true;
	}
	void PageRequirements::OnLoadProject(PackageProject::RequirementsSection& projectRequirements)
	{
		wxWindowUpdateLocker lock(this);

		m_GroupsModel->SetProject(projectRequirements.GetProject());
		m_EntriesModel->SetProject(projectRequirements.GetProject());
	}

	PageRequirements::PageRequirements(Workspace& mainWorkspace, WorkspaceDocument& controller)
		:PageBase(mainWorkspace, controller)
		//m_MainOptions(this, "MainUI"), m_GroupsModelOptions(this, "GroupsListView"), m_EntriesModelOptions(this, "EntriesListView")
	{
	}
	PageRequirements::~PageRequirements()
	{
		if (IsCreated())
		{
			//KProgramOptionSerializer::SaveDataViewLayout(m_GroupsModel->GetView(), m_GroupsModelOptions);
			//KProgramOptionSerializer::SaveDataViewLayout(m_EntriesModel->GetView(), m_EntriesModelOptions);

			m_GroupsModel->SetDataVector();
			m_EntriesModel->SetDataVector();
		}
	}

	PackageProject::RequirementsSection& PageRequirements::GetProjectRequirements() const
	{
		return GetProject()->GetRequirements();
	}
	void PageRequirements::SelectComboBoxItem(KxComboBox* control, int itemIndex)
	{
		control->SetSelection(itemIndex);

		wxCommandEvent event(wxEVT_COMBOBOX, control->GetId());
		event.SetEventObject(control);
		event.SetInt(itemIndex);
		control->HandleWindowEvent(event);
	}

	void PageRequirements::CreateGroupsControls()
	{
		// Main caption
		KxLabel* label = CreateCaptionLabel(this, KTr("PackageCreator.PageRequirements.Groups"));
		m_MainSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

		// Sizer
		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		m_MainSizer->Add(sizer, 0, wxEXPAND|wxLEFT, ms_LeftMargin);

		// List
		m_GroupsModel = new PageRequirementsNS::GroupsModel();
		m_GroupsModel->Create(m_Controller, this, sizer);

		// Default groups button
		m_DefaultGroupsButton = new KxButton(this, KxID_NONE, KTr("PackageCreator.PageRequirements.DefaultGroups"));
		m_DefaultGroupsButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
		{
			PageComponentsNS::RequirementsSelectorDialog dialog(this, m_DefaultGroupsButton->GetLabel(), m_Controller);
			dialog.SetDataVector(GetProjectRequirements().GetDefaultGroup(), &m_Controller->GetProject()->GetRequirements());
			if (dialog.ShowModal() == KxID_OK)
			{
				GetProjectRequirements().GetDefaultGroup() = dialog.GetSelectedItems();
				m_Controller->ChangeNotify();
			}
		});
		sizer->Add(m_DefaultGroupsButton, 0, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING_SMALL);
	}
	void PageRequirements::CreateEntriesControls()
	{
		// Main caption
		KxLabel* label = CreateCaptionLabel(this, KTr("PackageCreator.PageRequirements.Name"));
		m_MainSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

		// Sizer
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_MainSizer->Add(sizer, 1, wxEXPAND|wxLEFT, ms_LeftMargin);

		// List
		m_EntriesModel = new PageRequirementsNS::EntriesListModel();
		m_EntriesModel->Create(m_Controller, this, sizer);
		m_GroupsModel->SetEntriesModel(m_EntriesModel);
	}
	void PageRequirements::CreateStdReqsControls()
	{
		// Main caption
		KxLabel* label = CreateCaptionLabel(this, KTr("PackageCreator.PageRequirements.StdReqs"));
		m_MainSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

		// Sizer
		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		m_MainSizer->Add(sizer, 0, wxEXPAND|wxLEFT, ms_LeftMargin);

		// Categories
		m_StdReqs_Categories = new KxComboBox(this, KxID_NONE);
		m_StdReqs_Categories->Bind(wxEVT_COMBOBOX, &PageRequirements::OnSelectStdReqCategory, this);
		sizer->Add(m_StdReqs_Categories, 0, wxEXPAND)->SetMinSize(200, wxDefaultCoord);

		// List
		m_StdReqs_List = new KxComboBox(this, KxID_NONE);
		m_StdReqs_List->Enable(false);
		m_StdReqs_List->Bind(wxEVT_COMBOBOX, &PageRequirements::OnSelectStdReq, this);
		sizer->Add(m_StdReqs_List, 1, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING_SMALL);

		// Add button
		m_StdReqs_Add = new KxButton(this, KxID_NONE, KTr(KxID_ADD));
		m_StdReqs_Add->Enable(false);
		m_StdReqs_Add->Bind(wxEVT_BUTTON, &PageRequirements::OnAddStdReq, this);
		sizer->Add(m_StdReqs_Add, 0, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING_SMALL);

		// Add items
		LoadStdReqs();
	}
	void PageRequirements::LoadStdReqs()
	{
		m_StdReqs_Categories->AddItem(KVarExp("<$T(Generic.All)>"));

		std::unordered_set<wxString> categoriesSet;
		for (const auto& entry: Kortex::IPackageManager::GetInstance()->GetStdRequirements())
		{
			if (categoriesSet.emplace(entry->GetCategory()).second)
			{
				int index = m_StdReqs_Categories->AddItem(entry->GetCategory());
				m_StdReqs_Categories->SetClientObject(index, new wxStringClientData(entry->GetCategory()));
			}
		}
		SelectComboBoxItem(m_StdReqs_Categories, 0);
	}
	void PageRequirements::OnSelectStdReqCategory(wxCommandEvent& event)
	{
		int index = event.GetInt();
		if (index != -1)
		{
			wxString requestedCategory;
			wxString lastCategory;
			if (wxStringClientData* data = static_cast<wxStringClientData*>(m_StdReqs_Categories->GetClientObject(index)))
			{
				requestedCategory = data->GetData();
			}

			m_StdReqs_List->Clear();

			for (const auto& entry: Kortex::IPackageManager::GetInstance()->GetStdRequirements())
			{
				if (requestedCategory.IsEmpty() || requestedCategory == entry->GetCategory())
				{
					if (lastCategory != entry->GetCategory() && requestedCategory.IsEmpty())
					{
						m_StdReqs_List->AddItem(wxString::Format("-- %s", entry->GetCategory()));
					}
					lastCategory = entry->GetCategory();

					int index = m_StdReqs_List->AddItem(wxString::Format("%s - \"%s\"", entry->GetID(), entry->GetName()));
					m_StdReqs_List->SetClientData(index, entry.get());
				}
			}

			SelectComboBoxItem(m_StdReqs_List, requestedCategory.IsEmpty() ? 1 : 0);
			m_StdReqs_List->Enable(true);
			return;
		}
		m_StdReqs_List->Enable(false);
	}
	void PageRequirements::OnSelectStdReq(wxCommandEvent& event)
	{
		m_StdReqs_Add->Enable(m_StdReqs_List->GetClientData(event.GetInt()));
	}
	void PageRequirements::OnAddStdReq(wxCommandEvent& event)
	{
		int index = m_StdReqs_List->GetSelection();
		if (index != -1)
		{
			if (PackageProject::RequirementItem* stdEntry = static_cast<PackageProject::RequirementItem*>(m_StdReqs_List->GetClientData(index)))
			{
				if (PackageProject::RequirementGroup* pSet = m_EntriesModel->GetRequirementsGroup())
				{
					if (!pSet->HasItemWithID(stdEntry->GetID()))
					{
						auto& newEntry = pSet->GetItems().emplace_back(std::make_unique<PackageProject::RequirementItem>(*stdEntry));
						newEntry->ResetObjectFunctionResult();
						newEntry->SetCurrentVersion(KxNullVersion);

						m_EntriesModel->NotifyAddedItem(m_EntriesModel->GetItem(m_EntriesModel->GetItemCount() - 1));
					}
					else
					{
						WarnIDCollision(m_StdReqs_Add);
					}
				}
			}
		}
	}

	wxString PageRequirements::GetID() const
	{
		return "KPackageCreator.PageRequirements";
	}
	wxString PageRequirements::GetPageName() const
	{
		return KTr("PackageCreator.PageRequirements.Name");
	}
}
