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
class KPackageCreatorManager;
class KPackageCreatorController;
class KPackageCreatorPageBase;
class KPackageCreatorPageInfo;
class KPackageCreatorPageFileData;
class KPackageCreatorPageInterface;
class KPackageCreatorPageRequirements;
class KPackageCreatorPageComponents;
class KPackageProject;

namespace Kortex
{
	class IGameMod;
}

class KPackageCreatorWorkspace: public KWorkspace, public KxSingletonPtr<KPackageCreatorWorkspace>
{
	friend class KPackageCreatorPageBase;
	friend class KPackageCreatorController;

	private:
		KPackageCreatorManager* m_Manager = nullptr;
		KPackageCreatorController* m_Controller = nullptr;

		KxAuiToolBar* m_MenuBar = nullptr;
		KxAuiToolBarItem* m_MenuBar_Project = nullptr;
		KxAuiToolBarItem* m_MenuBar_Import = nullptr;
		KxAuiToolBarItem* m_MenuBar_Build = nullptr;

		wxBoxSizer* m_MainSizer = nullptr;
		KxPanel* m_MainPane = nullptr;
		KxPanel* m_PagesBookPane = nullptr;
		wxSimplebook* m_PagesBook = nullptr;
		KxTreeList* m_PagesList = nullptr;

		// Pages
		KPackageCreatorPageInfo* m_PageInfo = nullptr;
		KPackageCreatorPageFileData* m_PageFileData = nullptr;
		KPackageCreatorPageInterface* m_PageInterface = nullptr;
		KPackageCreatorPageRequirements* m_PageRequirements = nullptr;
		KPackageCreatorPageComponents* m_PageComponents = nullptr;

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
		KPackageProject& CreateProjectFromModEntry(const Kortex::IGameMod& modEntry);

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
