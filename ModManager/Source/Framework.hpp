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
#include <kxf/General/Color.h>

#include <kxf/EventSystem/Common.h>
#include <kxf/EventSystem/IEvtHandler.h>

#include <kxf/RTTI/Common.h>
#include <kxf/RTTI/QueryInterface.h>

#include <kxf/FileSystem/Common.h>
#include <kxf/FileSystem/FileItem.h>
#include <kxf/FileSystem/FSPath.h>
#include <kxf/FileSystem/IFileSystem.h>
