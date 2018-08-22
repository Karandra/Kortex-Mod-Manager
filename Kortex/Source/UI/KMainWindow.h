#pragma once
#include "stdafx.h"
#include "KApp.h"
#include "KImageProvider.h"
#include "KProgramOptions.h"
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
class KProfile;
class KWorkspace;
class KVFSEvent;

class KMainWindow: public KxFrame, public KxSingletonPtr<KMainWindow>
{
	public:
		typedef std::unordered_map<wxString, KWorkspace*> WorkspaceInstancesMapType;

	public:
		static KxAuiToolBarItem* CreateToolBarButton(KxAuiToolBar* toolBar, const wxString& label = wxEmptyString, KImageEnum imageID = KIMG_NONE, wxItemKind kind = wxITEM_NORMAL, int index = -1);
		static wxSize GetDialogBestSize(wxWindow* dialog);
		static const void* GetUniqueID();

	private:
		KApp& m_App;
		wxBoxSizer* m_MainSizer = NULL;
		wxBoxSizer* m_ToolBarSizer = NULL;

		/* StatusBar */
		KxStatusBarEx* m_StatusBar = NULL;
		
		/* ToolBar */
		KxAuiToolBar* m_ToolBar = NULL;
		KxAuiToolBarItem* m_ToolBar_MainMenu = NULL;
		int m_ToolBar_InsertionIndex = 0;

		KxAuiToolBar* m_QuickToolBar = NULL;
		KxAuiToolBarItem* m_QuickToolBar_Login = NULL;
		KxAuiToolBarItem* m_QuickToolBar_QuickSettingsMenu = NULL;
		KxAuiToolBarItem* m_QuickToolBar_Help = NULL;

		/* Workspaces */
		WorkspaceInstancesMapType m_WorkspaceInstances;
		wxSimplebook* m_WorkspaceContainer = NULL;
		bool m_HasCurrentWorkspace = false;

		/* MainMenu */
		KxMenu* m_MainMenu = NULL;
		KxMenuItem* m_MainMenu_Settings = NULL;
		KxMenuItem* m_MainMenu_ChangeProfile = NULL;

		/* Loadable managers menu */
		KxMenu* m_ManagersMenu = NULL;

		KProgramOptionUI m_WindowOptions;

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

		void CreatePluggableManagersWorkspaces(KWorkspace* pParentWorkspace = NULL);
		void CreateMainWorkspaces();
		void CreateMainMenu();

	private:
		void OnQSMButton(KxAuiToolBarEvent& event);
		void OnWindowClose(wxCloseEvent& event);
		void OnChangeProfile(KxMenuEvent& event);

		void OnVFSToggled(KVFSEvent& event);
		void OnPluggableManagersMenuVFSToggled(KVFSEvent& event);
	
	public:
		KMainWindow();
		virtual ~KMainWindow();

	private:
		bool SwitchWorkspaceHelper(KWorkspace* nextWorkspace, KWorkspace* prevWorkspace = NULL);
		void ProcessSwitchWorkspace(KWorkspace* nextWorkspace, KWorkspace* prevWorkspace);
		KWorkspace* DoAddWorkspace(KWorkspace* workspace);

	public:
		KApp& GetApp() const
		{
			return m_App;
		}
		KxAuiToolBar* GetMainToolBar() const
		{
			return m_ToolBar;
		}
		KxAuiToolBar* GetQuickToolBar() const
		{
			return m_QuickToolBar;
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
		template<class ManagerType, class ConfigType> ManagerType* CreateWorkspaceOf(const ConfigType* config)
		{
			if (config)
			{
				ManagerType* manager = config->GetManager();
				if (manager)
				{
					KWorkspace* workspace = manager->GetWorkspace();
					if (workspace)
					{
						if (!workspace->IsWorkspaceCreated() && manager->IsActiveVFSNeeded() && !KModManager::Get().IsVFSMounted())
						{
							return NULL;
						}

						if (workspace->CreateNow())
						{
							return manager;
						}
					}
					else if (!manager->IsActiveVFSNeeded() || KModManager::Get().IsVFSMounted())
					{
						return manager;
					}
				}
			}
			return NULL;
		}

		void ClearStatus(int index = 0);
		void SetStatus(const wxString& label, int index = 0, KImageEnum image = KIMG_NONE);
		void SetStatusProgress(int current);
		void SetStatusProgress(int64_t current, int64_t total);
};
