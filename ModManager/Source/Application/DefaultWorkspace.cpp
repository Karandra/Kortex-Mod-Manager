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
			auto& widget = GetWidget();
			widget.Show();

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
			auto& widget = GetWidget();
			widget.Hide();

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
				auto& widget = GetWidget();
				wxBoxSizer* workspaceSizer = new wxBoxSizer(wxVERTICAL);

				wxSizer* windowSizer = widget.GetWxWindow()->GetSizer();
				widget.GetWxWindow()->SetSizer(workspaceSizer, false);

				if (windowSizer)
				{
					workspaceSizer->Add(windowSizer, 1, wxEXPAND|wxALL, std::min(LayoutConstant::HorizontalSpacing, LayoutConstant::VerticalSpacing));
				}
				else
				{
					for (auto childWidget: widget.EnumChildWidgets())
					{
						workspaceSizer->Add(childWidget->GetWxWindow(), 1, wxEXPAND|wxALL, std::min(LayoutConstant::HorizontalSpacing, LayoutConstant::VerticalSpacing));
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
