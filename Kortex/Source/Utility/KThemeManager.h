#pragma once
#include "stdafx.h"
#include <KxFramework/KxColor.h>
class KxSplitterWindow;
class KxStatusBarEx;
class KxAuiToolBar;
class KMainWindow;
class KWorkspace;
class KApp;

enum KThemeManagerColors
{
	KTMC_WINDOW_BG,
	KTMC_WINDOW_FG,
	KTMC_SASH_BG,
	KTMC_WORKSPACE_BG,
	KTMC_TOOLBAR_BG,
	KTMC_STATUSBAR_BG,
	KTMC_STATUSBAR_ACTIVE_BG,
	KTMC_BORDER,
};

//////////////////////////////////////////////////////////////////////////
class KThemeManager
{
	friend class KApp;

	private:
		static KThemeManager* ms_Instance;

	public:
		static KThemeManager& Get();
	protected:
		static void Set(KThemeManager* theme);
	private:
		static void Cleanup();

	private:
		bool m_Win8OrGreater = false;

	protected:
		void InheritColors(wxWindow* window, const wxWindow* from);

	public:
		KThemeManager();
		virtual ~KThemeManager();

	public:
		virtual bool IsWindows8OrGreater() const
		{
			return m_Win8OrGreater;
		}

		virtual KxColor GetColor(KThemeManagerColors index) = 0;

		virtual void ProcessWindow(wxWindow* window) = 0;
		virtual void ProcessWindow(KMainWindow* window) = 0;
		virtual void ProcessWindow(KWorkspace* window) = 0;
		virtual void ProcessWindow(KxSplitterWindow* window, bool visibleSash = false) = 0;
		virtual void ProcessWindow(KxAuiToolBar* window) = 0;
		virtual void ProcessWindow(KxStatusBarEx* window, bool isActive) = 0;
};
