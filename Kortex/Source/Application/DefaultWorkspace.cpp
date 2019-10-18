#include "stdafx.h"
#include "DefaultWorkspace.h"
#include "BroadcastProcessor.h"
#include "IWorkspaceContainer.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxCallAtScopeExit.h>

namespace Kortex::Application
{
	bool DefaultWorkspace::CallOnOpenWorkspace()
	{
		if (m_ReloadSheduled)
		{
			Reload();
		}

		if (OnOpenWorkspace())
		{
			m_OpenCount++;

			ShowWorkspace();

			wxWindow& window = GetWindow();
			window.Show();
			window.PostSizeEvent();
			window.PostSizeEventToParent();
			return true;
		}
		return false;
	}
	bool DefaultWorkspace::CallOnCloseWorkspace()
	{
		if (OnCloseWorkspace())
		{
			HideWorkspace();

			wxWindow& window = GetWindow();
			window.Hide();

			return true;
		}
		return false;
	}
	bool DefaultWorkspace::CallOnCreateWorkspace()
	{
		if (m_IsCreated)
		{
			return true;
		}
		else
		{
			m_InsideOnCreate = true;
			KxCallAtScopeExit atExit([this]()
			{
				m_InsideOnCreate = false;
			});

			if (OnCreateWorkspace())
			{
				m_IsCreated = true;

				// Insert window sizer into our own sizer
				wxWindow& window = GetWindow();
				wxBoxSizer* workspaceSizer = new wxBoxSizer(wxVERTICAL);

				wxSizer* windowSizer = window.GetSizer();
				window.SetSizer(workspaceSizer, false);

				if (windowSizer)
				{
					workspaceSizer->Add(windowSizer, 1, wxEXPAND|wxALL, std::min(KLC_HORIZONTAL_SPACING, KLC_VERTICAL_SPACING));
				}
				else
				{
					for (wxWindow* window: window.GetChildren())
					{
						workspaceSizer->Add(window, 1, wxEXPAND|wxALL, std::min(KLC_HORIZONTAL_SPACING, KLC_VERTICAL_SPACING));
					}
				}
				

				// Open the workspace
				return CallOnOpenWorkspace();
			}
		}
		return false;
	}
	
	void DefaultWorkspace::ApplyWorkspaceTheme()
	{
		// Do nothing
	}

	bool DefaultWorkspace::Reload()
	{
		m_ReloadSheduled = false;

		if (m_IsCreated || m_InsideOnCreate)
		{
			Reload();
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

		BroadcastProcessor::Get().CallAfter([this]()
		{
			if (IsActive())
			{
				// Reload immediately
				Reload();
			}
			else
			{
				// ReloadWorkspace will be called when workspace will be opened
				m_ReloadSheduled = true;
			}
		});
	}
	bool DefaultWorkspace::EnsureCreated()
	{
		if (!m_IsCreated)
		{
			return CallOnCreateWorkspace();
		}
		return false;
	}
}
