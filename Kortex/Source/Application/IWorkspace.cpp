#include "stdafx.h"
#include "IWorkspace.h"
#include "IWorkspaceContainer.h"

namespace Kortex
{
	IWorkspace* IWorkspace::FromWindow(const wxWindow* window)
	{
		using namespace Application;

		if (window)
		{
			auto clientData = dynamic_cast<WorkspaceClientData*>(window->GetClientObject());
			if (clientData)
			{
				return &clientData->GetWorkspace();
			}
		}
		return nullptr;
	}

	void IWorkspace::ShowWorkspace()
	{
		GetWorkspaceContainer().ShowWorkspace(*this);
	}
	void IWorkspace::HideWorkspace()
	{
		GetWorkspaceContainer().HideWorkspace(*this);
	}

	bool IWorkspace::IsCurrent() const
	{
		return GetWorkspaceContainer().GetCurrentWorkspace() == this;
	}
	bool IWorkspace::IsActive() const
	{
		return IsCurrent() && GetWindow().IsShown();
	}
	bool IWorkspace::IsSubWorkspace() const
	{
		return HasWorkspaceContainer() && GetWorkspaceContainer().IsSubContainer();
	}
	bool IWorkspace::SwitchHere()
	{
		return GetWorkspaceContainer().SwitchWorkspace(*this);
	}
}
