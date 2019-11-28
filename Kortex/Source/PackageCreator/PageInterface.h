#pragma once
#include "stdafx.h"
#include "PageBase.h"
class KxComboBox;

namespace Kortex::PackageDesigner
{
	class Workspace;
	class KPackageProjectInterface;
}

namespace Kortex::PackageDesigner::PageInterfaceNS
{
	class ImageListModel;
}

namespace Kortex::PackageDesigner
{
	class PageInterface: public PageBase
	{
		friend class Workspace;

		private:
			wxBoxSizer* m_MainSizer = nullptr;
			PageInterfaceNS::ImageListModel* m_ImageListModel = nullptr;
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
			PageInterface(Workspace& mainWorkspace, WorkspaceDocument& controller);
			virtual ~PageInterface();

		public:
			ResourceID GetIcon() const override
			{
				return ImageResourceID::Image;
			}
			wxString GetID() const override;
			wxString GetPageName() const override;
	};
}