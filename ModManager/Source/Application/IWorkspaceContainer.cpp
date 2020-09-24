#include "pch.hpp"
#include "IWorkspaceContainer.h"
#include "IMainWindow.h"
#include "Log.h"

namespace Kortex
{
	bool IWorkspaceContainer::AddWorkspace(IWorkspace& workspace)
	{
		auto EnsureWindowCreated = [](IWorkspace& workspace, wxWindow& containerWindow)
		{
			wxWindow& window = workspace.GetWindow();
			if (!window.GetHandle())
			{
				workspace.CreateWorkspaceWindow(containerWindow);
			}

			return window.GetHandle() != nullptr;
		};

		wxWindow& containerWindow = GetWindow();
		if (workspace.GetCurrentContainer() == nullptr && EnsureWindowCreated(workspace, containerWindow))
		{
			workspace.SetCurrentContainer(this);

			wxWindow& workspaceWindow = workspace.GetWindow();
			workspaceWindow.SetClientObject(new Application::WorkspaceClientData(workspace));
			workspaceWindow.Reparent(&containerWindow);

			return true;
		}
		return false;
	}
	bool IWorkspaceContainer::RemoveWorkspace(IWorkspace& workspace)
	{
		if (workspace.GetCurrentContainer() == this)
		{
			workspace.SetCurrentContainer(nullptr);

			wxWindow& workspaceWindow = workspace.GetWindow();
			workspaceWindow.SetClientObject(nullptr);
			workspaceWindow.Reparent(&IMainWindow::GetInstance()->GetFrame());

			return true;
		}
		return false;
	}

	bool IWorkspaceContainer::SwitchWorkspaceByID(const kxf::String& id)
	{
		if (IWorkspace* workspace = GetWorkspaceByID(id))
		{
			return SwitchWorkspace(*workspace);
		}
		return false;
	}
}
