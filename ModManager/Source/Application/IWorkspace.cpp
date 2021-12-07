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

	kxf::String IWorkspace::GetID() const
	{
		using namespace kxf::RTTI;

		auto classInfo = QueryInterface<ClassInfo>();
		return classInfo->GetTraits().Contains(ClassTrait::Implementation) ? classInfo->GetFullyQualifiedName() : kxf::NullString;
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
		return IsCurrent() && GetWidget().IsDisplayed();
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
	void IWorkspace::Show()
	{
		if (IWorkspaceContainer* container = GetCurrentContainer())
		{
			container->ShowWorkspace(*this);
		}
	}
	void IWorkspace::Hide()
	{
		if (IWorkspaceContainer* container = GetCurrentContainer())
		{
			container->HideWorkspace(*this);
		}
	}
}
