#include "pch.hpp"
#include "BookWorkspaceContainer.h"
#include "Log.h"
#include <kxf/UI/Controls/Simplebook.h>
#include <kxf/UI/Controls/AUI/AuiNotebook.h>

namespace Kortex::Application
{
	bool BookWorkspaceContainer::DoInsertWorkspacePage(IWorkspace& workspace, size_t index)
	{
		wxBookCtrlBase& bookCtrl = GetBookCtrl();
		if (bookCtrl.InsertPage(index, &workspace.GetWindow(), workspace.GetName(), false))
		{
			if (auto iconID = workspace.GetIcon().ToInt())
			{
				bookCtrl.SetPageImage(index, *iconID);
			}
			return true;
		}
		return false;
	}

	bool BookWorkspaceContainer::DoSwitchWorkspace(IWorkspace* fromWorkspace, IWorkspace& toWorkspace)
	{
		Log::Info("{}: switching from {} to {}", __FUNCTION__, fromWorkspace ? fromWorkspace->GetID() : "null", toWorkspace.GetID());

		if (fromWorkspace == &toWorkspace)
		{
			Log::Info("Can't switch workspace to itself");
			return false;
		}
		if (fromWorkspace && (!fromWorkspace->GetCurrentContainer() || !toWorkspace.GetCurrentContainer()))
		{
			Log::Info("Invalid workspace");
			return false;
		}

		// Close current workspace
		if (fromWorkspace && !CallOnClose(*fromWorkspace))
		{
			Log::Info("{}: {} refused to close", __FUNCTION__, fromWorkspace->GetID());
			return false;
		}

		// Create next workspace if needed
		if (!toWorkspace.IsCreated() && !CallOnCreate(toWorkspace))
		{
			Log::Info("{}: unable to create {} workspace", __FUNCTION__, toWorkspace.GetID());
			return false;
		}

		// Open next workspace
		if (CallOnOpen(toWorkspace))
		{
			Log::Info("{}: {} opened. Process switching", __FUNCTION__, toWorkspace.GetID());
			return true;
		}
		else
		{
			Log::Info("{}: {} refused to open", __FUNCTION__, toWorkspace.GetID());
		}

		return false;
	}
	void BookWorkspaceContainer::ShowWorkspace(IWorkspace& workspace)
	{
		Log::Info("{}: displaying workspace \"{}\"", __FUNCTION__, workspace.GetID());

		if (auto index = GetWorkspaceIndex(workspace))
		{
			GetBookCtrl().ChangeSelection(*index);
			m_HasCurrentWorkspace = true;
		}
	}
	void BookWorkspaceContainer::HideWorkspace(IWorkspace& workspace)
	{
		Log::Info("{}: hiding workspace \"{}\"", __FUNCTION__, workspace.GetID());

		// Nothing to do
	}

	IWorkspace* BookWorkspaceContainer::GetWorkspaceByID(const kxf::String& id) const
	{
		Log::Info("Attempt to convert workspace ID ({}) to workspace instance", id);

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
	IWorkspace* BookWorkspaceContainer::GetWorkspaceByIndex(size_t index) const
	{
		if (wxWindow* window = GetBookCtrl().GetPage(index))
		{
			return IWorkspace::FromWindow(*window);
		}
		return nullptr;
	}
	IWorkspace* BookWorkspaceContainer::GetCurrentWorkspace() const
	{
		Log::Info("Attempt to get current workspace");

		wxWindow* window = GetBookCtrl().GetCurrentPage();
		if (m_HasCurrentWorkspace && window)
		{
			return IWorkspace::FromWindow(*window);
		}
		return nullptr;
	}
	
	size_t BookWorkspaceContainer::GetWorkspacesCount() const
	{
		return GetBookCtrl().GetPageCount();
	}
	size_t BookWorkspaceContainer::EnumWorkspaces(std::function<bool(IWorkspace&)> func)
	{
		size_t count = 0;
		const wxBookCtrlBase& bookCtrl = GetBookCtrl();

		for (size_t i = 0; i < bookCtrl.GetPageCount(); i++)
		{
			if (wxWindow* window = bookCtrl.GetPage(i))
			{
				if (IWorkspace* workspace = IWorkspace::FromWindow(*window))
				{
					count++;
					if (!std::invoke(func, *workspace))
					{
						break;
					}
				}
			}
		}
		return count;
	}
	std::optional<size_t> BookWorkspaceContainer::GetWorkspaceIndex(const IWorkspace& workspace) const
	{
		const wxBookCtrlBase& bookCtrl = GetBookCtrl();

		int index = wxNOT_FOUND;
		if (bookCtrl.IsKindOf(wxCLASSINFO(wxAuiNotebook)))
		{
			// wxAuiNotebook implements pages sifferentely and, for some weird reason, 'wxBookCtrlBase::FindPage' isn't virtual.
			index = static_cast<const wxAuiNotebook&>(bookCtrl).GetPageIndex(const_cast<wxWindow*>(&workspace.GetWindow()));
		}
		else
		{
			index = bookCtrl.FindPage(&workspace.GetWindow());
		}

		if (index != wxNOT_FOUND)
		{
			return index;
		}
		return {};
	}
	bool BookWorkspaceContainer::ChangeWorkspaceIndex(IWorkspace& workspace, size_t newIndex)
	{
		if (auto currentIndex = GetWorkspaceIndex(workspace))
		{
			wxBookCtrlBase& bookCtrl = GetBookCtrl();
			if (currentIndex != newIndex && newIndex < bookCtrl.GetPageCount())
			{
				return bookCtrl.RemovePage(*currentIndex) && DoInsertWorkspacePage(workspace, newIndex);
			}
		}
		return false;
	}

	bool BookWorkspaceContainer::AttachWorkspace(IWorkspace& workspace)
	{
		if (IWorkspaceContainer::AttachWorkspace(workspace))
		{
			return DoInsertWorkspacePage(workspace, GetBookCtrl().GetPageCount());
		}
		else
		{
			IWorkspaceContainer::DetachWorkspace(workspace);
			return false;
		}
	}
	bool BookWorkspaceContainer::DetachWorkspace(IWorkspace& workspace)
	{
		if (IWorkspaceContainer::DetachWorkspace(workspace))
		{
			if (auto index = GetWorkspaceIndex(workspace))
			{
				return GetBookCtrl().RemovePage(*index);
			}
		}
		return false;
	}
	bool BookWorkspaceContainer::SwitchWorkspace(IWorkspace& nextWorkspace)
	{
		Log::Info("Attempt to switch workspace to {}", nextWorkspace.GetID());

		return DoSwitchWorkspace(GetCurrentWorkspace(), nextWorkspace);
	}
}
