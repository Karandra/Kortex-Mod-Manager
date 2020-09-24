#include "pch.hpp"
#include "IWorkspace.h"
#include "IWorkspaceContainer.h"

namespace Kortex
{
	IWorkspace* IWorkspace::FromWindow(wxWindow& window)
	{
		if (auto clientData = dynamic_cast<Application::WorkspaceClientData*>(window.GetClientObject()))
		{
			return &clientData->GetWorkspace();
		}
		return nullptr;
	}

	void IWorkspace::ShowWorkspace()
	{
		if (IWorkspaceContainer* container = GetCurrentContainer())
		{
			container->ShowWorkspace(*this);
		}
	}
	void IWorkspace::HideWorkspace()
	{
		if (IWorkspaceContainer* container = GetCurrentContainer())
		{
			container->HideWorkspace(*this);
		}
	}

	bool IWorkspace::IsCurrent() const
	{
		if (IWorkspaceContainer* container = GetCurrentContainer())
		{
			return container->GetCurrentWorkspace() == this;
		}
		return false;
	}
	bool IWorkspace::IsActive() const
	{
		return IsCurrent() && GetWindow().IsShown();
	}
	bool IWorkspace::IsSubWorkspace() const
	{
		if (IWorkspaceContainer* container = GetCurrentContainer())
		{
			return container->IsSubContainer();
		}
		return false;
	}
	bool IWorkspace::SwitchHere()
	{
		if (IWorkspaceContainer* container = GetCurrentContainer())
		{
			return container->SwitchWorkspace(*this);
		}
		return false;
	}
}
