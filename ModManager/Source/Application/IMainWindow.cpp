#include "pch.hpp"
#include "IMainWindow.h"
#include "IApplication.h"
#include "IModule.h"
#include <kxf/System/SystemInformation.h>
#include "IWorkspace.h"

namespace Kortex
{
	kxf::Size IMainWindow::GetDialogBestSize(const wxWindow& dialog)
	{
		return kxf::System::GetMetric(kxf::SystemSizeMetric::Screen).Scale(0.8);
	}

	void IMainWindow::CreateWorkspaces()
	{
		for (IModule& module: IApplication::GetInstance().EnumModules())
		{
			// Add them to preferred container if the workspace defines one
			IWorkspaceContainer& defaultContainer = GetWorkspaceContainer();
			for (IWorkspace& workspace: module.EnumWorkspaces())
			{
				if (workspace.GetCurrentContainer() == nullptr)
				{
					if (workspace.EnsureCreated())
					{
						if (IWorkspaceContainer* preferredContainer = workspace.GetPreferredContainer())
						{
							preferredContainer->AttachWorkspace(workspace);
						}
						else
						{
							defaultContainer.AttachWorkspace(workspace);
						}
					}
				}
			}
		}
	}
}
