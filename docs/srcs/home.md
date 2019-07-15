# Kortex Mod Manager

Kortex Mod Manager is a mod-manager that uses a virtual file system (VFS) to isolate mods and provide simple resolution of conflicts between mods, clean installation and uninstallation.

## Configurator

Customize game parameters (graphics and similar). The full-featured configurator is currently only available for Skyrim, SkyrimSE and Fallout 4 (but SkyrimSE and Fallout use a copy of the settings for Skyrim because it is reasonably compatible). The rest of the games will have only with what Kortex can determine automatically (spoiler: only the types of parameters).

## Plugin activation

Like most other mod-managers Kortex can activate plugins. For Oblivion, Fallout 3 and FalloutNV, changing the file modification date is supported so that the load order will remain correct. For Fallout 4 and SkyrimSE, prepending of the asterisks in front of non-official plugins is supported.

## Mod installation

Currently Kortex supports the mods installation only from the special installers. Supported formats: FOMod (for Nexus Mod Manager, XML config only), Kortex Mod Package (KMP) - native format of Kortex. Support for other formats like OMOD or BAIN can be added in the future (when I find specifications for these formats).

## Virtual file system

This is perhaps the most interesting. Unlike KMM1, Kortex does not dump all mod files into the game folder, but dynamically combines them using VFS. This means that the mod's files is not copied into the game folder and nothing there is cluttered and not overwritten. Also VFS allows to simply move the mod in the list to change its priority (the mod is lower in the list -> priority is higher). If you used Mod Organizer, maybe you will be familiar with such a system.  

Unfortunately, VFS slows down the game starting. For about 200 mods installed and I have to wait ~1-2 minutes with ENB and 0.5-1 without ENB (you can reduce this time a bit using PrivateProfileRedirector [LE](https://www.nexusmods.com/skyrim/mods/92725), [SE](https://www.nexusmods.com/skyrimspecialedition/mods/18860), [F4](https://www.nexusmods.com/fallout4/mods/33947)). You may have a different result. Also I advise you to disable the antivirus (or add Kortex to its exclusion list) before starting the game otherwise AV most likely will increase game start time.

## Package creator

Kortex have built-in installer creation tool. This tool can be used to create full featured install packages in KMP and FOMod format.
