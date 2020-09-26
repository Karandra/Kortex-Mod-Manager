#include "pch.hpp"
#include "IMainWindow.h"
#include "IApplication.h"
#include <kxf/System/SystemInformation.h>
#include "IWorkspace.h"

namespace Kortex
{
	IMainWindow* IMainWindow::GetInstance() noexcept
	{
		return IApplication::GetInstance().GetMainWindow();
	}
	kxf::Size IMainWindow::GetDialogBestSize(const wxWindow& dialog)
	{
		return kxf::System::GetMetric(kxf::SystemSizeMetric::Screen).Scale(0.8);
	}

	void IMainWindow::InitializeWorkspaces()
	{
		#if 0
		for (IModule* module: IModule::GetInstances())
		{
			for (IManager* manager: module->GetManagers())
			{
				// Create workspace instances if we don't have them yet
				IWorkspace::RefVector workspaces = manager->EnumWorkspaces();
				if (workspaces.empty())
				{
					manager->CreateWorkspaces();
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
		#endif
	}
}
