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
		window->SetBackgroundColour(GetColor(ColorIndex::Window));
		window->SetForegroundColour(GetColor(ColorIndex::Window, ColorFlags::Foreground));
	}

	KxColor VisualStudio::GetColor(ColorIndex index, ColorFlags flags) const
	{
		switch (index)
		{
			case ColorIndex::MainWindow:
			{
				if (flags & ColorFlags::Foreground)
				{
					return {255, 255, 255};
				}
				else
				{
					return {93, 107, 153};
				}
			}
			case ColorIndex::Window:
			{
				if (flags & ColorFlags::Foreground)
				{
					return {255, 255, 255};
				}
				else
				{
					return {204, 213, 240};
				}
			}
			case ColorIndex::SplitterSash:
			{
				return {47, 54, 77};
			}
			case ColorIndex::ToolBar:
			{
				if (flags & ColorFlags::Foreground)
				{
					return {0, 0, 0};
				}
				else
				{
					return {204, 213, 240};
				}
			}
			case ColorIndex::StatusBar:
			{
				if (flags & ColorFlags::Foreground)
				{
					return {255, 255, 255};
				}
				else
				{
					if (flags & ColorFlags::Active)
					{
						return {162, 75, 64};
					}
					else
					{
						return {64, 80, 141};
					}
				}
			}
			case ColorIndex::Caption:
			{
				if (flags & ColorFlags::Foreground)
				{
					if (flags & ColorFlags::Active)
					{
						return {0, 0, 0};
					}
					else
					{
						return {255, 255, 255};
					}
				}
				else
				{
					if (flags & ColorFlags::Active)
					{
						return {245, 204, 132};
					}
					else
					{
						return {64, 86, 141};
					}
				}
			}
			case ColorIndex::Border:
			{
				return KxSystemSettings::GetColor(wxSYS_COLOUR_ACTIVEBORDER);
			}
		};
		return wxNullColour;
	}

	void VisualStudio::Apply(wxWindow* window)
	{
		wxWindowUpdateLocker redrawLock(window);
		AsWindow(window);
	}
	void VisualStudio::Apply(IWorkspace* workspace)
	{
		wxWindow* window = &workspace->GetWindow();
		wxWindowUpdateLocker redrawLock(window);

		AsWindow(window);
	}
	void VisualStudio::Apply(IMainWindow* mainWindow)
	{
		wxWindow* window = &mainWindow->GetFrame();
		wxWindowUpdateLocker redrawLock(window);

		window->SetBackgroundColour(GetColor(ColorIndex::MainWindow, ColorFlags::Background));
		window->SetForegroundColour(GetColor(ColorIndex::MainWindow, ColorFlags::Foreground));
	}
	void VisualStudio::Apply(KxSplitterWindow* window)
	{
		wxWindowUpdateLocker redrawLock(window);

		AsWindow(window);
		window->SetSashColor(window->GetParent()->GetBackgroundColour());
	}
	void VisualStudio::Apply(KxAuiToolBar* window)
	{
		wxWindowUpdateLocker redrawLock(window);

		window->SetBorderColor(GetColor(ColorIndex::Border));
		window->SetForegroundColour(GetColor(ColorIndex::ToolBar, ColorFlags::Foreground));
		window->SetBackgroundColour(GetColor(ColorIndex::ToolBar, ColorFlags::Background));
	}
	void VisualStudio::Apply(KxStatusBarEx* window, bool isActive)
	{
		wxWindowUpdateLocker redrawLock(window);
		const ColorFlags flags = isActive ? ColorFlags::Active : ColorFlags::None;

		window->SetSeparatorsVisible(false);
		window->SetBorderColor(wxNullColour);
		window->SetBackgroundColour(GetColor(ColorIndex::StatusBar, flags|ColorFlags::Background));
		window->SetForegroundColour(GetColor(ColorIndex::StatusBar, flags|ColorFlags::Foreground));
	}
}
