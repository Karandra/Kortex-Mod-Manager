#pragma once
#include "stdafx.h"
#include "PageBase.h"
class KxComboBox;

namespace Kortex::PackageProject
{
	class InterfaceSection;
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
			PackageProject::InterfaceSection& GetProjectInterface() const;
			void OnLoadProject(PackageProject::InterfaceSection& projectInterface);
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