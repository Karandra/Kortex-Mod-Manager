#include "pch.hpp"
#include "IWorkspaceContainer.h"
#include "IApplication.h"
#include "IMainWindow.h"
#include "Log.h"

namespace Kortex
{
	bool IWorkspaceContainer::ContainsWorkspace(const IWorkspace& workspace) const
	{
		return workspace.GetCurrentContainer() == this;
	}

	bool IWorkspaceContainer::AttachWorkspace(IWorkspace& workspace)
	{
		if (ContainsWorkspace(workspace))
		{
			return false;
		}

		auto EnsureWindowCreated = [](IWorkspace& workspace, wxWindow& containerWindow)
		{
			wxWindow& window = *workspace.GetWidget().GetWxWindow();
			if (!window.GetHandle())
			{
				workspace.DoCreateWorkspaceWindow(containerWindow);
			}

			return window.GetHandle() != nullptr;
		};

		wxWindow& containerWindow = *GetWidget().GetWxWindow();
		if (!workspace.GetCurrentContainer() && EnsureWindowCreated(workspace, containerWindow))
		{
			workspace.DoSetCurrentContainer(this);

			wxWindow& workspaceWindow = *workspace.GetWidget().GetWxWindow();
			workspaceWindow.SetClientObject(std::make_unique<Application::WorkspaceClientData>(workspace).release());
			workspaceWindow.Reparent(&containerWindow);

			workspace.OnWorkspaceAttached();
			return true;
		}
		return false;
	}
	bool IWorkspaceContainer::DetachWorkspace(IWorkspace& workspace)
	{
		if (ContainsWorkspace(workspace))
		{
			workspace.DoSetCurrentContainer(nullptr);

			wxWindow& workspaceWindow = *workspace.GetWidget().GetWxWindow();
			workspaceWindow.SetClientObject(nullptr);
			workspaceWindow.Reparent(IApplication::GetInstance().GetMainWindow()->GetWidget().GetWxWindow());

			workspace.OnWorkspaceDetached();
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
