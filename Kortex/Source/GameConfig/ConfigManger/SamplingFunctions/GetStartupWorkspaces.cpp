#include "stdafx.h"
#include "GetStartupWorkspaces.h"
#include "Application/IMainWindow.h"
#include "Application/IWorkspace.h"

namespace Kortex::GameConfig::SamplingFunction
{
	void GetStartupWorkspaces::OnCall(const ItemValue::Vector& arguments)
	{
		const IMainWindow* mainWindow = IMainWindow::GetInstance();
		if (mainWindow)
		{
			for (const IWorkspace* workspace: mainWindow->GetWorkspaceContainer().EnumWorkspaces())
			{
				m_Values.emplace_back(workspace->GetID()).SetLabel(workspace->GetName());
			}
		}
	}
}
