#pragma once

#define KVAR(s)			"$(" ## s ## ")"
#define KVAR_EXP(s)		V(KVAR(s))

/* App-wide */
// Non-profile
#define KVAR_PROFILES_ROOT "ProfilesRoot"

// Profile-specific
#define KVAR_CURRENT_PROFILE_CONFIG "CurrentProfileConfig"
#define KVAR_CURRENT_PROFILE_ROOT "CurrentProfileRoot"
#define KVAR_MODS_ROOT "ModsRoot"
#define KVAR_PACKAGES_REPOSITORY "PackagesRepository"

/* Profile-wide */
#define KVAR_GAME_EXECUTABLE "GameExecutable"
#define KVAR_VIRTUAL_GAME_EXECUTABLE "VirtualGameExecutable"

#define KVAR_GAME_ROOT "GameRoot"
#define KVAR_VIRTUAL_GAME_ROOT "VirtualGameRoot"
#define KVAR_WRITE_TARGET_ROOT "WriteTargetRoot"

#define KVAR_CONFIG_ROOT "ConfigRoot"
#define KVAR_VIRTUAL_CONFIG_ROOT "VirtualConfigRoot"
