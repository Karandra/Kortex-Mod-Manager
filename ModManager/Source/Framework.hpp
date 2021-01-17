#pragma once

// Required because of wxWidgets
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// KxFramework
#include <kxf/Common.hpp>

#include <kxf/General/Common.h>
#include <kxf/General/String.h>
#include <kxf/General/StringFormatter.h>
#include <kxf/General/DateTime.h>
#include <kxf/General/BinarySize.h>
#include <kxf/General/ResourceID.h>
#include <kxf/General/FlagSet.h>
#include <kxf/General/Version.h>

#include <kxf/EventSystem/Common.h>
#include <kxf/EventSystem/IEvtHandler.h>

#include <kxf/RTTI/Common.h>
#include <kxf/RTTI/QueryInterface.h>

#include <kxf/FileSystem/Common.h>
#include <kxf/FileSystem/FileItem.h>
#include <kxf/FileSystem/FSPath.h>
#include <kxf/FileSystem/IFileSystem.h>

#include <kxf/Drawing/Color.h>
#include <kxf/Drawing/Angle.h>
#include <kxf/Drawing/BitmapImage.h>

namespace Kortex::LayoutConstant
{
	constexpr int HorizontalSpacing = 3;
	constexpr int VerticalSpacing = 3;

	constexpr int HorizontalSpacingSmall = 2;
	constexpr int VerticalSpacingSmall = 2;
}
