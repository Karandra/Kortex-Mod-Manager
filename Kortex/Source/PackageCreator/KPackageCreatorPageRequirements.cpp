#include "stdafx.h"
#include "KPackageCreatorPageRequirements.h"
#include "KPackageCreatorPageBase.h"
#include "KPackageCreatorWorkspace.h"
#include "KPackageCreatorController.h"
#include "PageRequirements/KPCRGroupsModel.h"
#include "PageRequirements/KPCREntriesListModel.h"
#include "PageComponents/KPCCRequirementsSelectorModel.h"
#include "PackageManager/KPackageManager.h"
#include "KThemeManager.h"
#include "KApp.h"
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxComboBox.h>
#include <KxFramework/KxTextBoxDialog.h>
#include <KxFramework/KxTaskDialog.h>

KPackageCreatorPageRequirements::KPackageCreatorPageRequirements(KPackageCreatorWorkspace* mainWorkspace, KPackageCreatorController* controller)
	:KPackageCreatorPageBase(mainWorkspace, controller),
	m_MainOptions(this, "MainUI"), m_GroupsModelOptions(this, "GroupsListView"), m_EntriesModelOptions(this, "EntriesListView")
{
}
KPackageCreatorPageRequirements::~KPackageCreatorPageRequirements()
{
	if (IsWorkspaceCreated())
	{
		KProgramOptionSerializer::SaveDataViewLayout(m_GroupsModel->GetView(), m_GroupsModelOptions);
		KProgramOptionSerializer::SaveDataViewLayout(m_EntriesModel->GetView(), m_EntriesModelOptions);

		m_GroupsModel->SetDataVector();
		m_EntriesModel->SetDataVector();
	}
}

bool KPackageCreatorPageRequirements::OnCreateWorkspace()
{
	CreateGroupsControls();
	CreateEntriesControls();
	CreateStdReqsControls();

	KProgramOptionSerializer::LoadDataViewLayout(m_GroupsModel->GetView(), m_GroupsModelOptions);
	KProgramOptionSerializer::LoadDataViewLayout(m_EntriesModel->GetView(), m_EntriesModelOptions);
	return true;
}
KPackageProjectRequirements& KPackageCreatorPageRequirements::GetProjectRequirements() const
{
	return GetProject()->GetRequirements();
}
void KPackageCreatorPageRequirements::SelectComboBoxItem(KxComboBox* control, int itemIndex)
{
	control->SetSelection(itemIndex);

	wxCommandEvent event(wxEVT_COMBOBOX, control->GetId());
	event.SetEventObject(control);
	event.SetInt(itemIndex);
	control->HandleWindowEvent(event);
}

