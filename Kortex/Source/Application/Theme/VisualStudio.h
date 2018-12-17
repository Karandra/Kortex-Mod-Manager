#pragma once
#include "stdafx.h"
#include "Application/IThemeManager.h"

namespace Kortex::Theme
{
	class VisualStudio: public IThemeManager
	{
		private:
			void AsWindow(wxWindow* window);

		public:
			KxColor GetColor(ColorIndex index) const override;

			void ProcessWindow(wxWindow* window) override;
			void ProcessWindow(KWorkspace* window) override;
			void ProcessWindow(KMainWindow* window) override;
			void ProcessWindow(KxSplitterWindow* window, bool visibleSash = false) override;
			void ProcessWindow(KxAuiToolBar* window) override;
			void ProcessWindow(KxStatusBarEx* window, bool isActive) override;
	};
}
