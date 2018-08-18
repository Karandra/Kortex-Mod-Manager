#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include "KPackageCreatorPageBase.h"
#include "KProgramOptions.h"
class KPackageCreatorWorkspace;
class KPackageProjectInterface;
class KPCIImagesListModel;
class KxComboBox;

class KPackageCreatorPageInterface:	public KPackageCreatorPageBase
{
	friend class KPackageCreatorWorkspace;

	private:
		KPCIImagesListModel* m_ImageListModel = NULL;
		KProgramOptionUI m_MainOptions;
		KProgramOptionUI m_ListOptions;

	public:
		KPackageCreatorPageInterface(KPackageCreatorWorkspace* mainWorkspace, KPackageCreatorController* controller);
		virtual ~KPackageCreatorPageInterface();

	private:
		virtual bool OnCreateWorkspace() override;
		KPackageProjectInterface& GetProjectInterface() const;
		void CreateImageListControls();

	private:
		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnLoadProject(KPackageProjectInterface& projectInterface);

	public:
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_IMAGE;
		}
		virtual wxString GetID() const override;
		virtual wxString GetPageName() const override;
};
