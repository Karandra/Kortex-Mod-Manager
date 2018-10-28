#pragma once

#define KVAR(s)			wxS("$(") s wxS(")")
#define KVAR_EXP(s)		KVarExp(KVAR(s))

/* App-wide */
// These variables are set by Kortex itself
#define KVAR_INSTANCES_DIR wxS("InstancesDir")
#define KVAR_INSTANCE_ID wxS("InstanceID")
#define KVAR_PROFILE_ID wxS("ProfileID")

/* Instance */
// These are required variables for instance template
// Except for 'KVAR_SCRIPT_EXTENDER_ID' which is optional
#define KVAR_GAME_ID wxS("GameID")
#define KVAR_GAME_NAME wxS("GameName")
#define KVAR_GAME_SHORT_NAME wxS("GameShortName")
#define KVAR_GAME_SORT_ORDER wxS("GameSortOrder")
#define KVAR_ACTUAL_GAME_DIR wxS("ActualGameDir")
#define KVAR_GAME_EXECUTABLE wxS("GameExecutable")
#define KVAR_SCRIPT_EXTENDER_ID wxS("ScriptExtenderID")

// Each instance will have these variables set to appropriate value
#define KVAR_VIRTUAL_GAME_DIR wxS("VirtualGameDir")
#define KVAR_INSTANCE_DIR wxS("InstanceDir")
#define KVAR_MODS_DIR wxS("ModsDir")
#define KVAR_PROFILES_DIR wxS("ProfilesDir")

/* Profile */
// Variables which names starts from 'Actual' and 'Global' are constant
// during instance lifetime, anything else is changed on profile switch
#define KVAR_PROFILE_ID wxS("ProfileID")
#define KVAR_PROFILE_DIR wxS("ProfileDir")

#define KVAR_OVERWRITES_DIR wxS("OverwritesDir")

#define KVAR_SAVES_DIR wxS("SavesDir")
#define KVAR_ACTUAL_SAVES_DIR wxS("ActualSavesDir")
#define KVAR_GLOBAL_SAVES_DIR wxS("GlobalSavesDir")

#define KVAR_CONFIG_DIR wxS("ConfigDir")
#define KVAR_ACTUAL_CONFIG_DIR wxS("ActualConfigDir")
#define KVAR_GLOBAL_CONFIG_DIR wxS("GlobalConfigDir")
