# Version history

<div class="right-aligned-text italic-text">
	Dates in DD.MM.YYYY format
</div>

More changes in [commit history](https://github.com/KerberX/Kortex-Mod-Manager/commits/master).

### Version 2.0a11 - 10.03.2020
- [General] Using user-specific settings to format date and time instead of hardcoded fixed format.
- [General] Fixed initial config dialog which allows to choose instances folder. Default folder location was always used unless another path was specified in the settings file.
- [Plugin Manager] Fixed standalone LOOT sorting.
- [Package Manager] Fixed reading condition group block for native package serializer.
- [Package Manager] Fixed reading of tags array from the project file.
- [Package Manager] Fixed mod name and ID pair logic in project config and package creator.
- [Download Manager] Disabled saving of default download meta-info if it didn't contain it on load.

### Version 2.0a10 - 04.03.2020
- [Mod Manager] Fixed `ERROR(76): Attempted to access a path that is not on the disk.` in FNIS. The error was caused by VFS when there was a file copy/move operation but target directory tree doesn't exist in *non*-virtual mod directory (or overwrite).
- [Package Manager] Fixed crash during FOMod loading.
- [Package Manager] Fixed missing nodes in FOMod XML files when their value is empty to conform to the FOMod's XML scheme.
- [Download Manager] Fixed query download info.
- [Download Manager] Fixed changing download source.
- [Download Manager] Added an option to change download target game.
- [Download Manager] Added an option to show only archives in the downloads list.
- [Download Manager] Added a warning before refreshing downloads list if there are any downloads running.

### Version 2.0a9 - 13.02.2020
- [General] Added experimental WebView based on [Sciter](https://sciter.com) to display mod descriptions.
- [General] Fixed disabled mods workspace when it's not the initial workspace.
- [General] Fixed missing buttons on 100% scaling (the fix should have been in 2.0a8 but it's not there apparently).
- [General] Some minor changes.

### Version 2.0a8 - 02.12.2019
- [General] UI component to run programs and VFS added to the main window.
- [General] Currently active workspace (tab or window) is now remembered.
- [General] Fixed UI on HiDPI displays.
- [General] Removed VFS drivers from the main package.
- [General] KxVirtualFileSystem updated to v2.1.1.
- [General] Various small fixes and refactoring.
- [Mod Manager] Added automatic creation of "ModFiles" folder when empty mod folder is created.
- [Mod Manager] Added handling of VFS mount process failure.
- [Mod Manager] Added check for programs started from the VFS on file system termination request.
- [Mod Manager] Added progress indicator for list refresh action.
- [Plugin Manager] LibLoot updated to v15.0.
- [Plugin Manager] Integrated LibLoot now supports Morrowind.
- [Network Manager] Fixed primary NXM handler registering when there is no handler registered yet in the system.
- [Network Manager] Fixed registration of the NXM handler as URL protocol.
- [Network Manager] Disabled annoying mods automatic update check notifications.
- [Package Manager] Fixed wrong required version operator attribute name in package XML file.
- [Package Manager] Fixed some crashes in Package Designer UI.
- [Package Manager] Version 1.3.1.

### Version 2.0a7 - 25.08.2019
- [General] Added network functionality, sign-in, updates, download system for NexusMods.
- [General] Removed refresh button from toolbar.
- [General] Fixed some crashes at startup.
- [General] Drag and drop will cause scroll on attempt to drop to first or last visible row.
- [General] Fixed loading instance when Kortex is unable to locate game directory automatically.
- [General] Profile system reworked, adopted Mod Organizer naming convention (Instance -> Profile instead of Profile -> Mod list).
- [General] Added local saves and local game configuration options for profiles.
- [General] Instances no longer required to be inside folder named after managed game.
- [General] Added game definition for Enderal.
- [General] Fixed plugins limit values in game definitions.
- [General] Added notifications center.
- [General] New UI for About dialog.
- [Mod Manager] Added grouping by priority with collapsible groups.
- [Mod Manager] Added colors to mods and separators.
- [Mod Manager] Added import from Nexus Mod Manager.
- [Mod Manager] Fixed import from MO when data folders (mods, downloads, etc) is not located in specified folder.
- [Mod Manager] Fixed reversed mod order for MO import.
- [Mod Manager] Search by mod name also searches by its ID.
- [Mod Manager] Removed save-undo functions. All changes applied immediately.
- [Mod Manager] Improved performance of statistics calculation.
- [Mod Manager] Added game data window with entire virtual files tree.
- [Mod Manager] Fixed dropped mod position when only one mod is selected in drag and drop operation.
- [Mod Manager] Fixed crash when on sequentially enabling-disabling mod in mod list.
- [Mod Manager] Added column with mod main image.
- [Mod Manager] Various VFS improvements regarding loading speed and file access latency.
- [Mod Manager] Version 1.3.
- [Package Manager] Fixed assigning image as main or as header in "Interface" page.
- [Package Manager] Added back "Import from files" option to components.
- [Package Manager] Fixed search in packages list.
- [Package Manager] Changing mod ID to existing mod's ID in install wizard is now allowed.
- [Package Manager] Fixed requirements check (removed two-phased checking).
- [Package Manager] Component entries in package creator view can now be moved between different branches.
- [Package Manager] Changed conditions system. Fixed undefined condition evaluation order.
- [Package Manager] Version 1.3.
- [Program Manager] Program manager redone. Workspace moved to mods window as a separate tab.
- [Program Manager] Version 2.0.
- [Download Manager] Support for association with NXM links.
- [Download Manager] Support for mod downloads from NexusMods.
- [Download Manager] Support for querying info for unknown downloads.
- [Download Manager] Support for file change log displaying.
- [Download Manager] Added NXM handler to dispatch download link to desired instance or to an external program.
- [Download Manager] Version 1.0.
- [Plugin Manager] For Morrowind, Oblivion, Fallout 3 and Fallout: New Vegas forced sorting by plugin modification date is disabled.
- [Plugin Manager] Added plugins counter for all types.
- [Plugin Manager] Version 1.3.
- [Save Manager] Improved saves list performance.
- [Save Manager] Hiding screenshot column changes rows height.
- [Save Manager] Added reading of ESL block for Fallout 4 and SkyrimSE. Fixed reading ESP list for SkyrimSE.
- [Save Manager] Fixed save screenshot for Morrowind.
- [Save Manager] Version 1.1.
- [Config Manager] Configurator rewritten from scratch.
- [Config Manager] Version 2.0.

### Version 1.3 - 12.07.2018
- [General] The ability to specify a folder for program settings via command-line arguments is added. Syntax: -GlobalConfigPath "Path to folder"
- [General] Changed the interface of several windows.
- [General] Added background color selection in the image viewer.
- [General] Added clearing of log files when the program is terminated. It is allowed to store no more than 10 files.
- [General] Added saving the width of columns and the ability to hide columns in some lists (in the context menu of the header).
- [General] Added support for versions using the date format "YYYY-MM-DD HH: MM: SS" (ISO 8601).
- [General] The active profile folder is no longer available for deletion while the program is running.
- [General] The text view window now supports displaying images from the internet.
- [General] Profile templates for Skyrim VR and Fallout 4 VR were added.
- [Config Manager] The "Mouse Acceleration" setting (Controls::bMouseAcceleration) for supported games was deleted.
- [Config Manager] Added several new settings.
- [Config Manager] Added displaying of all available files regardless of VFS activity.
- [Config Manager] Version 1.1.
- [Package Manager] Added tracking of identifiers for flags. Also, tracking is also performed when items are deleted.
- [Package Manager] Added the function of replacing the contents of a folder in the project without deleting its entry and re-adding it to the project.
- [Package Manager] Removed the toolbars in package creator pages, its functions were moved its list context menu. Lists now supports drag-and-drop.
- [Package Manager] Added check for scripted FOMod install (with the file "Script.cs").
- [Package Manager] Components configuration page was updated.
- [Package Manager] Version 1.2.1.
- [Mod Manager] Added a window to view the mod files and their collisions.
- [Mod Manager] Deleted mods no longer save the status of active after a re-installation.
- [Mod Manager] Added a cross-selection between the list of plugins and the list of mods.
- [Mod Manager] Added the ability to use multiple mod lists in one profile.
- [Mod Manager] Added import from Mod Organizer (1 and 2).
- [Mod Manager] Added search in the mod list.
- [Mod Manager] Added the ability to move the installed mod files to any folder on the computer (thereby making this mod a linked mod).
- [Mod Manager] Each mod list have its own overwrites folder.
- [Mod Manager] Version 1.2.
- [Plugin Manager] Added plugins list import and export.
- [Plugin Manager] A more detailed description of the type for .esl files.
- [Plugin Manager] Added counter for plugins (for each type separately and for all together).
- [Plugin Manager] No more activation of VFS is required to manage plugins.
- [Plugin Manager] Added sorting using the LOOT API v13.
- [Plugin Manager] The workspace is moved to the "Mod manager" window.
- [Plugin Manager] Added search by plugin file name.
- [Plugin Manager] Version 1.2
- [Save Manager] The workspace is moved to the "Mod manager" window.
- [Save Manager] Version 1.0.1
- [Program Manager] The ability to specify the working folder of the program is added.
- [Program Manager] Version 1.0.1.

### Version 1.2 - 02.05.2018
- [Package Manager] Fixed bug due to which conditions for the installation step were not saved.
- [Package Manager] The Install Wizard now does not close after displaying the list of files in the preview mode and goes back to the first page instead.
- [Package Manager] Added the ability to change the target path at once for all folders and files through the appropriate editing menu by right-clicking on the  -column header.
- [Package Manager] Added tracking of item IDs in the project. Now when you change the ID of an element, it changes in the whole project.
- [Package Manager] Added the ability to specify the use of the HTTPS protocol for XML schemas in the FOMod configuration files.
- [Package Manager] Fixed deletion of stages and requirement groups from the project. Now the removal of one stage or requirements group does not cause the - removal of others.
- [Package Manager] Version 1.2.
- [Mod Manager] A little faster start of the game.
- [Mod Manager] Added the ability to create a linked mod.
- [Mod Manager] The calculation of the mod signatures when non-Latin characters is used in identifiers has been changed.
- [Mod Manager] Version 1.1.
- [Plugin Manager] Added support for .esl files for games which uses BethesdaGeneric2 interface (SkyrimSE, Fallout 4).
- [Plugin Manager] Version 1.1.

### Version 1.1 - 21.04.2018
- [General] When creating a profile, the folder with settings and game save is now copied to the profile folder and renamed, and not just moved to the profile - folder.
- [General] Fixed the definition of the folder with the game when creating a new profile.
- [General] English localization is added. Thank you Monday for the translation.
- [Package Manager] Fixed display of images in the interface of creating installers.
- [Package Manager] Importing the project from the installer now specifies this installer as the destination path for the build.
- [Package Manager] Fixed the name of the Required Files item in the project XML file. This means that the list of required installer files will be empty and - such installers must be rebuilt with the reassignment of the required files (if they were used). Sorry.
- [Package Manager] Added an explicit indication of priorities for folders and files when creating the installer.
- [Package Manager] Added context menu items for importing the project and unpacking the archive in the list of available installers.
- [Package Manager] Added the ability to import information to create a project from the installation log.
- [Package Manager] Added the creation of a list of components from the folders added to the project.
- [Package Manager] Added the ability to change the properties of all components at once in the group through the appropriate editing menu by calling it by - right-clicking on the column header.
- [Package Manager] You can now add multiple files at the same time when you add documents to the installer project.
- [Package Manager] Module Version 1.1.
- [Mod Manager] Fixed the impossibility of changing the name of an unset mod in the "Manage Mods" window.
- [Mod Manager] Added a request to unmount VFS when trying to close the program instead of minimizing the window.
- [Mod Manager] Added export of a list of mods in HTML format.
- [Mod Manager] Mods in the download list can be moved by dragging the selected range with the mouse. Both continuous and unbound bands are supported.
- [Mod Manager] Module Version 1.0.1.
- [Plugin Manager] Added transition to the mode when double clicking on the column "Part" in the list of plug-ins.
- [Plugin Manager] Drag and drop is available in the list of plug-ins.
- [Plugin Manager] Module Version 1.0.1.

### Version 1.0 - 09.04.2018
- Initial release
