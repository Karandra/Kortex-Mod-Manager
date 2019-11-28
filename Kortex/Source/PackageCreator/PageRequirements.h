#pragma once
#include "stdafx.h"
#include "PageBase.h"
class KxComboBox;
class KxButton;

namespace Kortex::PackageProject
{
	class KPackageProjectRequirements;
}
namespace Kortex::PackageDesigner
{
	class Workspace;
}
namespace Kortex::PackageDesigner::PageRequirementsNS
{
	class GroupsModel;
	class EntriesListModel;
}

namespace Kortex::PackageDesigner
{
	class PageRequirements: public PageBase
	{
		friend class Workspace;

		private:
			//KProgramOptionAI m_MainOptions;
			
			wxBoxSizer* m_MainSizer = nullptr;
			
			// Sets list
			PageRequirementsNS::GroupsModel* m_GroupsModel = nullptr;
			KxButton* m_DefaultGroupsButton = nullptr;
			//KProgramOptionAI m_GroupsModelOptions;

			// Requirements list
			PageRequirementsNS::EntriesListModel* m_EntriesModel = nullptr;
			//KProgramOptionAI m_EntriesModelOptions;

			// Std requirements list
			KxComboBox* m_StdReqs_Categories = nullptr;
			KxComboBox* m_StdReqs_List = nullptr;
			KxButton* m_StdReqs_Add = nullptr;

		private:
			void OnLoadProject(PackageProject::KPackageProjectRequirements& projectRequirements);
			PackageProject::KPackageProjectRequirements& GetProjectRequirements() const;
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
			PageRequirements(Workspace& mainWorkspace, WorkspaceDocument& controller);
			~PageRequirements();

		public:
			ResourceID GetIcon() const override
			{
				return ImageResourceID::Cheque;
			}
			wxString GetID() const override;
			wxString GetPageName() const override;
	};
}
