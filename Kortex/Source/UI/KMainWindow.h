#pragma once
#include "stdafx.h"
#include "Application/Options/Option.h"
#include "Utility/KImageProvider.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxFrame.h>
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxListView.h>
#include <KxFramework/KxImageView.h>
#include <KxFramework/KxToolBar.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxProgressBar.h>
#include <KxFramework/KxStatusBarEx.h>
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxColor.h>
class KWorkspace;

namespace Kortex
{
	class VirtualFileSystemEvent;
}

class KMainWindow:
	public KxFrame,
	public KxSingletonPtr<KMainWindow>,
	public Kortex::Application::WithOptions<KMainWindow>
{
	public:
		typedef std::unordered_map<wxString, KWorkspace*> WorkspaceInstancesMapType;

	public:
		static KxAuiToolBarItem* CreateToolBarButton(KxAuiToolBar* toolBar, const wxString& label = wxEmptyString, KImageEnum imageID = KIMG_NONE, wxItemKind kind = wxITEM_NORMAL, int index = -1);
		static wxSize GetDialogBestSize(wxWindow* dialog);
		static const void* GetUniqueID();

	private:
		template<class T> static KxAuiToolBarItem* AddToolBarButton(KxAuiToolBar* toolBar, KImageEnum imageID = KIMG_NONE)
		{
			KxAuiToolBarItem* button = toolBar->AddTool(wxEmptyString, KGetBitmap(imageID), wxITEM_NORMAL);
			button->Bind(KxEVT_AUI_TOOLBAR_CLICK, &T::CallOnToolBarButton, T::GetInstance());
			T::GetInstance()->OnSetToolBarButton(button);
			return button;
		}

	private:
		wxBoxSizer* m_MainSizer = nullptr;
		wxBoxSizer* m_ToolBarSizer = nullptr;

		/* StatusBar */
		KxStatusBarEx* m_StatusBar = nullptr;
		
		/* ToolBar */
		KxAuiToolBar* m_ToolBar = nullptr;
		KxAuiToolBarItem* m_ToolBar_MainMenu = nullptr;
		int m_ToolBar_InsertionIndex = 0;

		KxAuiToolBar* m_QuickToolBar = nullptr;
		KxAuiToolBarItem* m_QuickToolBar_QuickSettingsMenu = nullptr;
		KxAuiToolBarItem* m_QuickToolBar_Help = nullptr;

		/* Workspaces */
		WorkspaceInstancesMapType m_WorkspaceInstances;
		wxSimplebook* m_WorkspaceContainer = nullptr;
		bool m_HasCurrentWorkspace = false;

		/* Loadable managers menu */
		KxMenu* m_ManagersMenu = nullptr;

	private:
		void CreateToolBar();
		void CreateStatusBar();
		void CreateBaseLayout();
		virtual WXLRESULT MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam) override;

	private:
		bool Create(wxWindow* parent,
					wxWindowID id,
					const wxString& caption,
					const wxPoint& pos = wxDefaultPosition,
					const wxSize& size = wxDefaultSize,
					long style = DefaultStyle
		);

		void CreatePluggableManagersWorkspaces(KWorkspace* parentWorkspace = nullptr);
		void CreateMainWorkspaces();
		void CreateMainMenu(KxMenu& mainMenu);

	private:
		void OnQSMButton(KxAuiToolBarEvent& event);
		void OnWindowClose(wxCloseEvent& event);
		void OnChangeInstance(KxMenuEvent& event);

		void OnVFSToggled(Kortex::VirtualFileSystemEvent& event);
		void OnPluggableManagersMenuVFSToggled(Kortex::VirtualFileSystemEvent& event);
	
	public:
		KMainWindow();
		virtual ~KMainWindow();

	private:
		bool SwitchWorkspaceHelper(KWorkspace* nextWorkspace, KWorkspace* prevWorkspace = nullptr);
		void ProcessSwitchWorkspace(KWorkspace* nextWorkspace, KWorkspace* prevWorkspace);
		KWorkspace* DoAddWorkspace(KWorkspace* workspace);

	public:
		KxAuiToolBar* GetMainToolBar() const
		{
			return m_ToolBar;
		}
		KxAuiToolBar* GetQuickToolBar() const
		{
			return m_QuickToolBar;
		}
		KxStatusBarEx* GetStatusBar() const
		{
			return m_StatusBar;
		}
		KxMenu* GetManagersMenu() const
		{
			return m_ManagersMenu;
		}
		int GetToolBarInsertionIndex() const
		{
			return m_ToolBar_InsertionIndex;
		}

		const WorkspaceInstancesMapType& GetWorkspacesList() const
		{
			return m_WorkspaceInstances;
		}
		wxSimplebook* GetWorkspaceContainer() const
		{
			return m_WorkspaceContainer;
		}
		template<class T> T* AddWorkspace(T* workspace)
		{
			DoAddWorkspace(workspace);
			return workspace;
		}
		
		KWorkspace* GetWorkspace(const wxString& id) const;
		KWorkspace* GetCurrentWorkspace() const;
		KWorkspace* GetFirstWorkspace() const;
		bool SwitchWorkspace(KWorkspace* nextWorkspace);
		bool SwitchWorkspace(const wxString& id);

		void ClearStatus(int index = 0);
		void SetStatus(const wxString& label, int index = 0, KImageEnum image = KIMG_NONE);
		void SetStatusProgress(int current);
		void SetStatusProgress(int64_t current, int64_t total);
};
