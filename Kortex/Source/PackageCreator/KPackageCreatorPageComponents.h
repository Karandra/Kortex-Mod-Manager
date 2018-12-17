#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "KPackageCreatorPageBase.h"
#include "PackageProject/KPackageProjectComponents.h"
class KPackageCreatorWorkspace;
class KPackageProjectComponents;
class KPCCFileDataSelectorModelCB;
class KPCComponentsModel;

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
		KPCComponentsModel* m_ComponentsModel = nullptr;

		// Misc controls
		KPCCFileDataSelectorModelCB* m_RequiredFilesModel = nullptr;
		KxImageView* m_EntryImage = nullptr;

	public:
		KPackageCreatorPageComponents(KPackageCreatorWorkspace* mainWorkspace, KPackageCreatorController* controller);
		virtual ~KPackageCreatorPageComponents();

	private:
		virtual bool OnCreateWorkspace() override;
		KPackageProjectComponents& GetProjectComponents() const;

		void CreateComponentsView();
		void CreateMiscControls();

	private:
		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnLoadProject(KPackageProjectComponents& projectComponents);

	public:
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_BLOCK;
		}
		virtual wxString GetID() const override;
		virtual wxString GetPageName() const override;
};
