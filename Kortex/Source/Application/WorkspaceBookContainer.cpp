#include "stdafx.h"
#include "WorkspaceBookContainer.h"
#include "Utility/Log.h"

namespace Kortex::Application
{
	bool WorkspaceBookContainer::RunSwitchSequence(IWorkspace& fromWorkspace, IWorkspace& toWorkspace)
	{
		Utility::Log::LogInfo("%1: switching from %2 to %3", __FUNCTION__, fromWorkspace.GetID(), toWorkspace.GetID());

		if (&fromWorkspace == &toWorkspace)
		{
			Utility::Log::LogInfo("Can't switch workspace to itself");
			return false;
		}
		if (!fromWorkspace.HasWorkspaceContainer() || !toWorkspace.HasWorkspaceContainer())
		{
			Utility::Log::LogInfo("Invalid workspace");
			return false;
		}

		// Close current workspace
		if (!CallOnClose(fromWorkspace))
		{
			Utility::Log::LogInfo("%1: %2 refused to close", __FUNCTION__, fromWorkspace.GetID());
			return false;
		}

		// Create next workspace if needed
		if (!toWorkspace.IsCreated() && !CallOnCreate(toWorkspace))
		{
			Utility::Log::LogInfo("%1: unable to create %2 workspace", __FUNCTION__, toWorkspace.GetID());
			return false;
		}

		// Open next workspace
		if (CallOnOpen(toWorkspace))
		{
			Utility::Log::LogInfo("%1: %2 opened. Process switching", __FUNCTION__, toWorkspace.GetID());
			return true;
		}
		else
		{
			Utility::Log::LogInfo("%1: %2 refused to open", __FUNCTION__, toWorkspace.GetID());
		}

		return false;
	}
	void WorkspaceBookContainer::ShowWorkspace(IWorkspace& workspace)
	{
		Utility::Log::LogInfo("%1: displaying workspace \"%2\"", __FUNCTION__, workspace.GetID());

		if (auto index = GetWorkspaceIndex(workspace))
		{
			GetBookCtrl().ChangeSelection(*index);
			m_HasCurrentWorkspace = true;
		}
	}
	void WorkspaceBookContainer::HideWorkspace(IWorkspace& workspace)
	{
		Utility::Log::LogInfo("%1: hiding workspace \"%2\"", __FUNCTION__, workspace.GetID());

		// Nothing to do
	}

	IWorkspace::RefVector WorkspaceBookContainer::EnumWorkspaces() const
	{
		const wxBookCtrlBase& bookCtrl = GetBookCtrl();

		IWorkspace::RefVector items;
		items.reserve(bookCtrl.GetPageCount());

		for (size_t i = 0; i < bookCtrl.GetPageCount(); i++)
		{
			if (!items.emplace_back(IWorkspace::FromWindow(bookCtrl.GetPage(i))))
			{
				items.pop_back();
			}
		}
		return items;
	}
	IWorkspace* WorkspaceBookContainer::GetWorkspaceByID(const wxString& id) const
	{
		Utility::Log::LogInfo("Attempt to convert workspace ID (%1) to workspace instance", id);

		const wxBookCtrlBase& bookCtrl = GetBookCtrl();
		for (size_t i = 0; i < bookCtrl.GetPageCount(); i++)
		{
			IWorkspace* workspace = GetWorkspaceByIndex(i);
			if (workspace && workspace->GetID() == id)
			{
				return workspace;
			}
		}
		return nullptr;
	}
	IWorkspace* WorkspaceBookContainer::GetCurrentWorkspace() const
	{
		Utility::Log::LogInfo("Attempt to get current workspace");

		if (m_HasCurrentWorkspace)
		{
			return IWorkspace::FromWindow(GetBookCtrl().GetCurrentPage());
		}
		return nullptr;
	}
	size_t WorkspaceBookContainer::GetWorkspaceCount() const
	{
		return GetBookCtrl().GetPageCount();
	}

	void WorkspaceBookContainer::AddWorkspace(IWorkspace& workspace)
	{
		wxBookCtrlBase& bookCtrl = GetBookCtrl();

		if (bookCtrl.AddPage(&workspace.GetWindow(), workspace.GetName()))
		{
			if (auto iconID = workspace.GetIcon().TryAsInt())
			{
				bookCtrl.SetPageImage(bookCtrl.GetPageCount() - 1, *iconID);
			}
			IWorkspaceContainer::AddWorkspace(workspace);
		}
	}
	bool WorkspaceBookContainer::RemoveWorkspace(IWorkspace& workspace)
	{
		if (IWorkspaceContainer::RemoveWorkspace(workspace))
		{
			if (auto index = GetWorkspaceIndex(workspace))
			{
				return GetBookCtrl().RemovePage(*index);
			}
		}
		return false;
	}
	bool WorkspaceBookContainer::SwitchWorkspace(IWorkspace& nextWorkspace)
	{
		Utility::Log::LogInfo("Attempt to switch workspace to %1", nextWorkspace.GetID());

		IWorkspace* current = GetCurrentWorkspace();
		if (current != &nextWorkspace)
		{
			return RunSwitchSequence(*current, nextWorkspace);
		}
		return true;
	}

	IWorkspace* WorkspaceBookContainer::GetWorkspaceByIndex(size_t index) const
	{
		return IWorkspace::FromWindow(GetBookCtrl().GetPage(index));
	}
	std::optional<size_t> WorkspaceBookContainer::GetWorkspaceIndex(const IWorkspace& workspace) const
	{
		int index = GetBookCtrl().FindPage(&workspace.GetWindow());
		if (index != wxNOT_FOUND)
		{
			return index;
		}
		return std::nullopt;
	}
}
