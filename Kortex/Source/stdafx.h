#pragma once

/* KxFramework */
#pragma comment(lib, "KxFramework.lib")

#include <KxFramework/KxFramework.h>
#include <KxFramework/KxApp.h>
#include <KxFramework/KxTranslation.h>
#include <KxFramework/KxSystemSettings.h>
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxXML.h>

#include <KxFramework/KxFrame.h>
#include <KxFramework/KxDialog.h>
#include <KxFramework/KxTaskDialog.h>

/* Kortex */
#define KIPC_SERVER 0
#define KLogMessage	wxLogMessage

#pragma comment(lib, "delayimp")

#include "KTranslation.h"

enum KLayoutConstants
{
	KLC_VERTICAL_SPACING = 3,
	KLC_HORIZONTAL_SPACING = 3,

	KLC_VERTICAL_SPACING_SMALL = 2,
	KLC_HORIZONTAL_SPACING_SMALL = 2,
};
