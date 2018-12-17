#pragma once
#include "stdafx.h"

namespace Kortex::Variables
{
	template<class T> wxString WrapAsInline(T&& variable, const wxString& variableNamespace = wxEmptyString)
	{
		return wxS('$') + variableNamespace + wxS('(') + variable + wxS(')');
	}
}

namespace Kortex::Variables::NS
{
	const constexpr wxChar Global[] = wxS("");
	const constexpr wxChar Translation[] = wxS("T");
	const constexpr wxChar ShellFolder[] = wxS("SHF");
	const constexpr wxChar Environment[] = wxS("ENV");
}

// App-wide
namespace Kortex::Variables
{
	// These variables are set by Kortex itself.
	const constexpr wxChar KVAR_APP_SETTINGS_DIR[] = wxS("AppSettingsDir");
	const constexpr wxChar KVAR_INSTANCES_DIR[] = wxS("InstancesDir");
	
	const constexpr wxChar KVAR_GAME_ID[] = wxS("GameID");
	const constexpr wxChar KVAR_INSTANCE_ID[] = wxS("InstanceID");
}

// Instance
namespace Kortex::Variables
{
	// These are required variables for an instance template,
	// except for 'KVAR_SCRIPT_EXTENDER_ID' which is optional.
	const constexpr wxChar KVAR_GAME_NAME[] = wxS("GameName");
	const constexpr wxChar KVAR_GAME_SHORT_NAME[] = wxS("GameShortName");
	const constexpr wxChar KVAR_GAME_SORT_ORDER[] = wxS("GameSortOrder");
	const constexpr wxChar KVAR_ACTUAL_GAME_DIR[] = wxS("ActualGameDir");
	const constexpr wxChar KVAR_GAME_EXECUTABLE[] = wxS("GameExecutable");
	const constexpr wxChar KVAR_SCRIPT_EXTENDER_ID[] = wxS("ScriptExtenderID");

	// Each instance will have these variables set to appropriate value.
	const constexpr wxChar KVAR_VIRTUAL_GAME_DIR[] = wxS("VirtualGameDir");
	const constexpr wxChar KVAR_INSTANCE_DIR[] = wxS("InstanceDir");
	const constexpr wxChar KVAR_MODS_DIR[] = wxS("ModsDir");
	const constexpr wxChar KVAR_PROFILES_DIR[] = wxS("ProfilesDir");
}

// Profile
namespace Kortex::Variables
{
	// Variables which names starts from 'Actual' and 'Global' are constant
	// during instance lifetime, anything else is changed on profile switch.
	const constexpr wxChar KVAR_PROFILE_ID[] = wxS("ProfileID");
	const constexpr wxChar KVAR_PROFILE_DIR[] = wxS("ProfileDir");

	const constexpr wxChar KVAR_OVERWRITES_DIR[] = wxS("OverwritesDir");

	const constexpr wxChar KVAR_SAVES_DIR[] = wxS("SavesDir");
	const constexpr wxChar KVAR_ACTUAL_SAVES_DIR[] = wxS("ActualSavesDir");
	const constexpr wxChar KVAR_GLOBAL_SAVES_DIR[] = wxS("GlobalSavesDir");

	const constexpr wxChar KVAR_CONFIG_DIR[] = wxS("ConfigDir");
	const constexpr wxChar KVAR_ACTUAL_CONFIG_DIR[] = wxS("ActualConfigDir");
	const constexpr wxChar KVAR_GLOBAL_CONFIG_DIR[] = wxS("GlobalConfigDir");

}

//#define KVAR(s)			wxS("$(") s wxS(")")
//#define KVAR_EXP(s)		KVarExp(KVAR(s))
