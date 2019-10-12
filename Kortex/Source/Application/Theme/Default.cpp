#include "stdafx.h"
#include "Default.h"
#include <Kortex/Theme.hpp>
#include <Kortex/Application.hpp>
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxTopLevelWindow.h>
#include <KxFramework/KxSystemSettings.h>
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxStatusBarEx.h>

namespace Kortex::Theme
{
	void Default::AsWindow(wxWindow* window)
	{
		window->SetBackgroundColour(GetColor(ColorIndex::Window));
		window->SetForegroundColour(GetColor(ColorIndex::Window, ColorFlags::Foreground));
	}

	KxColor Default::GetColor(ColorIndex index, ColorFlags flags) const
	{
		switch (index)
		{
			case ColorIndex::MainWindow:
			{
				if (flags & ColorFlags::Foreground)
				{
					return KxSystemSettings::GetColor(wxSYS_COLOUR_WINDOWTEXT);
				}
				else
				{
					return KxSystemSettings::GetColor(wxSYS_COLOUR_APPWORKSPACE);
				}
			}
			case ColorIndex::Window:
			{
				if (flags & ColorFlags::Foreground)
				{
					return KxSystemSettings::GetColor(wxSYS_COLOUR_WINDOWTEXT);
				}
				else
				{
					return KxSystemSettings::GetColor(wxSYS_COLOUR_WINDOW);
				}
			}
			case ColorIndex::SplitterSash:
			{
				return KxSystemSettings::GetColor(wxSYS_COLOUR_WINDOW);
			}
			case ColorIndex::ToolBar:
			case ColorIndex::StatusBar:
			{
				if (flags & ColorFlags::Foreground)
				{
					if (flags & ColorFlags::Active)
					{
						return KxSystemSettings::GetColor(wxSYS_COLOUR_HIGHLIGHTTEXT);
					}
					else
					{
						return KxSystemSettings::GetColor(wxSYS_COLOUR_WINDOWTEXT);
					}					
				}
				else
				{
					if (flags & ColorFlags::Active)
					{
						if (IsWin8OrGreater())
						{
							return KxTopLevelWindow<>::DWMGetGlassColor().ChangeLightness(75);
						}
						else
						{
							return KxSystemSettings::GetColor(wxSYS_COLOUR_HOTLIGHT);
						}
					}
					else
					{
						if (IsWin8OrGreater())
						{
							return KxTopLevelWindow<>::DWMGetGlassColor();
						}
						else
						{
							return KxSystemSettings::GetColor(wxSYS_COLOUR_MENUHILIGHT).ChangeLightness(160);
						}
					}
				}
			}
			case ColorIndex::Caption:
			{
				if (flags & ColorFlags::Foreground)
				{
					if (flags & ColorFlags::Active)
					{
						return KxSystemSettings::GetColor(wxSYS_COLOUR_CAPTIONTEXT);
					}
					else
					{
						return KxSystemSettings::GetColor(wxSYS_COLOUR_INACTIVECAPTIONTEXT);
					}
				}
				else
				{
					if (flags & ColorFlags::Active)
					{
						return KxSystemSettings::GetColor(wxSYS_COLOUR_ACTIVECAPTION);
					}
					else
					{
						return KxSystemSettings::GetColor(wxSYS_COLOUR_INACTIVECAPTION);
					}
				}
			}
			case ColorIndex::Border:
			{
				if (flags & ColorFlags::Active)
				{
					return KxSystemSettings::GetColor(wxSYS_COLOUR_ACTIVEBORDER);
				}
				else
				{
					return KxSystemSettings::GetColor(wxSYS_COLOUR_INACTIVEBORDER);
				}
			}
		};
		return wxNullColour;
	}

	void Default::Apply(wxWindow* window)
	{
		wxWindowUpdateLocker redrawLock(window);
		AsWindow(window);
	}
	void Default::Apply(IWorkspace* workspace)
	{
		wxWindow* window = &workspace->GetWindow();
		wxWindowUpdateLocker redrawLock(window);

		AsWindow(window);
	}
	void Default::Apply(IMainWindow* mainWindow)
	{
		wxWindow* window = &mainWindow->GetFrame();
		wxWindowUpdateLocker redrawLock(window);

		window->SetBackgroundColour(GetColor(ColorIndex::MainWindow, ColorFlags::Background));
		window->SetForegroundColour(GetColor(ColorIndex::MainWindow, ColorFlags::Foreground));
	}
	void Default::Apply(KxSplitterWindow* window)
	{
		wxWindowUpdateLocker redrawLock(window);
		InheritColors(window, window->GetParent());

		window->SetSashColor(window->GetParent()->GetBackgroundColour());
	}
	void Default::Apply(KxAuiToolBar* window)
	{
		wxWindowUpdateLocker redrawLock(window);

		window->SetBorderColor(GetColor(ColorIndex::Border));
		window->SetForegroundColour(GetColor(ColorIndex::ToolBar, ColorFlags::Foreground));
		window->SetBackgroundColour(GetColor(ColorIndex::ToolBar, ColorFlags::Background));
	}
	void Default::Apply(KxStatusBarEx* window, bool isActive)
	{
		wxWindowUpdateLocker redrawLock(window);

		const ColorFlags flags = isActive ? ColorFlags::Active : ColorFlags::None;
		window->SetSeparatorsVisible(false);
		window->SetBorderColor(wxNullColour);
		window->SetBackgroundColour(GetColor(ColorIndex::StatusBar, ColorFlags::Background|flags));
		window->SetForegroundColour(GetColor(ColorIndex::StatusBar, ColorFlags::Foreground|flags));
	}
}
