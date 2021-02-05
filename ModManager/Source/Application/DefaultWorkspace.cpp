#include "pch.hpp"
#include "DefaultWorkspace.h"
#include "BroadcastProcessor.h"
#include "IWorkspaceContainer.h"
#include <kxf/Utility/ScopeGuard.h>

namespace Kortex::Application
{
	// DefaultWorkspace
	void DefaultWorkspace::ApplyWorkspaceTheme()
	{
		// Do nothing
	}

	// IWorkspace
	bool DefaultWorkspace::DoOnOpenWorkspace()
	{
		if (m_ReloadSheduled)
		{
			Reload();
		}

		if (OnOpenWorkspace())
		{
			wxWindow& window = GetWindow();
			window.Show();
			window.PostSizeEvent();
			window.PostSizeEventToParent();

			Show();
			m_OpenCount++;
			return true;
		}
		return false;
	}
	bool DefaultWorkspace::DoOnCloseWorkspace()
	{
		if (OnCloseWorkspace())
		{
			wxWindow& window = GetWindow();
			window.Hide();

			Hide();
			return true;
		}
		return false;
	}
	bool DefaultWorkspace::DoOnCreateWorkspace()
	{
		if (m_IsCreated)
		{
			return true;
		}
		else
		{
			m_InsideOnCreate = true;
			kxf::Utility::ScopeGuard atExit([&]()
			{
				m_InsideOnCreate = false;
			});

			if (OnCreateWorkspace())
			{
				m_IsCreated = true;

				// Insert window's sizer into our own sizer
				wxWindow& window = GetWindow();
				wxBoxSizer* workspaceSizer = new wxBoxSizer(wxVERTICAL);

				wxSizer* windowSizer = window.GetSizer();
				window.SetSizer(workspaceSizer, false);

				if (windowSizer)
				{
					workspaceSizer->Add(windowSizer, 1, wxEXPAND|wxALL, std::min(LayoutConstant::HorizontalSpacing, LayoutConstant::VerticalSpacing));
				}
				else
				{
					for (wxWindow* window: window.GetChildren())
					{
						workspaceSizer->Add(window, 1, wxEXPAND|wxALL, std::min(LayoutConstant::HorizontalSpacing, LayoutConstant::VerticalSpacing));
					}
				}

				// Open the workspace
				return DoOnOpenWorkspace();
			}
		}
		return false;
	}
	
	// IWorkspace
	bool DefaultWorkspace::Reload()
	{
		m_ReloadSheduled = false;

		if (m_IsCreated || m_InsideOnCreate)
		{
			OnReloadWorkspace();
			return true;
		}
		return false;
	}
	void DefaultWorkspace::ScheduleReload()
	{
		if (!m_IsCreated && !m_ReloadSheduled)
		{
			return;
		}

		QueryInterface<kxf::IEvtHandler>()->CallAfter([this]()
		{
			if (IsActive())
			{
				// Reload immediately
				Reload();
			}
			else
			{
				// 'ReloadWorkspace' will be called when workspace will be opened
				m_ReloadSheduled = true;
			}
		});
	}
	bool DefaultWorkspace::EnsureCreated()
	{
		if (!m_IsCreated)
		{
			return DoOnCreateWorkspace();
		}
		return false;
	}
}
