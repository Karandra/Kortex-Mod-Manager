#include "stdafx.h"
#include "Default.h"
#include <Kortex/Theme.hpp>
#include "UI/KMainWindow.h"
#include "UI/KWorkspace.h"
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
		window->SetBackgroundColour(GetColor(ColorIndex::WindowBG));
		window->SetForegroundColour(GetColor(ColorIndex::WindowFG));
	}

	KxColor Default::GetColor(ColorIndex index) const
	{
		switch (index)
		{
			case ColorIndex::WindowBG:
			case ColorIndex::SplitterSash:
			{
				return KxSystemSettings::GetColor(wxSYS_COLOUR_WINDOW);
			}
			case ColorIndex::WindowFG:
			{
				return KxSystemSettings::GetColor(wxSYS_COLOUR_WINDOWTEXT);
			}
			case ColorIndex::WorkspaceBG:
			{
				return KxSystemSettings::GetColor(wxSYS_COLOUR_FRAMEBK);
			}
			case ColorIndex::ToolBarBG:
			case ColorIndex::StatusBarBG:
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
			case ColorIndex::StatusBarActiveBG:
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
			case ColorIndex::Border:
			{
				return KxSystemSettings::GetColor(wxSYS_COLOUR_ACTIVEBORDER);
			}
		};
		return wxNullColour;
	}

	void Default::ProcessWindow(wxWindow* window)
	{
		wxWindowUpdateLocker redrawLock(window);
		AsWindow(window);
	}
	void Default::ProcessWindow(KWorkspace* window)
	{
		wxWindowUpdateLocker redrawLock(window);

		window->SetForegroundColour(GetColor(ColorIndex::WindowFG));
		if (window->IsSubWorkspace())
		{
			window->SetBackgroundColour(GetColor(ColorIndex::WindowBG));
		}
		else
		{
			window->SetBackgroundColour(GetColor(ColorIndex::WorkspaceBG));
		}
	}
	void Default::ProcessWindow(KMainWindow* window)
	{
		wxWindowUpdateLocker redrawLock(window);
		AsWindow(window);
		window->SetBackgroundColour(GetColor(ColorIndex::WorkspaceBG));
	}
	void Default::ProcessWindow(KxSplitterWindow* window, bool visibleSash)
	{
		wxWindowUpdateLocker redrawLock(window);
		InheritColors(window, window->GetParent());

		if (!visibleSash)
		{
			window->SetSashColor(window->GetParent()->GetBackgroundColour());
		}
	}
	void Default::ProcessWindow(KxAuiToolBar* window)
	{
		wxWindowUpdateLocker redrawLock(window);

		window->SetBorderColor(GetColor(ColorIndex::Border));
		window->SetForegroundColour(GetColor(ColorIndex::WindowFG));
		window->SetBackgroundColour(GetColor(ColorIndex::ToolBarBG));
	}
	void Default::ProcessWindow(KxStatusBarEx* window, bool isActive)
	{
		wxWindowUpdateLocker redrawLock(window);

		window->SetBackgroundColour(GetColor(isActive ? ColorIndex::StatusBarActiveBG : ColorIndex::StatusBarBG));
		window->SetSeparatorsVisible(false);
		window->SetBorderColor(wxNullColour);

		KxColor textColor = GetColor(ColorIndex::WindowFG);
		window->SetForegroundColour(isActive ? textColor.Negate() : textColor);
	}
}
