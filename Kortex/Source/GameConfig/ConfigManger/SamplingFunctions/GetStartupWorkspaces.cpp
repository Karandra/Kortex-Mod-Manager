#include "stdafx.h"
#include "GetStartupWorkspaces.h"
#include "UI/KMainWindow.h"
#include "UI/KWorkspace.h"

namespace Kortex::GameConfig::SamplingFunction
{
	void GetStartupWorkspaces::OnCall(const ItemValue::Vector& arguments)
	{
		const KMainWindow* mainWindow = KMainWindow::GetInstance();
		if (mainWindow)
		{
			for (const auto&[id, workspace]: mainWindow->GetWorkspacesList())
			{
				if (workspace->CanBeStartPage())
				{
					m_Values.emplace_back(id).SetLabel(workspace->GetNameShort());
				}
			}
		}
	}
}
