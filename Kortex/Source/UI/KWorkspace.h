#pragma once
#include "stdafx.h"
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxAuiToolBar.h>
#include "Application/Options/Option.h"
#include "Utility/KImageProvider.h"
class KMainWindow;
class KWorkspaceController;
class KPackageCreatorWorkspace;

namespace Kortex::ModManager
{
	class Workspace;
}

class KxMenu;
class KxMenuItem;
class KxAuiToolBarItem;

class KWorkspace: public KxPanel, public Kortex::Application::WithOptions<KWorkspace>
{
	friend class KMainWindow;
	friend class KPackageCreatorWorkspace;
	friend class Kortex::ModManager::Workspace;

	public:
		enum class TabIndex
		{
			Plugins,
			VirtualGameFolder,
			Saves,
			Downloads,
			Programs,
		};

	public:
		template<class T> static void ScheduleReloadOf()
		{
			if (T* workspace = T::GetInstance())
			{
				workspace->ScheduleReload();
			}
		}

	private:
		KMainWindow* m_MainWindow = nullptr;
		KWorkspaceController* m_WorkspaceController = nullptr;
		wxBoxSizer* m_Sizer = nullptr;
		KxAuiToolBarItem* m_ToolBarButton = nullptr;
		KxMenuItem* m_ManagersMenuItem = nullptr;
		KxMenu* m_QSM = nullptr;
		
		bool m_CreatingWorkspace = false;
		bool m_IsCreated = false;
		bool m_IsFirstTimeOpen = true;
		bool m_IsReloadSheduled = false;

	private:
		bool OnOpenWorkspaceInternal();
		bool OnCloseWorkspaceInternal();
		bool OnCreateWorkspaceInternal();
		void Init();

	protected:
		virtual wxString OnGetWindowTitle() const;
		virtual bool OnOpenWorkspace();
		virtual bool OnCloseWorkspace();
		virtual bool OnCreateWorkspace() = 0;
		
		virtual bool DoCanBeStartPage() const
		{
			return false;
		}
		
		bool MakeSubWorkspace(KWorkspace* workspace);
		KxAuiToolBarItem* CreateToolBarButton();
		KxMenuItem* CreateItemInManagersMenu();

	public:
		KWorkspace(KMainWindow* mainWindow, wxWindow* parentWindow);
		KWorkspace(KMainWindow* mainWindow);
		virtual ~KWorkspace();

	protected:
		void SwitchHereEvent(wxNotifyEvent& event);

		void SetWorkspaceController(KWorkspaceController* controller)
		{
			m_WorkspaceController = controller;
		}

	public:
		KMainWindow* GetMainWindow() const
		{
			return m_MainWindow;
		}
		
		virtual KxAuiToolBarItem* GetToolBarButton() const
		{
			return m_ToolBarButton;
		}
		virtual KxMenuItem* GetManagersMenuItem() const
		{
			return m_ManagersMenuItem;
		}
		virtual wxSizer* GetWorkspaceSizer() const = 0;

		virtual wxString GetID() const = 0;
		virtual wxString GetName() const = 0;
		virtual wxString GetNameShort() const
		{
			return GetName();
		}
		virtual KImageEnum GetImageID() const = 0;
		int GetWorkspaceIndex() const;
		KWorkspaceController* GetWorkspaceController()
		{
			return m_WorkspaceController;
		}

		virtual bool IsSubWorkspace() const
		{
			return false;
		}
		virtual wxBookCtrlBase* GetSubWorkspaceContainer()
		{
			return nullptr;
		}
		virtual bool AddSubWorkspace(KWorkspace* workspace)
		{
			return false;
		}
		virtual size_t GetTabIndex() const
		{
			return (size_t)-1;
		}
		template<class T> T* CreateAsSubWorkspace()
		{
			T* workspace = new T(KMainWindow::GetInstance());
			if (MakeSubWorkspace(workspace))
			{
				AddSubWorkspace(workspace);
			}
			return workspace;
		}

		bool HasQuickSettingsMenu() const
		{
			return GetQuickSettingsMenu() != nullptr;
		}
		virtual KxMenu* GetQuickSettingsMenu() const
		{
			return nullptr;
		}
		virtual void ShowQuickSettingsMenu()
		{
		}

		virtual bool ShouldRecieveVFSEvents() const
		{
			return true;
		}
		bool CanBeStartPage() const
		{
			return !IsSubWorkspace() && DoCanBeStartPage();
		}
		virtual bool CanReload() const
		{
			return false;
		}
		virtual bool NeedsReload() const
		{
			return false;
		}
		
		virtual void OnReloadWorkspace()
		{
		}
		virtual void RefreshWindowTitle();
		
		bool IsWorkspaceVisible() const;
		bool IsWorkspaceCreated() const
		{
			return m_IsCreated;
		}
		bool IsFirstTimeOpen() const
		{
			return m_IsFirstTimeOpen;
		}
		bool ReloadWorkspace();
		bool ReloadWorkspaceInNeeded()
		{
			if (NeedsReload())
			{
				return ReloadWorkspace();
			}
			return false;
		}
		bool SwitchHere();
		bool CreateNow();
		void ScheduleReload();
};
