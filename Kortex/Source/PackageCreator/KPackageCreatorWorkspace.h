#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxButton.h>
class KModEntry;
class KPackageCreatorManager;
class KPackageCreatorController;
class KPackageCreatorPageBase;
class KPackageCreatorPageInfo;
class KPackageCreatorPageFileData;
class KPackageCreatorPageInterface;
class KPackageCreatorPageRequirements;
class KPackageCreatorPageComponents;
class KPackageProject;

class KPackageCreatorWorkspace: public KWorkspace, public KxSingletonPtr<KPackageCreatorWorkspace>
{
	friend class KPackageCreatorPageBase;
	friend class KPackageCreatorController;

	private:
		KPackageCreatorManager* m_Manager = NULL;
		KPackageCreatorController* m_Controller = NULL;

		KxAuiToolBar* m_MenuBar = NULL;
		KxAuiToolBarItem* m_MenuBar_Project = NULL;
		KxAuiToolBarItem* m_MenuBar_Import = NULL;
		KxAuiToolBarItem* m_MenuBar_Build = NULL;

		wxBoxSizer* m_MainSizer = NULL;
		KxPanel* m_MainPane = NULL;
		KxPanel* m_PagesBookPane = NULL;
		wxSimplebook* m_PagesBook = NULL;
		KxTreeList* m_PagesList = NULL;

		// Pages
		KPackageCreatorPageInfo* m_PageInfo = NULL;
		KPackageCreatorPageFileData* m_PageFileData = NULL;
		KPackageCreatorPageInterface* m_PageInterface = NULL;
		KPackageCreatorPageRequirements* m_PageRequirements = NULL;
		KPackageCreatorPageComponents* m_PageComponents = NULL;

	public:
		KPackageCreatorWorkspace(KMainWindow* mainWindow);
		virtual ~KPackageCreatorWorkspace();
		virtual bool OnCreateWorkspace() override;

	private:
		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;
		virtual void OnReloadWorkspace() override;

		void CreateMenuBar(wxSizer* sizer);
		void CreateProjectMenu();
		void CreateImportMenu();
		void CreateBuildMenu();

		void CreatePagesListView();
		void CreatePagesView();

		template<class T> T* AddPage(T* page, bool select = false)
		{
			AddPageBase(page, select);
			return page;
		}
		void AddPageBase(KPackageCreatorPageBase* page, bool select = false);
		void OnSwitchPage(wxTreeListEvent& event);
		void DoLoadAllPages();

	public:
		virtual wxString GetID() const override;
		virtual wxString GetName() const override;
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_BOX;
		}
		virtual wxSizer* GetWorkspaceSizer() const override
		{
			return m_MainSizer;
		}
		virtual bool DoCanBeStartPage() const override
		{
			return true;
		}
		virtual bool CanReload() const override
		{
			return false;
		}
		virtual bool ShouldRecieveVFSEvents() const override
		{
			return false;
		}

		KPackageCreatorPageBase* GetCurrentPage() const;
		bool SwitchToPage(size_t index);
		KPackageProject& ImportProjectFromPackage(const wxString& path);
		KPackageProject& CreateProjectFromModEntry(const KModEntry& modEntry);

	private:
		void RefreshWindowTitleForProject();
		void OnNewProject(KxMenuEvent& event);
		void OnOpenProject(KxMenuEvent& event);
		void OnSaveProject(KxMenuEvent& event);
		void OnImportProject(KxMenuEvent& event);
		void OnExportProject(KxMenuEvent& event);
		void OnBuildProject(KxMenuEvent& event);
		void OnBuildProjectPreview(KxMenuEvent& event);
};
