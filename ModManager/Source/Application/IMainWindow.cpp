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
		IApplication::GetInstance().EnumLoadedModules([&](IModule& module)
		{
			// Add them to preferred container if the workspace defines one
			IWorkspaceContainer& defaultContainer = GetWorkspaceContainer();
			module.EnumWorkspaces([&](IWorkspace& workspace)
			{
				IWorkspaceContainer* preferredContainer = workspace.GetPreferredContainer();
				if (workspace.GetCurrentContainer() == nullptr)
				{
					if (workspace.EnsureCreated())
					{
						if (preferredContainer)
						{
							preferredContainer->AttachWorkspace(workspace);
						}
						else
						{
							defaultContainer.AttachWorkspace(workspace);
						}
					}
				}
				return true;
			});
			return true;
		});
	}
}
