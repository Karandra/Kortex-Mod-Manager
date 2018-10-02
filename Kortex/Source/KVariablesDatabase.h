#pragma once

#define KVAR(s)			wxS("$(") s wxS(")")
#define KVAR_EXP(s)		V(KVAR(s))

/* App-wide */
// Non-profile
#define KVAR_PROFILES_ROOT wxS("ProfilesRoot")

// Profile-specific
#define KVAR_CURRENT_PROFILE_CONFIG wxS("CurrentProfileConfig")
#define KVAR_PROFILE_ROOT wxS("ProfileRoot")

#define KVAR_MODS_ROOT wxS("ModsRoot")
#define KVAR_PACKAGES_REPOSITORY wxS("PackagesRepository")

#define KVAR_CURRENT_MOD_LIST_ID wxS("ModListID")
#define KVAR_CURRENT_MOD_LIST_SIGNATURE wxS("ModListSignature")
#define KVAR_CURRENT_MOD_LIST_ROOT wxS("ModListRoot")

/* Profile-wide */
#define KVAR_GAME_EXECUTABLE wxS("GameExecutable")
#define KVAR_VIRTUAL_GAME_EXECUTABLE wxS("VirtualGameExecutable")

#define KVAR_GAME_ROOT wxS("GameRoot")
#define KVAR_VIRTUAL_GAME_ROOT wxS("VirtualGameRoot")
#define KVAR_OVERWRITES_ROOT wxS("OverwritesRoot")

#define KVAR_SAVES_ROOT_LOCAL wxS("SavesRootLocal")
#define KVAR_SAVES_ROOT_GLOBAL wxS("SavesRootGlobal")

#define KVAR_CONFIG_ROOT_TARGET wxS("ConfigRootTarget")
#define KVAR_CONFIG_ROOT_GLOBAL wxS("ConfigRootGlobal")
#define KVAR_CONFIG_ROOT_LOCAL wxS("ConfigRootLocal")
