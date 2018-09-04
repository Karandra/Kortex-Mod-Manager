#include "stdafx.h"
#include "KThemeDefault.h"
#include "UI/KMainWindow.h"
#include "UI/KWorkspace.h"
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxTopLevelWindow.h>
#include <KxFramework/KxSystemSettings.h>
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxStatusBarEx.h>

KThemeDefault::KThemeDefault()
{
}
KThemeDefault::~KThemeDefault()
{
}

void KThemeDefault::AsWindow(wxWindow* window)
{
	window->SetBackgroundColour(GetColor(KTMC_WINDOW_BG));
	window->SetForegroundColour(GetColor(KTMC_WINDOW_FG));
}

KxColor KThemeDefault::GetColor(KThemeManagerColors index)
{
	switch (index)
	{
		case KTMC_WINDOW_BG:
		case KTMC_SASH_BG:
		{
			return KxSystemSettings::GetColor(wxSYS_COLOUR_WINDOW);
		}
		case KTMC_WINDOW_FG:
		{
			return KxSystemSettings::GetColor(wxSYS_COLOUR_WINDOWTEXT);
		}
		case KTMC_WORKSPACE_BG:
		{
			return KxSystemSettings::GetColor(wxSYS_COLOUR_FRAMEBK);
		}
		case KTMC_TOOLBAR_BG:
		case KTMC_STATUSBAR_BG:
		{
			if (IsWindows8OrGreater())
			{
				return KxTopLevelWindow<>::DWMGetGlassColor();
			}
			else
			{
				return KxSystemSettings::GetColor(wxSYS_COLOUR_MENUHILIGHT).ChangeLightness(160);
			}
		}
		case KTMC_STATUSBAR_ACTIVE_BG:
		{
			if (IsWindows8OrGreater())
			{
				return KxTopLevelWindow<>::DWMGetGlassColor().ChangeLightness(75);
			}
			else
			{
				return KxSystemSettings::GetColor(wxSYS_COLOUR_HOTLIGHT);
			}
		}
		case KTMC_BORDER:
		{
			return KxSystemSettings::GetColor(wxSYS_COLOUR_ACTIVEBORDER);
		}
	};
	return wxNullColour;
}

void KThemeDefault::ProcessWindow(wxWindow* window)
{
	wxWindowUpdateLocker redrawLock(window);
	AsWindow(window);
}
void KThemeDefault::ProcessWindow(KWorkspace* window)
{
	wxWindowUpdateLocker redrawLock(window);

	window->SetForegroundColour(GetColor(KTMC_WINDOW_FG));
	if (window->IsSubWorkspace())
	{
		window->SetBackgroundColour(GetColor(KTMC_WINDOW_BG));
	}
	else
	{
		window->SetBackgroundColour(GetColor(KTMC_WORKSPACE_BG));
	}
}
void KThemeDefault::ProcessWindow(KMainWindow* window)
{
	wxWindowUpdateLocker redrawLock(window);
	AsWindow(window);
	window->SetBackgroundColour(GetColor(KTMC_WORKSPACE_BG));
}
void KThemeDefault::ProcessWindow(KxSplitterWindow* window, bool visibleSash)
{
	wxWindowUpdateLocker redrawLock(window);
	InheritColors(window, window->GetParent());

	if (!visibleSash)
	{
		window->SetSashColor(window->GetParent()->GetBackgroundColour());
	}
}
void KThemeDefault::ProcessWindow(KxAuiToolBar* window)
{
	wxWindowUpdateLocker redrawLock(window);

	window->SetBorderColor(GetColor(KTMC_BORDER));
	window->SetForegroundColour(GetColor(KTMC_WINDOW_FG));
	window->SetBackgroundColour(GetColor(KTMC_TOOLBAR_BG));
}
void KThemeDefault::ProcessWindow(KxStatusBarEx* window, bool isActive)
{
	wxWindowUpdateLocker redrawLock(window);

	window->SetBackgroundColour(GetColor(isActive ? KTMC_STATUSBAR_ACTIVE_BG : KTMC_STATUSBAR_BG));
	window->SetSeparatorsVisible(false);
	window->SetBorderColor(wxNullColour);
	
	KxColor textColor = GetColor(KTMC_WINDOW_FG);
	window->SetForegroundColour(isActive ? textColor.Negate() : textColor);
}
