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
			KxColor GetColor(ColorIndex index, ColorFlags flags = ColorFlags::None) const override;

			void Apply(wxWindow* window) override;
			void Apply(IWorkspace* window) override;
			void Apply(IMainWindow* window) override;
			void Apply(KxSplitterWindow* window) override;
			void Apply(KxAuiToolBar* window) override;
			void Apply(KxStatusBarEx* window, bool isActive) override;
	};
}
