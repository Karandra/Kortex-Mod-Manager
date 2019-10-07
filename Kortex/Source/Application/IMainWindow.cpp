#include "stdafx.h"
#include "IMainWindow.h"
#include "IWorkspace.h"
#include "IManager.h"

namespace Kortex
{
	wxSize IMainWindow::GetDialogBestSize(const wxWindow* dialog)
	{
		int screenWidth = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
		int screenHeight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
		float scaleX = 0.85f;
		float scaleY = scaleX;

		return wxSize(screenWidth * scaleX, screenHeight * scaleY);
	}

	void IMainWindow::InitializeWorkspaces()
	{
		for (IModule* module: IModule::GetInstances())
		{
			for (IManager* manager: module->GetManagers())
			{
				// Create workspace instances if we don't have them yet
				IWorkspace::RefVector workspaces = manager->EnumWorkspaces();
				if (workspaces.empty())
				{
					manager->CreateWorkspace();
				}

				// Add them to containers if the workspace isn't added
				workspaces = manager->EnumWorkspaces();
				for (IWorkspace* workspace: workspaces)
				{
					IWorkspaceContainer* preferredContainer = workspace->GetPreferredContainer();
					if (preferredContainer && !workspace->GetCurrentContainer())
					{
						preferredContainer->AddWorkspace(*workspace);
					}
				}
			}
		}
	}
}
