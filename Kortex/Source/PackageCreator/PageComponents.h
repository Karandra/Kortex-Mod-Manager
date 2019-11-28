#pragma once
#include "stdafx.h"
#include "PageBase.h"
#include "PackageProject/ComponentsSection.h"
class KxImageView;

namespace Kortex::PackageDesigner
{
	class Workspace;
}
namespace Kortex::PackageProject
{
	class ComponentsSection;
}
namespace Kortex::PackageDesigner::PageComponentsNS
{
	class FileDataSelectorComboBox;
	class ComponentsModel;
}

namespace Kortex::PackageDesigner
{
	class PageComponents: public PageBase
	{
		friend class Workspace;

		public:
			static wxString FormatArrayToText(const KxStringVector& array);
			static wxString ConditionToString(const PackageProject::Condition& condition, bool isRequired);
			static wxString ConditionGroupToString(const PackageProject::ConditionGroup& conditionGroup);

		private:
			//KProgramOptionAI m_MainOptions;
			//KProgramOptionAI m_ComponentsOptions;
			
			wxBoxSizer* m_MainSizer = nullptr;
			PageComponentsNS::ComponentsModel* m_ComponentsModel = nullptr;
			
			// Misc controls
			PageComponentsNS::FileDataSelectorComboBox* m_RequiredFilesModel = nullptr;
			KxImageView* m_EntryImage = nullptr;

		private:
			PackageProject::ComponentsSection& GetProjectComponents() const;
			void OnLoadProject(PackageProject::ComponentsSection& projectComponents);
			
			void CreateComponentsView();
			void CreateMiscControls();

		protected:
			bool OnCreateWorkspace() override;
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;

		public:
			PageComponents(Workspace& mainWorkspace, WorkspaceDocument& controller);
			~PageComponents();

		public:
			ResourceID GetIcon() const override
			{
				return ImageResourceID::Block;
			}
			wxString GetID() const override;
			wxString GetPageName() const override;
	};
}
