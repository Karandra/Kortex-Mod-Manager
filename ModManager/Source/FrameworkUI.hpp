#pragma once
#include "Framework.hpp"

// Drawing
#include <kxf/Drawing/Common.h>
#include <kxf/Drawing/SizeRatio.h>

// UI
#include <kxf/UI/Common.h>

#include <kxf/UI/Windows/Panel.h>
#include <kxf/UI/Windows/Frame.h>
#include <kxf/UI/Windows/TopLevelWindow.h>

#include <kxf/UI/Dialogs/Dialog.h>
#include <kxf/UI/Dialogs/StdDialog.h>
#include <kxf/UI/Dialogs/TaskDialog.h>
#include <kxf/UI/Dialogs/FileBrowseDialog.h>

#include <kxf/UI/Controls/Button.h>
#include <kxf/UI/Controls/DataView.h>

namespace Kortex
{
	namespace DataView = kxf::UI::DataView;
}

namespace Kortex::LayoutConstant
{
	constexpr int HorizontalSpacing = 3;
	constexpr int VerticalSpacing = 3;

	constexpr int HorizontalSpacingSmall = 2;
	constexpr int VerticalSpacingSmall = 2;
}
