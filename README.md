# [Kortex Mod Manager](https://kerberx.github.io/Kortex-Mod-Manager/?page=home)
Kortex Mod Manager is a mod-manager that uses a [virtual file system (VFS)](https://github.com/KerberX/KxVirtualFileSystem) to isolate mods to provide simple resolution of conflicts between mods, clean installation and uninstallation.

Visit Kortex [website](https://kerberx.github.io/Kortex-Mod-Manager) if you want to read more about it. If you have any questions, suggestions or anything else feel free to join our [discord](https://discord.gg/ZyzWjYj) server.

# Supported games
- The Elder Scrolls III: Morrowind
- The Elder Scrolls IV: Oblivion
- The Elder Scrolls V: Skyrim (including [Enderal: Forgotten Stories](https://enderal.com))
- The Elder Scrolls V: Skyrim - Special Edition (+VR)
- Fallout 3
- Fallout: New Vegas
- Fallout 4 (+VR)
- Sacred 2 (experiment with a non-Bethesda game if you wanted to ask why exactly this game)

# [Features](https://kerberx.github.io/Kortex-Mod-Manager/?page=features)

### Virtual file system (VFS)
The virtual file system is the core component of Kortex. It allows to quickly change installed mods without having to copy quite a few gigabytes of data around. With the help of the VFS it's also possible to keep the game folder clean as if you have no mods installed at all.

Advantages of the VFS:
- Quick mod activation since no files are moved.
- No original game files lost because original game's folder is untouched.
- No lost files due to new mod overwritten an old one - all files are still in their folders.
- Easier file collision detection (not yet implemented).
- Safe mod installation and uninstallation.

There are also disadvantages, mostly the game's startup speed. Kortex uses its own VFS solution based on [Dokany](https://github.com/dokan-dev/dokany) [2 Beta](https://github.com/dokan-dev/dokany/releases/tag/v2.0.0-BETA1) - **KxVFS**. Dokany is a kernel mode file system driver and that means every IO call (read/write operation) in mounted directory will be served by the driver which requires constant user-to-kernel-to-user jumps. User mode solution like the one used in Mod Organizer or something completely different (symlinks, hardlinks) will be faster. I spent a lot of time optimizing KxVFS. It went from 10 minutes startup time for Skyrim to 1 minute in v2.0a7, I hope you'll appreciate that.

### Mod management
Kortex provides pretty much standard set of features for a generic mod manager.
- Drag and drop load order.
- Instances and profiles support.
- Linking files from any folder on the system as a mod.
- Collapsible separators for mod list.
- FOMod support (XML only).
- KMP support (Kortex Mod Package - native mod package format).

### Plugin management
Kortex provides plugin management tools for all games it supports. At the moment the plugin management that is available for all Bethesda games includes:
- Drag and drop load order.
- Calculating correct load order index (including games with ESL support).
- Sorting using LOOT (both integrated and as an external program) and for older games BOSS.
- Import and export of plugins list.
- Active plugins counter.
- Plugin dependencies scan.

### Package creator
Kortex have built-in installer creation tool. This tool can be used to create full featured install packages in KMP and FOMod format.

### Other features
- Virtual folder explorer
- Saves manager
- Program list
- Download manager
- More features are covered in the [comparison table](https://kerberx.github.io/Kortex-Mod-Manager/?page=comparison).

# Change log
Look here: https://kerberx.github.io/Kortex-Mod-Manager/?page=version-history

# Usage info
## [Requirements](https://kerberx.github.io/Kortex-Mod-Manager/?page=requirements)
- Windows 7 and newer, **x64 system is strongly recommended**.
- MSVC 2015-2019 redistributable packages, both for x86 and x86.
- At least 2GB of RAM.

## Installation
Unpack from the archive to any directory (not in the directory with the game). After first installation (or after updating) run Kortex as an administrator. When updating, follow the removal instructions and then the installation instructions.

## Uninstallation
Close the program, delete the VFS service by typing the command `sc delete KortexVFS` in Windows console (cmd.exe, run it as administrator), restart the computer and delete the program's files.

## Update
1.x -> 1.3: Plugins order and activation state will be reset, mod order and state should remain untouched.
1.3 -> 2.x: Should probably work without significant issues.

# Download location
- Nexus: https://www.nexusmods.com/skyrim/mods/90868
- TESALL.RU: http://tesall.ru/files/file/9540-kortex-mod-manager

# Building
**This paragraph is under construction**

### Dependencies
- [KxFramework](https://github.com/KerberX/KxFramework)
- [KxVirtualFileSystem](https://github.com/KerberX/KxVirtualFileSystem)
- [LOOT API](https://github.com/loot/loot-api) (prebuild binaries included)

### MSVC2019+
- Install **KxFramework** (using [VCPkg](https://github.com/microsoft/vcpkg)) following its building instructions.
- Install additional packages with VCPkg: `kxvfs`, ~~`libloot`~~ (not available yet, manual installation is required for now).
- Build **Release** configuration for x86 and x64.
