#pragma once
#include "stdafx.h"
#include <KxFramework/KxColor.h>
#include "Utility/EnumClassOperations.h"
class KxSplitterWindow;
class KxStatusBarEx;
class KxAuiToolBar;
class IMainWindow;
class IWorkspace;
class KApp;

namespace Kortex::Theme
{
	enum class ColorIndex: uint32_t
	{
		MainWindow,
		Window,
		SplitterSash,
		ToolBar,
		StatusBar,
		Border,
		Caption
	};
	enum class ColorFlags: uint32_t
	{
		None = 0,

		Background = 0,
		Foreground = 1 << 0,

		Default = 0,
		Active = 1 << 1
	};
}
namespace KxEnumClassOperations
{
	KxImplementEnum(Kortex::Theme::ColorIndex);
	KxImplementEnum(Kortex::Theme::ColorFlags);
}

namespace Kortex
{
	class IThemeManager
	{
		public:
			using ColorIndex = Theme::ColorIndex;
			using ColorFlags = Theme::ColorFlags;

		public:
			static IThemeManager& GetActive();

		protected:
			void InheritColors(wxWindow* window, const wxWindow* from) const;
			bool IsWin8OrGreater() const;

		public:
			virtual ~IThemeManager() = default;

		public:
			virtual KxColor GetColor(ColorIndex index, ColorFlags flags = ColorFlags::None) const = 0;

			virtual void Apply(wxWindow* window) = 0;
			virtual void Apply(IMainWindow* window) = 0;
			virtual void Apply(IWorkspace* window) = 0;
			virtual void Apply(KxSplitterWindow* window) = 0;
			virtual void Apply(KxAuiToolBar* window) = 0;
			virtual void Apply(KxStatusBarEx* window, bool isActive) = 0;
	};
}
