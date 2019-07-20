#pragma once
#include "stdafx.h"
#include "Core.hpp"

#include "Application/IApplication.h"
#include "Application/DefaultApplication.h"
#include "Application/BroadcastProcessor.h"
#include "Application/Events/LogEvent.h"

#include "Application/IAppOption.h"
#include "Application/Options/Option.h"
#include "Application/Options/OptionSerializer.h"
#include "Application/Options/OptionDatabase.h"

#include "Application/INotification.h"
#include "Application/INotificationCenter.h"
#include "Application/IThemeManager.h"

#include "Application/IVariableTable.h"
#include "Application/VariablesTable/VariablesDatabase.h"
#include "Application/VariablesTable/StaticVariableTable.h"
#include "Application/VariablesTable/DynamicVariableTable.h"
