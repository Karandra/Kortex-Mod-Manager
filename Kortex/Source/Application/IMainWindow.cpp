#include "stdafx.h"
#include "IMainWindow.h"
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

	void IMainWindow::CreateWorkspace(IManager& manager)
	{
		if (manager.EnumWorkspaces().empty())
		{
			manager.CreateWorkspace();
		}
	}
}
