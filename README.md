# Kortex Mod Manager
Kortex Mod Manager is a mod-manager that uses a virtual file system (VFS) to isolate mods to provide simple resolution of conflicts between mods, clean installation and uninstallation.

Discord server: https://discord.gg/ZyzWjYj

# Supported games
- The Elder Scrolls III: Morrowind
- The Elder Scrolls IV: Oblivion
- The Elder Scrolls V: Skyrim
- [Enderal: Forgotten Stories](https://enderal.com) (TES5: Skyrim total conversion)
- The Elder Scrolls V: Skyrim - Special Edition
- The Elder Scrolls V: Skyrim - VR Edition
- Fallout 3
- Fallout: New Vegas
- Fallout 4
- Fallout 4 - VR Edition
- Sacred 2

# Features
### Configurator
Customize game parameters (graphics and similar). The full-featured configurator is currently only available for Skyrim, SkyrimSE and Fallout 4 (but SkyrimSE and Fallout use a copy of the settings for Skyrim because it is reasonably compatible). The rest of the games will have only with what Kortex can determine automatically (spoiler: only the types of parameters).

### Plugin activation
Like most other mod-managers Kortex can activate plugins. For Oblivion, Fallout 3 and FalloutNV, changing the file modification date is supported so that the load order will remain correct. For Fallout 4 and SkyrimSE, prepending of the asterisks in front of non-official plugins is supported.

### Mod installation
Currently Kortex supports the mods installation only from the special installers. Supported formats: FOMod (for Nexus Mod Manager, XML config only), Kortex Mod Package (KMP) - native format of Kortex. Support for other formats like OMOD or BAIN can be added in the future (when I find specifications for these formats).

### Virtual file system
This is perhaps the most interesting. Unlike [KMM1](https://www.nexusmods.com/skyrim/mods/75738), Kortex does not dump all mod files into the game folder, but dynamically combines them using VFS. This means that the mod's files is not copied into the game folder and nothing there is cluttered and not overwritten. Also VFS allows to simply move the mod in the list to change its priority (the mod is lower in the list -> priority is higher). If you used Mod Organizer, maybe you will be familiar with such a system.

Unfortunately, VFS slows down the game starting. I have more than 200 mods installed and I wait ~2-3 minutes with ENB and 1-1.5 without ENB (you can reduce this time a bit using PrivateProfileRedirector [LE](https://www.nexusmods.com/skyrim/mods/92725), [SE](https://www.nexusmods.com/skyrimspecialedition/mods/18860/), [F4](https://www.nexusmods.com/fallout4/mods/33947)). You may have a different result. Also I advise you to disable the antivirus (or add Kortex to its exclusion list) before starting the game otherwise AV most likely will increase game start time.

### Package creator
Kortex have built-in installer creation tool. This tool can be used to create full featured install packages in KMP and FOMod format.

# Usage info
## Requirements
Windows 7 or later, builin LOOT requires MSVC 2015 redistributable [x86, x64].

## Installation
Unpack from the archive to a folder (not in the folder with the game) and run as an administrator. When updating, follow the removal instructions and then the installation instructions.

## Update
1.x -> 1.3: Plugins order and activation state will be reset, mod order and state should remain untouched.

## Uninstallation
Close the program, delete the VFS service by typing the command sc delete KortexVFS in Windows console (cmd.exe, run it as admin), restart the computer and delete the program files.

# Download location
- Nexus: https://www.nexusmods.com/skyrim/mods/90868
- TESALL.RU: http://tesall.ru/files/file/9540-kortex-mod-manager

# Building
### Dependencies
- [KxFramework](https://github.com/KerberX/KxFramework)
- [KxVirtualFileSystem](https://github.com/KerberX/KxVirtualFileSystem)
- [7zip-cpp](https://github.com/KerberX/7zip-cpp)
- [LOOT API](https://github.com/loot/loot-api) (prebuild binaries included)

### MSVC2017+
- Go to project *Properties* -> *C/C++* -> *General* -> *Additional include directories* and chnage include paths to required libraries.
- Same with .lib files (*Librarian* -> *General* -> *Additional library directories*).
- Build **Release** configuration for x86 and x64.
