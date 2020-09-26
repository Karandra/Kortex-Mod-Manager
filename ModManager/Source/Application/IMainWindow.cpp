#include "pch.hpp"
#include "IMainWindow.h"
#include "IApplication.h"
#include "IModule.h"
#include "IManager.h"
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

	void IMainWindow::CreateWorkspaces()
	{
		IApplication::GetInstance().EnumLoadedManagers([](IManager& manager)
		{
			// Create workspace instances if we don't have them yet
			if (manager.OnCreateWorkspaces() != 0)
			{
				// Add them to preferred container if the workspace defines one
				manager.EnumWorkspaces([](IWorkspace& workspace)
				{
					IWorkspaceContainer* preferredContainer = workspace.GetPreferredContainer();
					if (preferredContainer && !workspace.GetCurrentContainer())
					{
						preferredContainer->AddWorkspace(workspace);
					}
					return true;
				});
			}
			return true;
		});
	}
}
