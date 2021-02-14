#pragma once

// Required because of wxWidgets
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// Kortex
#if defined KORTEX_LIBRARY
#define KORTEX_API __declspec(dllexport)
#elif defined KORTEX_MODULE
#define KORTEX_API __declspec(dllimport)
#else
#define KORTEX_API
#endif

// kxf
#include <kxf/Common.hpp>

// kxf: General
#include <kxf/General/Common.h>
#include <kxf/General/String.h>
#include <kxf/General/StringFormatter.h>
#include <kxf/General/DateTime.h>
#include <kxf/General/BinarySize.h>
#include <kxf/General/ResourceID.h>
#include <kxf/General/FlagSet.h>
#include <kxf/General/Version.h>
#include <kxf/General/Enumerator.h>
#include <kxf/General/UniversallyUniqueID.h>
#include <kxf/General/IVariablesCollection.h>

// kxf: RTTI
#include <kxf/RTTI/Common.h>
#include <kxf/RTTI/RTTI.h>

// kxf: EventSystem
#include <kxf/EventSystem/Common.h>
#include <kxf/EventSystem/IEvtHandler.h>

// kxf: FileSystem
#include <kxf/FileSystem/Common.h>
#include <kxf/FileSystem/FSPath.h>
#include <kxf/FileSystem/FileItem.h>
#include <kxf/FileSystem/IFileSystem.h>

// kxf: System
#include <kxf/System/Common.h>
#include <kxf/System/SystemInformation.h>
#include <kxf/System/SystemAppearance.h>

// kxf: Drawing
#include <kxf/Drawing/Common.h>
#include <kxf/Drawing/Color.h>
#include <kxf/Drawing/Angle.h>
#include <kxf/Drawing/BitmapImage.h>

// kxf: Utility
#include <kxf/Utility/Common.h>
#include <kxf/Utility/Literals.h>

namespace Kortex
{
	// Import all literal operators
	using namespace kxf::Literals;
}
