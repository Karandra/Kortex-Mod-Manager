#pragma once
#include "stdafx.h"
#include <KxFramework/KxColor.h>
class KxSplitterWindow;
class KxStatusBarEx;
class KxAuiToolBar;
class IMainWindow;
class IWorkspace;
class KApp;


namespace Kortex
{
	class IThemeManager
	{
		public:
			enum class ColorIndex
			{
				WindowBG,
				WindowFG,
				SplitterSash,
				WorkspaceBG,
				ToolBarBG,
				StatusBarBG,
				StatusBarActiveBG,
				Border,
			};

		public:
			static IThemeManager& GetActive();

		private:
			bool m_Win8OrGreater = false;

		protected:
			void InheritColors(wxWindow* window, const wxWindow* from);
			bool IsWin8OrGreater() const
			{
				return m_Win8OrGreater;
			}

		public:
			IThemeManager();
			virtual ~IThemeManager() = default;

		public:
			virtual KxColor GetColor(ColorIndex index) const = 0;

			virtual void ProcessWindow(wxWindow* window) = 0;
			virtual void ProcessWindow(IMainWindow* window) = 0;
			virtual void ProcessWindow(IWorkspace* window) = 0;
			virtual void ProcessWindow(KxSplitterWindow* window, bool visibleSash = false) = 0;
			virtual void ProcessWindow(KxAuiToolBar* window) = 0;
			virtual void ProcessWindow(KxStatusBarEx* window, bool isActive) = 0;
	};
}
