#include "stdafx.h"
#include "IWorkspaceContainer.h"
#include "IMainWindow.h"
#include <Utility/Log.h>

namespace Kortex
{
	void IWorkspaceContainer::AddWorkspace(IWorkspace& workspace)
	{
		wxWindow& workspaceWindow = workspace.GetWindow();
		wxWindow& containerWindow = GetWindow();

		workspace.SetWorkspaceContainer(this);
		workspaceWindow.SetClientObject(new Application::WorkspaceClientData(workspace));
		workspaceWindow.Reparent(&containerWindow);
	}
	bool IWorkspaceContainer::RemoveWorkspace(IWorkspace& workspace)
	{
		if (&workspace.GetWorkspaceContainer() == this)
		{
			wxWindow& workspaceWindow = workspace.GetWindow();

			workspace.SetWorkspaceContainer(nullptr);
			workspaceWindow.SetClientObject(nullptr);
			workspaceWindow.Reparent(&IMainWindow::GetInstance()->GetFrame());

			return true;
		}
		return false;
	}

	bool IWorkspaceContainer::SwitchWorkspaceByID(const wxString& id)
	{
		if (IWorkspace* workspace = GetWorkspaceByID(id))
		{
			return SwitchWorkspace(*workspace);
		}
		return false;
	}
}