void KPackageCreatorPageRequirements::CreateGroupsControls()
{
	// Main caption
	KxLabel* label = CreateCaptionLabel(this, T("PackageCreator.PageRequirements.Groups"));
	m_MainSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

	// Sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	m_MainSizer->Add(sizer, 0, wxEXPAND|wxLEFT, ms_LeftMargin);

	// List
	m_GroupsModel = new KPCRGroupsModel();
	m_GroupsModel->Create(m_Controller, this, sizer);

	// Default groups button
	m_DefaultGroupsButton = new KxButton(this, KxID_NONE, T("PackageCreator.PageRequirements.DefaultGroups"));
	m_DefaultGroupsButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
	{
		KPCCRequirementsSelectorModelDialog dialog(KApp::Get().GetMainWindow(), m_DefaultGroupsButton->GetLabel(), m_Controller);
		dialog.SetDataVector(GetProjectRequirements().GetDefaultGroup(), &m_Controller->GetProject()->GetRequirements());
		if (dialog.ShowModal() == KxID_OK)
		{
			GetProjectRequirements().GetDefaultGroup() = dialog.GetSelectedItems();
			m_Controller->ChangeNotify();
		}
	});
	sizer->Add(m_DefaultGroupsButton, 0, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING_SMALL);
}
void KPackageCreatorPageRequirements::CreateEntriesControls()
{
	// Main caption
	KxLabel* label = CreateCaptionLabel(this, T("PackageCreator.PageRequirements.Name"));
	m_MainSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

	// Sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	m_MainSizer->Add(sizer, 1, wxEXPAND|wxLEFT, ms_LeftMargin);

	// List
	m_EntriesModel = new KPCREntriesListModel();
	m_EntriesModel->Create(m_Controller, this, sizer);
	m_GroupsModel->SetEntriesModel(m_EntriesModel);
}
void KPackageCreatorPageRequirements::CreateStdReqsControls()
{
	// Main caption
	KxLabel* label = CreateCaptionLabel(this, T("PackageCreator.PageRequirements.StdReqs"));
	m_MainSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

	// Sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	m_MainSizer->Add(sizer, 0, wxEXPAND|wxLEFT, ms_LeftMargin);

	// Categories
	m_StdReqs_Categories = new KxComboBox(this, KxID_NONE);
	m_StdReqs_Categories->Bind(wxEVT_COMBOBOX, &KPackageCreatorPageRequirements::OnSelectStdReqCategory, this);
	sizer->Add(m_StdReqs_Categories, 0, wxEXPAND)->SetMinSize(200, wxDefaultCoord);

	// List
	m_StdReqs_List = new KxComboBox(this, KxID_NONE);
	m_StdReqs_List->Enable(false);
	m_StdReqs_List->Bind(wxEVT_COMBOBOX, &KPackageCreatorPageRequirements::OnSelectStdReq, this);
	sizer->Add(m_StdReqs_List, 1, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING_SMALL);

	// Add button
	m_StdReqs_Add = new KxButton(this, KxID_NONE, T(KxID_ADD));
	m_StdReqs_Add->Enable(false);
	m_StdReqs_Add->Bind(wxEVT_BUTTON, &KPackageCreatorPageRequirements::OnAddStdReq, this);
	sizer->Add(m_StdReqs_Add, 0, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING_SMALL);

	// Add items
	LoadStdReqs();
}
void KPackageCreatorPageRequirements::LoadStdReqs()
{
	m_StdReqs_Categories->AddItem(V("<$T(Generic.All)>"));

	std::unordered_set<wxString> categoriesSet;
	for (const auto& entry: KPackageManager::Get().GetStdRequirements())
	{
		if (categoriesSet.emplace(entry->GetCategory()).second)
		{
			int index = m_StdReqs_Categories->AddItem(entry->GetCategory());
			m_StdReqs_Categories->SetClientObject(index, new wxStringClientData(entry->GetCategory()));
		}
	}
	SelectComboBoxItem(m_StdReqs_Categories, 0);
}
void KPackageCreatorPageRequirements::OnSelectStdReqCategory(wxCommandEvent& event)
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

		for (const auto& entry: KPackageManager::Get().GetStdRequirements())
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
void KPackageCreatorPageRequirements::OnSelectStdReq(wxCommandEvent& event)
{
	m_StdReqs_Add->Enable(m_StdReqs_List->GetClientData(event.GetInt()));
}
void KPackageCreatorPageRequirements::OnAddStdReq(wxCommandEvent& event)
{
	int index = m_StdReqs_List->GetSelection();
	if (index != -1)
	{
		if (KPPRRequirementEntry* stdEntry = static_cast<KPPRRequirementEntry*>(m_StdReqs_List->GetClientData(index)))
		{
			if (KPPRRequirementsGroup* pSet = m_EntriesModel->GetRequirementsGroup())
			{
				if (!pSet->HasEntryWithID(stdEntry->GetID()))
				{
					auto& newEntry = pSet->GetEntries().emplace_back(new KPPRRequirementEntry(*stdEntry));
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

bool KPackageCreatorPageRequirements::OnOpenWorkspace()
{
	return true;
}
bool KPackageCreatorPageRequirements::OnCloseWorkspace()
{
	return true;
}
void KPackageCreatorPageRequirements::OnLoadProject(KPackageProjectRequirements& projectRequirements)
{
	wxWindowUpdateLocker lock(this);

	m_GroupsModel->SetProject(projectRequirements.GetProject());
	m_EntriesModel->SetProject(projectRequirements.GetProject());
}

wxString KPackageCreatorPageRequirements::GetID() const
{
	return "KPackageCreator.PageRequirements";
}
wxString KPackageCreatorPageRequirements::GetPageName() const
{
	return T("PackageCreator.PageRequirements.Name");
}
