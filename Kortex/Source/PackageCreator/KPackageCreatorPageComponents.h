#pragma once
#include "stdafx.h"
#include "KPackageCreatorPageBase.h"
#include "PackageProject/KPackageProjectComponents.h"
class KxImageView;

namespace Kortex::PackageDesigner
{
	class KPackageCreatorWorkspace;
	class KPackageProjectComponents;
	class KPCCFileDataSelectorModelCB;
	class KPCComponentsModel;
}

namespace Kortex::PackageDesigner
{
	class KPackageCreatorPageComponents: public KPackageCreatorPageBase
	{
		friend class KPackageCreatorWorkspace;

		public:
			static wxString FormatArrayToText(const KxStringVector& array);
			static wxString ConditionToString(const KPPCCondition& condition, bool isRequired);
			static wxString ConditionGroupToString(const KPPCConditionGroup& conditionGroup);

		private:
			//KProgramOptionAI m_MainOptions;
			//KProgramOptionAI m_ComponentsOptions;

			wxBoxSizer* m_MainSizer = nullptr;
			KPCComponentsModel* m_ComponentsModel = nullptr;

			// Misc controls
			KPCCFileDataSelectorModelCB* m_RequiredFilesModel = nullptr;
			KxImageView* m_EntryImage = nullptr;

		private:
			KPackageProjectComponents& GetProjectComponents() const;
			void OnLoadProject(KPackageProjectComponents& projectComponents);
			
			void CreateComponentsView();
			void CreateMiscControls();

		protected:
			bool OnCreateWorkspace() override;
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;

		public:
			KPackageCreatorPageComponents(KPackageCreatorWorkspace& mainWorkspace, KPackageCreatorController& controller);
			~KPackageCreatorPageComponents();

		public:
			ResourceID GetIcon() const override
			{
				return ImageResourceID::Block;
			}
			wxString GetID() const override;
			wxString GetPageName() const override;
	};
}
