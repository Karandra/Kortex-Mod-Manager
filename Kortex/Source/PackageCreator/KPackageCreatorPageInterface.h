#pragma once
#include "stdafx.h"
#include "KPackageCreatorPageBase.h"
class KxComboBox;

namespace Kortex::PackageDesigner
{
	class KPackageCreatorWorkspace;
	class KPackageProjectInterface;
	class KPCIImagesListModel;
}

namespace Kortex::PackageDesigner
{
	class KPackageCreatorPageInterface:	public KPackageCreatorPageBase
	{
		friend class KPackageCreatorWorkspace;

		private:
			wxBoxSizer* m_MainSizer = nullptr;
			KPCIImagesListModel* m_ImageListModel = nullptr;
			//KProgramOptionAI m_MainOptions;
			//KProgramOptionAI m_ListOptions;

		private:
			KPackageProjectInterface& GetProjectInterface() const;
			void OnLoadProject(KPackageProjectInterface& projectInterface);
			void CreateImageListControls();

		protected:
			bool OnCreateWorkspace() override;
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;

		public:
			KPackageCreatorPageInterface(KPackageCreatorWorkspace& mainWorkspace, KPackageCreatorController& controller);
			virtual ~KPackageCreatorPageInterface();

		public:
			ResourceID GetIcon() const override
			{
				return ImageResourceID::Image;
			}
			wxString GetID() const override;
			wxString GetPageName() const override;
	};
}