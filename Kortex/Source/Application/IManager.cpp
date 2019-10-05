#include "stdafx.h"
#include "IManager.h"
#include "IWorkspace.h"
#include "Resources/IImageProvider.h"
#include <KxFramework/KxAuiToolBar.h>

namespace Kortex
{
	void IManager::ScheduleWorkspacesReload()
	{
		for (IWorkspace* workspace: EnumWorkspaces())
		{
			workspace->ScheduleReload();
		}
	}
}

namespace Kortex::Application
{
	KxAuiToolBarItem& ManagerWithToolbarButton::AddToolbarButton(KxAuiToolBar& toolbar, const ResourceID& image)
	{
		KxAuiToolBarItem* button = toolbar.AddTool(wxEmptyString, ImageProvider::GetBitmap(image), wxITEM_NORMAL);
		button->Bind(KxEVT_AUI_TOOLBAR_CLICK, &ManagerWithToolbarButton::OnToolbarButton, this);
		OnSetToolbarButton(*button);

		return *button;
	}
}
