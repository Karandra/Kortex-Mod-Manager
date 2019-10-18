#pragma once
#include "stdafx.h"
#include "Application/DefaultWorkspace.h"
#include "Application/BookWorkspaceContainer.h"
#include "KPackageCreatorController.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxSimplebook.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxMenu.h>

namespace Kortex
{
	class IGameMod;
}
namespace Kortex::PackageDesigner
{
	class KPackageCreatorManager;
	class KPackageCreatorController;
	class KPackageCreatorPageBase;
	class KPackageCreatorPageInfo;
	class KPackageCreatorPageFileData;
	class KPackageCreatorPageInterface;
	class KPackageCreatorPageRequirements;
	class KPackageCreatorPageComponents;
	class KPackageProject;
}

namespace Kortex::PackageDesigner
{
	class KPackageCreatorWorkspace;
	class WorkspaceContainer: public Application::BookWorkspaceContainer
	{
		friend class KPackageCreatorWorkspace;

		private:
			KPackageCreatorWorkspace& m_Workspace;
			KxSimplebook* m_BookCtrl = nullptr;
			KxTreeList* m_PagesList = nullptr;

		private:
			void Create(wxWindow* listParent, wxWindow* viewParent);
			void OnPageSelected(wxTreeListEvent& event);
			
		protected:
			void ShowWorkspace(IWorkspace& workspace) override;
			void HideWorkspace(IWorkspace& workspace) override;
		
		public:
			WorkspaceContainer(KPackageCreatorWorkspace& workspace)
				:m_Workspace(workspace)
			{
			}

		public:
			wxWindow& GetWindow() override
			{
				return *m_BookCtrl;
			}
			wxWindow& GetListWindow()
			{
				return *m_PagesList;
			}

			bool AddWorkspace(IWorkspace& workspace) override;
			bool RemoveWorkspace(IWorkspace& workspace) override;
	};
}
namespace Kortex::PackageDesigner
{
	class KPackageCreatorWorkspace: public Application::DefaultWindowWorkspace<KxPanel>, public KxSingletonPtr<KPackageCreatorWorkspace>
	{
		friend class KPackageCreatorPageBase;
		friend class KPackageCreatorController;

		private:
			KPackageCreatorController m_Controller;

			KxAuiToolBar* m_MenuBar = nullptr;
			KxAuiToolBarItem* m_MenuBar_Project = nullptr;
			KxAuiToolBarItem* m_MenuBar_Import = nullptr;
			KxAuiToolBarItem* m_MenuBar_Build = nullptr;

			wxBoxSizer* m_MainSizer = nullptr;
			KxPanel* m_MainPane = nullptr;
			KxPanel* m_PagesBookPane = nullptr;

			// Pages
			WorkspaceContainer m_PagesContainer;
			KPackageCreatorPageInfo* m_PageInfo = nullptr;
			KPackageCreatorPageFileData* m_PageFileData = nullptr;
			KPackageCreatorPageInterface* m_PageInterface = nullptr;
			KPackageCreatorPageRequirements* m_PageRequirements = nullptr;
			KPackageCreatorPageComponents* m_PageComponents = nullptr;

		private:
			void CreateMenuBar(wxSizer* sizer);
			void CreateProjectMenu();
			void CreateImportMenu();
			void CreateBuildMenu();
			void CreatePagesView();

			void DoLoadAllPages();
			void OnNewProject(KxMenuEvent& event);
			void OnOpenProject(KxMenuEvent& event);
			void OnSaveProject(KxMenuEvent& event);
			void OnImportProject(KxMenuEvent& event);
			void OnExportProject(KxMenuEvent& event);
			void OnBuildProject(KxMenuEvent& event);
			void OnBuildProjectPreview(KxMenuEvent& event);

		protected:
			bool OnCreateWorkspace() override;
			bool OnOpenWorkspace() override;
			bool OnCloseWorkspace() override;
			void OnReloadWorkspace() override;

		public:
			KPackageCreatorWorkspace()
				:m_Controller(*this), m_PagesContainer(*this)
			{
			}

		public:
			using KxIObject::QueryInterface;
			bool QueryInterface(const KxIID& iid, void*& ptr) noexcept override
			{
				return QueryAnyOf(iid, ptr, *this, m_Controller, m_PagesContainer);
			}

			wxString GetID() const override;
			wxString GetName() const override;
			ResourceID GetIcon() const override
			{
				return Kortex::ImageResourceID::Box;
			}
			IWorkspaceContainer* GetPreferredContainer() const override;

			void RefreshWindowTitleForProject()
			{
				// TODO: update window title
			}
			KPackageCreatorPageBase* GetCurrentPage() const;
			KPackageProject& ImportProjectFromPackage(const wxString& path);
			KPackageProject& CreateProjectFromModEntry(const IGameMod& modEntry);
	};
}
