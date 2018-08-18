#pragma once
#include "stdafx.h"
#include "KThemeManager.h"

class KThemeVisualStudio: public KThemeManager
{
	public:
		KThemeVisualStudio();
		virtual ~KThemeVisualStudio();

	private:
		void AsWindow(wxWindow* window);

	public:
		virtual KxColor GetColor(KThemeManagerColors index) override;

		virtual void ProcessWindow(wxWindow* window) override;
		virtual void ProcessWindow(KWorkspace* window) override;
		virtual void ProcessWindow(KMainWindow* window) override;
		virtual void ProcessWindow(KxSplitterWindow* window, bool visibleSash = false) override;
		virtual void ProcessWindow(KxAuiToolBar* window) override;
		virtual void ProcessWindow(KxStatusBarEx* window, bool isActive) override;
};
