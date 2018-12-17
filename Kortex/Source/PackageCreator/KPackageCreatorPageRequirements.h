#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "KPackageCreatorPageBase.h"
class KPackageCreatorWorkspace;
class KPackageProjectRequirements;
class KPCRGroupsModel;
class KPCREntriesListModel;
class KxComboBox;
class KxButton;

class KPackageCreatorPageRequirements: public KPackageCreatorPageBase
{
	friend class KPackageCreatorWorkspace;

	private:
		//KProgramOptionAI m_MainOptions;

		// Sets list
		KPCRGroupsModel* m_GroupsModel = nullptr;
		KxButton* m_DefaultGroupsButton = nullptr;
		//KProgramOptionAI m_GroupsModelOptions;

		// Requirements list
		KPCREntriesListModel* m_EntriesModel = nullptr;
		//KProgramOptionAI m_EntriesModelOptions;

		// Std requirements list
		KxComboBox* m_StdReqs_Categories = nullptr;
		KxComboBox* m_StdReqs_List = nullptr;
		KxButton* m_StdReqs_Add = nullptr;

	public:
		KPackageCreatorPageRequirements(KPackageCreatorWorkspace* mainWorkspace, KPackageCreatorController* controller);
		virtual ~KPackageCreatorPageRequirements();

	private:
		virtual bool OnCreateWorkspace() override;
		KPackageProjectRequirements& GetProjectRequirements() const;
		void SelectComboBoxItem(KxComboBox* control, int itemIndex);
		
		void CreateGroupsControls();
		void CreateEntriesControls();
		void CreateStdReqsControls();
		void LoadStdReqs();
		void OnSelectStdReqCategory(wxCommandEvent& event);
		void OnSelectStdReq(wxCommandEvent& event);
		void OnAddStdReq(wxCommandEvent& event);

	private:
		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnLoadProject(KPackageProjectRequirements& projectRequirements);

	public:
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_CHEQUE;
		}
		virtual wxString GetID() const override;
		virtual wxString GetPageName() const override;
};
