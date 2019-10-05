#include "stdafx.h"
#include "VisualStudio.h"
#include <Kortex/Theme.hpp>
#include <Kortex/Application.hpp>
#include <KxFramework/KxSystemSettings.h>
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxStatusBarEx.h>

namespace Kortex::Theme
{
	void VisualStudio::AsWindow(wxWindow* window)
	{
		window->SetBackgroundColour(GetColor(ColorIndex::WindowBG));
		window->SetForegroundColour(GetColor(ColorIndex::WindowFG));
	}

	KxColor VisualStudio::GetColor(ColorIndex index) const
	{
		switch (index)
		{
			case ColorIndex::WindowBG:
			case ColorIndex::SplitterSash:
			{
				return KxColor(41, 58, 86);
			}
			case ColorIndex::WindowFG:
			{
				return KxColor(0, 0, 0);
			}
			case ColorIndex::WorkspaceBG:
			{
				return KxColor(230, 231, 232);
			}
			case ColorIndex::ToolBarBG:
			{
				return KxColor(214, 219, 233);
			}
			case ColorIndex::StatusBarBG:
			{
				return KxColor(0, 122, 204);
			}
			case ColorIndex::StatusBarActiveBG:
			{
				return KxColor(202, 81, 0);
			}
			case ColorIndex::Border:
			{
				return KxColor(142, 155, 188);
			}
		};
		return wxNullColour;
	}

	void VisualStudio::ProcessWindow(wxWindow* window)
	{
		wxWindowUpdateLocker redrawLock(window);
		AsWindow(window);
	}
	void VisualStudio::ProcessWindow(IWorkspace* workspace)
	{
		wxWindow* window = &workspace->GetWindow();
		wxWindowUpdateLocker redrawLock(window);

		AsWindow(window);
	}
	void VisualStudio::ProcessWindow(IMainWindow* mainWindow)
	{
		wxWindow* window = &mainWindow->GetFrame();
		wxWindowUpdateLocker redrawLock(window);

		window->SetBackgroundColour(GetColor(ColorIndex::WorkspaceBG));
		window->SetForegroundColour(GetColor(ColorIndex::WindowFG));
	}
	void VisualStudio::ProcessWindow(KxSplitterWindow* window, bool visibleSash)
	{
		wxWindowUpdateLocker redrawLock(window);

		AsWindow(window);
		if (!visibleSash)
		{
			window->SetSashColor(window->GetBackgroundColour());
		}
	}
	void VisualStudio::ProcessWindow(KxAuiToolBar* window)
	{
		wxWindowUpdateLocker redrawLock(window);

		window->SetBorderColor(GetColor(ColorIndex::Border));
		window->SetForegroundColour(GetColor(ColorIndex::WindowFG));
		window->SetBackgroundColour(KxColor(207, 214, 229));
	}
	void VisualStudio::ProcessWindow(KxStatusBarEx* window, bool isActive)
	{
		wxWindowUpdateLocker redrawLock(window);

		window->SetSeparatorsVisible(false);
		window->SetBorderColor(wxNullColour);
		window->SetBackgroundColour(GetColor(isActive ? ColorIndex::StatusBarActiveBG : ColorIndex::StatusBarBG));
		window->SetForegroundColour(KxColor(255, 255, 255));
	}
}
