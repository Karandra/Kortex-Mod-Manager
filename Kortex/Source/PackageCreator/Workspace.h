#pragma once
#include "stdafx.h"
#include "Application/DefaultWorkspace.h"
#include "Application/BookWorkspaceContainer.h"
#include "WorkspaceDocument.h"
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
	class KPackageProject;
}
namespace Kortex::PackageDesigner
{
	class WorkspaceDocument;

	class PageBase;
	class PageInfo;
	class PageFileData;
	class PageInterface;
	class PageRequirements;
	class PageComponents;
}

namespace Kortex::PackageDesigner
{
	class Workspace;
	class WorkspaceContainer: public Application::BookWorkspaceContainer
	{
		friend class Workspace;

		private:
			Workspace& m_Workspace;
			KxSimplebook* m_BookCtrl = nullptr;
			KxTreeList* m_PagesList = nullptr;

		private:
			void Create(wxWindow* listParent, wxWindow* viewParent);
			void OnPageSelected(wxTreeListEvent& event);
			
		protected:
			void ShowWorkspace(IWorkspace& workspace) override;
			void HideWorkspace(IWorkspace& workspace) override;
		
		public:
			WorkspaceContainer(Workspace& workspace)
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
	class Workspace: public Application::DefaultWindowWorkspace<KxPanel>, public KxSingletonPtr<Workspace>
	{
		friend class PageBase;
		friend class WorkspaceDocument;

		private:
			WorkspaceDocument m_WorkspaceDocument;

			KxAuiToolBar* m_MenuBar = nullptr;
			KxAuiToolBarItem* m_MenuBar_Project = nullptr;
			KxAuiToolBarItem* m_MenuBar_Import = nullptr;
			KxAuiToolBarItem* m_MenuBar_Build = nullptr;

			KxSplitterWindow* m_SplitterLeftRight = nullptr;
			KxPanel* m_RightPane = nullptr;

			// Pages
			WorkspaceContainer m_PagesContainer;
			PageInfo* m_PageInfo = nullptr;
			PageFileData* m_PageFileData = nullptr;
			PageInterface* m_PageInterface = nullptr;
			PageRequirements* m_PageRequirements = nullptr;
			PageComponents* m_PageComponents = nullptr;

		private:
			void CreateMenuBar(wxSizer* sizer);
			void CreateProjectMenu();
			void CreateImportMenu();
			void CreateBuildMenu();

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
			Workspace();
			~Workspace();

		public:
			using KxIObject::QueryInterface;
			bool QueryInterface(const KxIID& iid, void*& ptr) noexcept override;

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
			PageBase* GetCurrentPage() const;
			KPackageProject& ImportProjectFromPackage(const wxString& path);
			KPackageProject& CreateProjectFromModEntry(const IGameMod& modEntry);
	};
}
