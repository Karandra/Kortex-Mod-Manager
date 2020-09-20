# Features

## Virtual file system (VFS)
The virtual file system is the core component of Kortex. It allows to quickly change installed mods without having to copy quite a few gigabytes of data around. With the help of the VFS it's also possible to keep the game folder clean as if you have no mods installed at all.

Advantages of the VFS:
- Quick mod activation since no files are moved.
- No original game files lost because original game's folder is untouched.
- No lost files due to new mod overwritten an old one - all files are still in their folders.
- Easier file collision detection (not yet implemented).
- Safe mod installation and uninstallation.

There are also disadvantages, mostly the game's startup speed. Kortex uses its own VFS solution based on [Dokany](https://github.com/dokan-dev/dokany) [2 Beta](https://github.com/dokan-dev/dokany/releases/tag/v2.0.0-BETA1) - <span tooltip="KxVirtualFileSystem, Kx stands for Kortex">**KxVFS**</span>. Dokany is a kernel mode file system driver and that means every IO call (read/write operation) in mounted directory will be served by the driver which requires constant user-to-kernel-to-user jumps. User mode solution like the one used in Mod Organizer or something completely different (symlinks, hardlinks) will be faster. I spent a lot of time optimizing KxVFS. It went from 10 minutes startup time for Skyrim to 1 minute in v2.0a7, I hope you'll appreciate that.

If you've came here by an accident looking for a VFS solution then the **KxVFS** is a general purpose file system virtualization library written in C++17. Check out its [GitHub](https://github.com/KerberX/KxVirtualFileSystem).

## Mod management
Kortex provides pretty much standard set of features for a generic mod manager.
- Drag and drop load order.
- Instances and profiles support.
- Linking files from any folder on the system as a mod.
- Collapsible separators for mod list.
- FOMod support (XML only).
- KMP support (Kortex Mod Package - native mod package format).

## Plugin management
Kortex provides plugin management tools for all games it supports. At the moment the plugin management that is available for all Bethesda games includes:
- Drag and drop load order.
- Calculating correct load order index (including games with ESL support).
- Sorting using LOOT (both integrated and as an external program) and for older games BOSS.
- Import and export of plugins list.
- Active plugins counter.
- Plugin dependencies scan.

## Package creator
Kortex have built-in installer creation tool. This tool can be used to create full featured install packages in KMP and FOMod format.

## Configurator
Configurator allows you to customize game's parameters (graphics, gameplay, etc). The full-featured configurator is currently only available for Skyrim, SkyrimSE and Fallout 4 (but SkyrimSE and Fallout 4 use a copy of the settings for Skyrim because they are reasonably compatible). For games without predefined configuration information the configurator will try to load all available options.

## Other features
- Virtual folder explorer
- Saves manager
- Program list
- Download manager

More features are covered in the [comparison table](?page=comparison).
