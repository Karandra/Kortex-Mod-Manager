#include "stdafx.h"
#include "KThemeVisualStudio.h"
#include "UI/KMainWindow.h"
#include "UI/KWorkspace.h"
#include <KxFramework/KxSystemSettings.h>
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxStatusBarEx.h>

KThemeVisualStudio::KThemeVisualStudio()
{
}
KThemeVisualStudio::~KThemeVisualStudio()
{
}

void KThemeVisualStudio::AsWindow(wxWindow* window)
{
	window->SetBackgroundColour(GetColor(KTMC_WINDOW_BG));
	window->SetForegroundColour(GetColor(KTMC_WINDOW_FG));
}

KxColor KThemeVisualStudio::GetColor(KThemeManagerColors index)
{
	switch (index)
	{
		case KTMC_WINDOW_BG:
		case KTMC_SASH_BG:
		{
			return KxColor(41, 58, 86);
		}
		case KTMC_WINDOW_FG:
		{
			return KxColor(0, 0, 0);
		}
		case KTMC_WORKSPACE_BG:
		{
			return KxColor(230, 231, 232);
		}
		case KTMC_TOOLBAR_BG:
		{
			return KxColor(214, 219, 233);
		}
		case KTMC_STATUSBAR_BG:
		{
			return KxColor(0, 122, 204);
		}
		case KTMC_STATUSBAR_ACTIVE_BG:
		{
			return KxColor(202, 81, 0);
		}
		case KTMC_BORDER:
		{
			return KxColor(142, 155, 188);
		}
	};
	return wxNullColour;
}

void KThemeVisualStudio::ProcessWindow(wxWindow* window)
{
	wxWindowUpdateLocker redrawLock(window);
	AsWindow(window);
}
void KThemeVisualStudio::ProcessWindow(KWorkspace* window)
{
	wxWindowUpdateLocker redrawLock(window);

	AsWindow(window);
}
void KThemeVisualStudio::ProcessWindow(KMainWindow* window)
{
	wxWindowUpdateLocker redrawLock(window);

	window->SetBackgroundColour(GetColor(KTMC_WORKSPACE_BG));
	window->SetForegroundColour(GetColor(KTMC_WINDOW_FG));
}
void KThemeVisualStudio::ProcessWindow(KxSplitterWindow* window, bool visibleSash)
{
	wxWindowUpdateLocker redrawLock(window);
	
	AsWindow(window);
	if (!visibleSash)
	{
		window->SetSashColor(window->GetBackgroundColour());
	}
}
void KThemeVisualStudio::ProcessWindow(KxAuiToolBar* window)
{
	wxWindowUpdateLocker redrawLock(window);

	window->SetBorderColor(GetColor(KTMC_BORDER));
	window->SetForegroundColour(GetColor(KTMC_WINDOW_FG));
	window->SetBackgroundColour(GetColor(KTMC_TOOLBAR_BG));
}
void KThemeVisualStudio::ProcessWindow(KxStatusBarEx* window, bool isActive)
{
	wxWindowUpdateLocker redrawLock(window);

	window->SetSeparatorsVisible(false);
	window->SetBorderColor(wxNullColour);
	window->SetBackgroundColour(GetColor(isActive ? KTMC_STATUSBAR_ACTIVE_BG : KTMC_STATUSBAR_BG));
	window->SetForegroundColour(KxColor(255, 255, 255));
}
