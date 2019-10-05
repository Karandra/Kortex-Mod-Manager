#pragma once
#include "stdafx.h"
#include "KPackageCreatorPageBase.h"
class KxComboBox;
class KxButton;

namespace Kortex::PackageDesigner
{
	class KPackageCreatorWorkspace;
	class KPackageProjectRequirements;
	class KPCRGroupsModel;
	class KPCREntriesListModel;
}

namespace Kortex::PackageDesigner
{
	class KPackageCreatorPageRequirements: public KPackageCreatorPageBase
	{
		friend class KPackageCreatorWorkspace;

		private:
			//KProgramOptionAI m_MainOptions;
			
			wxBoxSizer* m_MainSizer = nullptr;
			
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

		private:
			void OnLoadProject(KPackageProjectRequirements& projectRequirements);
			KPackageProjectRequirements& GetProjectRequirements() const;
			void SelectComboBoxItem(KxComboBox* control, int itemIndex);
		
			void CreateGroupsControls();
			void CreateEntriesControls();
			void CreateStdReqsControls();
			void LoadStdReqs();
			void OnSelectStdReqCategory(wxCommandEvent& event);
			void OnSelectStdReq(wxCommandEvent& event);
			void OnAddStdReq(wxCommandEvent& event);

		protected:
			bool OnCreateWorkspace() override;
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;

		public:
			KPackageCreatorPageRequirements(KPackageCreatorWorkspace& mainWorkspace, KPackageCreatorController& controller);
			~KPackageCreatorPageRequirements();

		public:
			ResourceID GetIcon() const override
			{
				return ImageResourceID::Cheque;
			}
			wxString GetID() const override;
			wxString GetPageName() const override;
	};
}
