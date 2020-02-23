# Comparison to other mod managers

There are many mod managers around these days and it's may be a bit difficult to choose the one you need. This page is designed to highlight key features of known mod managers.

## Legend

* **KMM** - Kortex Mod Manager.
* **MO1** - [Mod Organizer 1](https://www.nexusmods.com/skyrim/mods/1334) (version 1.x).
* **MO2** - [Mod Organizer 2](https://www.nexusmods.com/skyrimspecialedition/mods/6194) (version 2.x).
* **NMM** - [Nexus Mod Manager](https://www.nexusmods.com/site/mods/4).
* [**Vortex**](https://www.nexusmods.com/about/vortex) - a successor to **NMM** by Nexus Mods.
* [**Wrye Bash**](https://github.com/wrye-bash/wrye-bash) - one of the oldest mod managers among them all.

## Features

<table>
    <tbody>
        <tr align="center">
            <th />
            <th>Kortex</th>
            <th>MO2</th>
            <th>MO1</th>
            <th>Vortex</th>
            <th>NMM</th>
            <th>Wrye Bash</th>
        </tr>
        <tr>
            <td class="feature-name">Supported games</td>
            <td id="KMM">
                <span tooltip="Morrowind, Oblivion, Skyrim (LE, SE, VR, Enderal)">TES series</span> starting from <b>Morrowind</b>.
                <br/>
                <span tooltip="Fallout 3, Fallout: New Vegas, Fallout 4 (VR)">Fallout series</span> starting from <b>Fallout 3</b>.
                <br/>
                <span tooltip="Non Bethesda game as an experiment"><b>Sacred 2</b></span>.
            </td>
            <td id="MO2">
                <b>Morrowind,</b>
                <b>Oblivion,</b>
                <b>Fallout 3,</b>
                <b>Fallout NV,</b>
                <b>Tale of Two Wastelands (TTW),</b>
                <b>Skyrim,</b>
                <b>Skyrim SE,</b>
                <b>Skyrim VR,</b>
                <b>Fallout 4,</b>
                <b>Fallout 4 VR,</b>
                <b>Enderal (Steam Edition)</b>
            </td>
            <td id="MO1">
                <b>Oblivion, Skyrim, Fallout 3, Fallout: New Vegas</b>.
            </td>
            <td id="Vortex">
                Supports over 90 games including TES and Fallout games.
            </td>
            <td id="NMM">
                Supports many games including TES and Fallout games.
            </td>
            <td id="WB">
                Morrowind using <a href="https://github.com/Wrye-Code-Collection/Wrye-Mash/releases">Wrye Mash</a>.
                <br/>
                <a href="https://www.nexusmods.com/fallout3/mods/11336">Fallout 3</a>/<a href="https://www.nexusmods.com/newvegas/mods/35003">Fallout NV</a> Using Valda's Wrye Flash.
                <br/>
                Oblivion, Skyrim (LE, SE, VR, Enderal) Fallout 4 (VR) Using <a href="https://www.dropbox.com/sh/iazpayeexiyazeh/AAAbGeVHrlIksp2AFgI4w48Oa?dl=0">Wrye Bash</a>.
            </td>
        </tr>
        <tr>
            <td class="feature-name">Importing from other managers</td>
            <td id="KMM">
                Nexus Mod Manager, MO1, MO2.
            <td id="MO2">
                <div class="cmp-no" />
            </td>
            <td id="MO1">
                <div class="cmp-no" />
            </td>
            <td id="Vortex">
                MO1, MO2, NMM.
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                <div class="cmp-no" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Profile management</td>
            <td id="KMM.MO2" colspan="2">
                Multiple instances per game, multiple profiles per instance.
            </td>
            <td id="MO1">
                One instance for one MO installation, multiple profiles per instance.
            </td>
            <td id="Vortex.NMM" colspan="2">
                Multiple profiles per game
            </td>
            <td id="WB">
                <div class="cmp-no" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Assets conflict management</td>
            <td id="KMM">
                <div class="cmp-yes" />
            </td>
            <td id="MO2">
                <div class="cmp-yes" />
            </td>
            <td id="MO1">
                <div class="cmp-yes" />
            </td>
            <td id="Vortex">
                <div class="cmp-yes" />
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                <div class="cmp-yes" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Conflict resolution</td>
            <td id="KMM">
                Using mod <b>priority</b> that can be changed by <b>drag & dropping</b> mods in a specific order<br>
            </td>
            <td id="MO2">
                Using mod <b>priority</b> that can be changed by <b>drag & dropping</b> mods in a specific order.<br>
                Per file resolution through <b>hiding</b> of files (renaming with .mohidden).
            </td>
            <td id="MO1">
                Using mod <b>priority</b> that can be changed by <b>drag & dropping</b> mods in a specific order.<br>
                Per file resolution through <b>hiding</b> of files (renaming with .mohidden).
            </td>
            <td id="Vortex">
                Through definition of order <b>rules</b> between mods, which can be created through drag & drop of the relationship icon.
                Per file resolution through advanced conflict view (this can be different per-profile)
            </td>
            <td id="NMM">
                One time popup during installation to decide whether to keep or overwrite other mods files.
            </td>
            <td id="WB">
                Using mod priority that can be changed by drag &amp; dropping mods in a specific order.
            </td>
        </tr>
        <tr>
            <td class="feature-name">Conflict visualization</td>
            <td id="KMM">
                <div class="cmp-no" />
            </td>
            <td id="MO2">
                <ul>
                    Same as MO1 plus:
                    <li>Advanced per-mod conflict views with alternatives, sorting, search, etc.</li>
                </ul>
            </td>
            <td id="MO1">
                <ul>
                    <li>Different conflict flags on modlist for different conflict types (winning/losing/both).</li>
                    <li>Highlighting of mods that have conflict with the currently selected mod.</li>
                    <li>Listing of losing, winning and non-conflicted files for each mod.</li>
                    <li>Ability to preview and cycle alternatives of textures and other file types.</li>
                </ul>
            </td>
            <td id="Vortex">
                <ul>
                    <li>Single generic flag on modlist for all conflicts types.</li>
                    <li>Only an "active-rule" icon once conflict has been resolved through a rule.</li>
                    <li>Popup when installing mod with conflicts.</li>
                    <li>Simple conflict resolution window (rules) with tooltip of conflicted files.</li>
                    <li>Advanced per-mod file tree structure of conflicts with alternatives from other mods.</li>
                </ul>
            </td>
            <td id="NMM">
                No visualization of conflicts as there can't be, just one time popup during mod installation to either overwrite or keep old files.
            </td>
            <td id="WB">
                <ul>
                    <li>Colored checkboxes indicate conflict.</li>
                    <li>Tabs can be selected to see detailed list of conflicts for both loose files and BSA/BA2 files.</li>
                    <li>The mod archive or package name is shown.</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td class="feature-name">Game archives <span tooltip="Bethesda Softworks Archive" flow="right">(BSA)</span> management</td>
            <td id="KMM">
                <div class="cmp-no" />
            </td>
            <td id="MO2">
                <ul>
                    <li>Extraction</li>
                    <li>Creation</li>
                    <li>Content preview</li>
                    <li>Experimental conflict detection</li>
                </ul>
            </td>
            <td id="MO1">
                <ul>
                    <li>Extraction</li>
                    <li>Conflict detection</li>
                    <li>Loading without plugins</li>
                </ul>
            </td>
            <td id="Vortex">
                <div class="cmp-no" />
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                <div class="cmp-no" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Game archives (BSA) Conflicts</td>
            <td id="KMM">
                <div class="cmp-no" />
            </td>
            <td id="MO2">
                Only experimental <span tooltip="Work in progress">WIP</span> conflict visualization of archives through BSA specific conflict flags. BSA order is decided by plugins order like normal.
            </td>
            <td id="MO1">
                Change of how the game handles BSAs to allow BSA files to be loaded without plugins, in a different order, and allow BSA contents to overwrite loose files.
                Order is decided through mod priority and conflicts are shown as part of the normal loose assets conflicts.
                <b>Cons</b>: some mods and programs relied on the previous game behavior.
            </td>
            <td id="Vortex">
                <div class="cmp-no" />
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                <div class="cmp-yes" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Saves management</td>
            <td id="KMM.MO2.MO1.Vortex" colspan="4">
                Virtualization per profile and dedicated tab to visualize them.
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                Create separate folders for each playthrough or for easy management of save games and eliminate clutter in your save game list.
            </td>
        </tr>
        <tr>
            <td class="feature-name">Change master name in header of save game from save game manager</td>
            <td id="KMM">
                <div class="cmp-unknown" />
            </td>
            <td id="MO2">
                <div class="cmp-unknown" />
            </td>
            <td id="MO1">
                <div class="cmp-unknown" />
            </td>
            <td id="Vortex">
                <div class="cmp-unknown" />
            </td>
            <td id="NMM">
                <div class="cmp-unknown" />
            </td>
            <td id="WB">
                <div class="cmp-yes" tooltip="Not supported for compressed save game headers like in Skyrim SE"/>
            </td>
        </tr>
        <tr>
            <td class="feature-name">List files used in a save game</td>
            <td id="KMM">
                <div class="cmp-unknown" />
            </td>
            <td id="MO2">
                Through tooltip. Warning icon if current loadorder is missing files for them.
            </td>
            <td id="MO1">
                Through tooltip. Warning icon if current loadorder is missing files for them.
            </td>
            <td id="Vortex">
                <div class="cmp-unknown" />
            </td>
            <td id="NMM">
                <div class="cmp-unknown" />
            </td>
            <td id="WB">Copied to Clipboard</td>
        </tr>
        <tr>
            <td class="feature-name">Config (INI) management</td>
            <td id="KMM">
                Virtualization per profile and advanced editor.
            </td>
            <td id="MO2.MO1" colspan="2">
                Virtualization per profile and editor.
            </td>
            <td id="Vortex">
                Virtualization per profile.
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                Easily alter game INI files by placing a checkmark in a list of changes. Restore INI files back to default.
            </td>
        </tr>
        <tr>
            <td class="feature-name">Load order management scheme</td>
            <td id="KMM.MO2.MO1" colspan="3">
                Drag and Drop for mods and plugins.
            </td>
            <td id="Vortex">
                Based on LOOT and its own rules system.
            </td>
            <td id="NMM">
                Drag and Drop for plugins.
                For mods it's the install order...
            </td>
            <td id="WB">
                Drag and Drop for mods and plugins.
            </td>
        </tr>
        <tr>
            <td class="feature-name">Add or Remove ESL Flag from plugin list</td>
            <td id="KMM">
                <div class="cmp-no" />
            </td>
            <td id="MO2">
                <div class="cmp-no" />
            </td>
            <td id="MO1">
                <div class="cmp-no" />
            </td>
            <td id="Vortex">
                <div class="cmp-yes" />
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                <div class="cmp-yes" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Add or Remove ESM Flag from plugin list</td>
            <td id="KMM">
                <div class="cmp-no" />
            </td>
            <td id="MO2">
                <div class="cmp-no" />
            </td>
            <td id="MO1">
                <div class="cmp-no" />
            </td>
            <td id="Vortex">
                <div class="cmp-unknown" />
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                <div class="cmp-yes" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Change master name in header of plugin from plugin list</td>
            <td id="KMM">
                <div class="cmp-no" />
            </td>
            <td id="MO2">
                <div class="cmp-no" />
            </td>
            <td id="MO1">
                <div class="cmp-no" />
            </td>
            <td id="Vortex">
                <div class="cmp-unknown" />
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                <div class="cmp-yes" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Integrated LOOT support</td>
            <td id="KMM">
                One click sort.
            </td>
            <td id="MO2">
                One click sort, detailed loot report, visualization of loot info on plugins list through flags and tooltips.
            </td>
            <td id="MO1">
                One click sort, detailed loot report. <b>Severely outdated version of integrated LOOT.</b>
            </td>
            <td id="Vortex" class="text-left">
                Drag & drop dependency icon for specific "load after" rules.
                <br/>
                Full features native to LOOT.
                <br/>
                Group management including:	adding and removing groups, changing a plugins groups, order of groups.
                <br/>
                LOOT defaults
                <br/>
                Automatic LOOT updates
                <br/>
                Plugin details visible by double clicking plugin.
                <br/>
                Visualization of info through flags.
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                <div class="cmp-no" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Backup features</td>
            <td id="KMM">
                <div class="cmp-no" />
            </td>
            <td id="MO2.MO1" colspan="2">
                Modlist order,
                Plugin order,
                Individual mod folder backup.
            </td>
            <td id="Vortex">
                Via <a href="https://github.com/Garethp/Vortex-Modlist-Backup">extension</a> only.
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                Modlist order, plugin order, settings.
            </td>
        </tr>
        <tr>
            <td class="feature-name"><span tooltip="Program list means a toolbar of some sort that allows you to execute programs and tools from the mod manager for convenience or need">Program list</span></td>
            <td id="KMM">
                <div class="cmp-yes" />
            </td>
            <td id="MO2">
                <div class="cmp-yes" />
            </td>
            <td id="MO1">
                <div class="cmp-yes" />
            </td>
            <td id="Vortex">
                <div class="cmp-yes" />
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                <div class="cmp-yes" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Mod package creator</td>
            <td id="KMM">
                FOMod (XML), KMP.
            </td>
            <td id="MO2">
                <div class="cmp-no" />
            </td>
            <td id="MO1">
                <div class="cmp-no" />
            </td>
            <td id="Vortex">
                <div class="cmp-no" />
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                <div class="cmp-no" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Supported mod package formats</td>
            <td id="KMM">FOMod (XML), KMP.</td>
            <td id="MO2.MO1" colspan="2">
                Same as Vortex/NMM, plus <span tooltip="Any content inside the mod archive. This installation mode usually requires user input to some extent.">generic.</span>
            </td>
            <td id="Vortex.NMM" colspan="2">FOMod (XML, C#), BAIN Wizard, BAIN Package, <span tooltip="Just some files such as .esm/.esp and/or Data folder inside the mod archive.">simple</span>.</td>
            <td id="WB">
                FOMod (XML) Beta, BAIN Wizard, BAIN Package, simple2.
            </td>
        </tr>
        <tr>
            <td class="feature-name">Virtualization scheme</td>
            <td id="KMM">
                <b><a href="https://github.com/KerberX/KxVirtualFileSystem">KxVFS</a></b> - based on <b><a href="https://github.com/dokan-dev/dokany">Dokany</a></b> - kernel-mode file system driver.
            </td>
            <td id="MO2">
                <b><a href="https://github.com/ModOrganizer2/usvfs">USVFS</a></b> - user-mode WinAPI hooks, a successor to the <b>hook.dll</b> to support 64-bit applications.
            </td>
            <td id="MO1">
                <b>Hook.dll</b> - a user-mode WinAPI hooks (32-bit only).
            </td>
            <td id="Vortex">
                Hardlinks by default,
                Symlinks or move deployment as an option on specific games,
                <b>USVFS</b> as an experimental extension (no overwrite support for now).
            </td>
            <td id="NMM">
                Symbolic links (symlinks).
            </td>
            <td id="WB">
                No virtualization, files are installed in the Data folder.
            </td>
        </tr>
        <tr>
            <td class="feature-name">Virtualization Pros and Cons</td>
            <td id="KMM">
                <ul>
                    <b>Pros:</b>
                    <li class="li-pro">Game folder remains clean all the time as exclusively hooked programs see the mods.</li>
                    <li class="li-pro">Zero overhead when installing/enabling mods or switching profiles.</li>
                    <li class="li-pro">Mounted VFS is visible to all processes on the system (it's kind of a con too).</li>
                    <li class="li-pro">Can link mods across different drives.</li>
                    <li class="li-pro">Allows top-level folder virtualization.</li>
                </ul>
                <ul>
                    <b>Cons:</b>
                    <li class="li-con">Slower than alternatives.</li>
                    <li class="li-con">Requires target folder (mount point) to be empty.</li>
                    <li class="li-con">Huge memory consumption on large mod setups (~200 MB for 50k files, ~5 GB for 975k files).</li>
                    <li class="li-con">Program startup overhead for VFS initialization.</li>
                    <li class="li-con">Kernel-mode driver, requires administrator privileges to install.</li>
                </ul>
            </td>
            <td id="MO2">
                <ul>
                    <b>Pros:</b>
                    <li class="li-pro">Supports 64-bit programs.</li>
                    <li class="li-pro">Game folder remains clean all the time as exclusively hooked programs see the mods.</li>
                    <li class="li-pro">Zero overhead when installing/enabling mods or switching profiles.</li>
                    <li class="li-pro">Doesn't require write access to destination.</li>
                    <li class="li-pro">Sources and target can be linked across different drives.</li>
                    <li class="li-pro">Intercepts creation of new files in overwrite, keeping game folder clean</li>
                </ul>
                <ul>
                    <b>Cons:</b>
                    <li class="li-con">Can be less intuitive to users since only the programs can see the mods.</li>
                    <li class="li-con">Can often be flagged by anti-viruses.</li>
                    <li class="li-con">Can't virtualize some top level files like load-time linked .dll.</li>
                    <li class="li-con">Can lead to hard to diagnose issues in case there is a bug.</li>
                    <li class="li-con">Small memory and computation overhead at runtime.</li>
                    <li class="li-con">Program startup overhead for VFS initialization.</li>
                    <li class="li-con">Can potentially break if Windows changes something.</li>
                    <li class="li-con">Uses Overwrite folder that can require some user cleanup and management</li>
                </ul>
            </td>
            <td id="MO1">
                <ul>
                    <b>Pros:</b>
                    <li class="li-pro">Game folder remains clean all the time as exclusively hooked programs see the mods.</li>
                    <li class="li-pro">Zero overhead when installing/enabling mods or switching profiles.</li>
                    <li class="li-pro">Doesn't require write access to destination.</li>
                    <li class="li-pro">Sources and target can be linked across different drives.</li>
                    <li class="li-pro">Intercepts creation of new files in overwrite, keeping game folder clean</li>
                </ul>
                <ul>
                    <b>Cons:</b>
                    <li class="li-con">Can't be used for 64-bit programs.</li>
                    <li class="li-con">Can be less intuitive to users since only the programs can see the mods.</li>
                    <li class="li-con">Can often be flagged by anti-viruses.</li>
                    <li class="li-con">Can't virtualize some top level files like load-time linked .dll.</li>
                    <li class="li-con">Can lead to hard to diagnose issues in case there is a bug.</li>
                    <li class="li-con">Small memory and computation overhead at runtime.</li>
                    <li class="li-con">Program startup overhead for VFS initialization.</li>
                    <li class="li-con">Can potentially break if Windows changes something</li>
                    <li class="li-con">Uses Overwrite folder that can require some user cleanup and management</li>
                </ul>
            </td>
            <td id="Vortex">
                <ul>
                    <b>Pros:</b>
                    <li class="li-pro">Remains even when the program is not running</li>
                    <li class="li-pro">Faster than alternatives because NTFS</li>
                    <li class="li-pro">Deployment only necessary when adding/removing mods "set and forget"</li>
                    <li class="li-pro">Windows updates shouldn't break it.</li>
                </ul>
                <ul>
                    <b>Cons:</b>
                    <li class="li-con">Improper removal of mod will cause mods to remain</li>
                    <li class="li-con">"Dirty" data folder</li>
                    <li class="li-con">Some applications may see the hardlinks as duplicates and count them as used disk space</li>
                    <li class="li-con">Requires deployment</li>
                    <li class="li-con">Requires full write/read access to target folder</li>
                    <li class="li-con">Takes time (based on file count) to switch profiles and enable/disable mods</li>
                    <li class="li-con">Can't link across drives</li>
                    <li class="li-con">Can be less convenient to use for mod authors messing with mod files</li>
                </ul>
            </td>
            <td id="NMM">
                <ul>
                    <b>Pros:</b>
                    <li class="li-pro">Can link across partitions.</li>
                    <li class="li-pro">Mostly same as Hardlinks (assuming good implementation)</li>
                </ul>
                <ul>
                    <b>Cons:</b>
                    <li class="li-con">Requires administrator privileges to create symlinks.</li>
                    <li class="li-con">Can leave the game folder with mess in case stuff goes wrong</li>
                    <li class="li-con">Mostly the same cons as Hardlinks</li>
                </ul>
            </td>
            <td id="WB">
                <ul>
                    <b>Pros:</b>
                    <li class="li-pro">Does not add complexity and potential failure points with VFSs, simple and reliable file copy.</li>
					<li class="li-pro">No problems with leftover/broken links and other inconsistencies, just normal files (that are all kept track of)</li>
                    <li class="li-pro">Allows you to have Game and your downloaded mods on different drives.</li>
                </ul>
                <ul>
                    <b>Cons:</b>
                    <li class="li-con">Depending on the amount and size of files copying files to the Data folder may take a reasonable amount of time to an extended amount of time.</li>
                    <li class="li-con">Installing 3GB mods like Interesting NPCs can take a very extended amount of time.</li>
					<li class="li-con">Changing order, enabling/disabling mods, clearing to vanilla, requires file trasnfers to update Data tab contents (user decides when to apply changes so multiple onse can be applied in one step).</li>
					<li class="li-con">Requires full write/read access to target folder</li>
					<li class="li-con">Requires more space as mod archives can't be deleted (source of files to be copied in Data)</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td class="feature-name"><span tooltip="Sopport for installing mods in the game top level directory, such as SKSE, ENB...">Game root directory support</span></td>
            <td id="KMM">
                <div class="cmp-yes" />
            </td>
            <td id="MO2">
                <div class="cmp-no" />
            </td>
            <td id="MO1">
                <div class="cmp-no" />
            </td>
            <td id="Vortex">
                <div class="cmp-yes" />
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                <div class="cmp-no" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">New files handling (overwrite) and file changes</td>
            <td id="KMM">
                All new files end up in profile specific Overwrite.
                All changes to existing virtual files are transparently executed on the correct mod files.
            </td>
            <td id="MO2">
                All new files end up in overwrite folder.
                Ability to Drag & Drop files to existing mod, create a new mod, delete the files, leave the files there.
                Allows specifying a mod to use as overwrite for a particular program.
                All changes to existing virtual files are transparently executed on the correct mod files.
            </td>
            <td id="MO1">
                All new files end up in overwrite folder.
                Ability to Drag & Drop files to existing mod, create a new mod, delete the files, leave the files there.
                Changes to existing virtual files can sometimes lead to deleted mod files and new files in Overwrite.
            </td>
            <td id="Vortex">
                No handling of completely new files, those end up in the game folder.
                File changes without deletion are executed transparently on mod files.
                Deleted/recreated files don't automatically reflect on mod files.
                A popup for deleted/recreated files will ask user whether to use mod version or game version.
            </td>
            <td id="NMM">
                <div class="cmp-unknown" />
            </td>
            <td id="WB">
                Files added or altered in the Data folder are tracked. New files become orphaned files.
                Changed files that belong to an installer can be easily synced with the Installer (Package Only).
                Orphaned files can be removed using Clean Data.
                Using Monitor External Installation during an install of a mod from a 3rd part utility other then Wrye Bash will track the changes and add them to a Package to be used for installation.
                Using Monitor External Installation while using the CK or other Tools like FNIS, Wrye Bash will track the changes and add both new and altered files to a Package to be used for installation.
            </td>
        </tr>
        <tr>
            <td class="feature-name">VFS persistence</td>
            <td id="KMM">
                Mod manager runtime.
            <td id="MO2.MO1" colspan="2">
                Only present for programs started through the manager and their child applications.
            </td>
            <td id="Vortex">
                For hardlinks, move, and symlinks it persists until purged.
                For USVFS - same as MO2/MO1.
            </td>
            <td id="NMM">
                Symlinks - persists until purged.
            </td>
            <td id="WB">
                <div class="cmp-no" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Game (virtual) folder layout visualization</td>
            <td id="KMM">
                Using any file explorer (when VFS is enabled) or with dedicated Data tab.
            </td>
            <td id="MO2">
                Through dedicated Data tab, or running Explorer++ and similar programs through MO2.
            </td>
            <td id="MO1">
                Through dedicated Data tab.
            </td>
            <td id="Vortex.NMM.WB" colspan="3">
                Not needed as Explorer already shows the correct final result.
            </td>
        </tr>
        <tr>
            <td class="feature-name">Websites-integrations</td>
            <td id="KMM"><b>NexusMods</b>, partially <b>LoversLab</b> and <b>tesall.ru</b></td>
            <td id="MO2.MO1.Vortex.NMM" colspan="4">
                <b>NexusMods</b>
            </td>
            <td id="WB">
                <div class="cmp-no" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Nexus Mods Integration features</td>
            <td id="KMM">
                Account login,
                Download with manager,
                Mod update check
            <td id="MO2">
                Account login,
                Download with manager,
                Mod update check,
                Mod endorsements,
                Mod tracking,
                Mod meta info like categories etc,
                Browser for mod description,
            </td>
            <td id="MO1">
                Integration no longer works since Nexus API update.
            <td id="Vortex">
                Account login,
                Download with manager,
                Mod update check,
                Automatic mod update for premium users,
                Mod endorsements,
                Mod tracking,
                Mod meta info like categories etc,
                Browser for mod description,
            </td>
            <td id="NMM">
                Account login,
                Download with manager,
                Mod endorsements,
                Mod meta info like categories etc.
            </td>
            <td id="WB">
                <div class="cmp-no" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Web-integration extent</td>
            <td id="KMM">
                <b>NexusMods</b>: Account, mod updates, mod source, downloads.
                <br>
                <b>LoversLab</b>: mod source.
                <b>TESALL.RU</b>: mod source.
            </td>
            <td id="MO2.MO1.Vortex.NMM" colspan="4">
                Account, mod updates, mod source, metadata query, downloads, endorsements.
            </td>
            <td id="WB">
                <div class="cmp-no" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">
                Self auto-update check
            </td>
            <td id="KMM">
                <div class="cmp-no" />
            </td>
            <td id="MO2">
                <div class="cmp-yes" />
            </td>
            <td id="MO1">
                <div class="cmp-no" />
            </td>
            <td id="Vortex">
                <div class="cmp-yes" />
            </td>
            <td id="NMM">
                <div class="cmp-yes" />
            </td>
            <td id="WB">
                <div class="cmp-no" />
            </td>
        </tr>
        <tr>
            <td class="feature-name" align="left"><span tooltip="The mod manager has internal download capabilities and uses can see the files being downloaded." >Integrated download manager</td>
            <td id="KMM">
                <div class="cmp-yes" />
            </td>
            <td id="MO2">
                <div class="cmp-yes" />
            </td>
            <td id="MO1">
                <div class="cmp-no" />
            </td>
            <td id="Vortex">
                <div class="cmp-yes" />
            </td>
            <td id="NMM">
                <div class="cmp-yes" />
            </td>
            <td id="WB">
                <div class="cmp-no" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Mod categorization</td>
            <td id="KMM">
                Custom tag system.
            </td>
            <td id="MO2">
                Import from Nexus, support for custom categories.
            </td>
            <td id="MO1">
                Broken import from Nexus, support for custom categories.
            </td>
            <td id="Vortex">
                Import from Nexus, support for custom categories.
            </td>
            <td id="NMM">
                Import from nexus, custom categories?
            </td>
            <td id="WB">
                Custom marker system.
            </td>
        </tr>
        <tr>
            <td class="feature-name">Problem detection</td>
            <td id="KMM">
                <div class="cmp-no" />
            </td>
            <td id="MO2">
                Plugins with missing masters,
                Files in overwrite,
                Form 43 plugins for Skyrim SE,
                Script Extender plugin load failure,
                Need to run FNIS,
                Presence of files with attributes that the game can't handle.
            </td>
            <td id="MO1">
                Plugins with missing masters,
                Files in overwrite,
                Need to run FNIS.
            </td>
            <td id="Vortex">
                Bug report automation in menu.
                Notifications in the bell icon including xSE error notifications.
                Missing masters?
                Form 43 with loot?
            </td>
            <td id="NMM">
                Missing masters.
            </td>
            <td id="WB">
                Missing masters, out of order masters, missing strings files, Form 43 plugins for Skyrim SE, ESL verification, File using same date for games that utilize a time date stamp, ghosted plugin, dirty edits, improperly configured plugins marked with Deactivate or NoMerge.
            </td>
        </tr>
        <tr>
            <td class="feature-name">GUI customization</td>
            <td id="KMM">
                <div class="cmp-no" />
            </td>
            <td id="MO2">
                Comes with 15+ built in styles, more available on Nexus.
                Styles created through QSS files, with CSS like syntax.
                Very limited color customization for highlights and separators from menu.
            </td>
            <td id="MO1">
                Comes with some built in styles, more available on Nexus.
                Styles created through QSS files, with CSS like syntax.
            </td>
            <td id="Vortex">
                Comes with 3 built in styles, several on nexus, customizable via CSS/SASS.
                Change colors and fonts via a setting menu.
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                Change colors via a settings menu.
            </td>
        </tr>
        <tr>
            <td class="feature-name">Exporting mod/plugins list</td>
            <td id="KMM">
                Non-customizable export of modlist to HTML file.
                Import and export of plugins list.
            </td>
            <td id="MO2">
                Customizable modlist export to CSV file.
                Otherwise only simple copy of loadorder.txt and modlist.txt.
            </td>
            <td id="MO1">
                Less customizable modlist export to CSV file.
            </td>
            <td id="Vortex">
                <div class="cmp-unknown" />
            </td>
            <td id="NMM">
                Can export mod list.
            </td>
            <td id="WB">
                <ul>
                    <li>Can export list of installed mods from Mods tab.</li>
                    <li>Can view and list files in an archive or package from Installers Tab.</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td class="feature-name">Modlist features</td>
            <td id="KMM">
                Color-coding of each mod,
                collapsible separators (using tag system).
                <br/><br/>
                <b>Mod metadata:</b>
                name,
                priority,
                version,
                author,
                tags,
                sources,
                date installed,
                date uninstalled,
                folder,
                install package,
                signature.
            </td>
            <td id="MO2">
                Grouping by category or Nexus ID.<br/>
                Filtering with advanced filters and RegEx search.<br/>
                Colored separators.<br/>
                Advanced mod counters.<br/>
                Conflict highlighting on selection.<br/>
                Plugin Highlighting on selection.<br/>
                Full refresh from disk.<br/>
                Backup, export as CSV.<br/>
                Update check.<br/>
                Various Keyboard shortcuts and support for multi selection operations.<br/>
                <br/>
                <b>Mod metadata:</b>
                conflicts,
                categories,
                contents,
                version,
                target game,
                custom notes,
                install time,
                Nexus ID,
                flags (endorsed, tracked, hidden files, valid).
            </td>
            <td id="MO1">
                Mostly same as MO2, but somethings are missing like separators and notes.
            </td>
            <td id="Vortex">
                Grouping by state, category, content, author, version, source..<br/>
                Filter by state, name, version, install time, category, endorsed state, content, dependency resolution.<br/>
                Update check, install from file, manage rules.<br/>
                <br/>
                <b>Mod metadata:</b>
                mod name,
                version number,
                author,
                install time,
                source (nexus or not),
                category,
                endorsed,
                content,
                priority,
                dependencies,
                highlight
            </td>
            <td id="NMM">
                <div class="cmp-unknown" />
            </td>
            <td id="WB">
                <div class="cmp-no" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Game files preview</td>
            <td id="KMM">
                <div class="cmp-no" />
            </td>
            <td id="MO2">
                .DDS Textures complete preview support.<br/>
                BSA/BA2 content preview.<br/>
                Normal images and text (.txt, .ini, .cfg, .log, .json) files.
            </td>
            <td id="MO1">
                .DDS Textures partial preview support.<br/>
                Normal images and .txt files.
            </td>
            <td id="Vortex">
                <div class="cmp-no" />
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                <ul>
                    <li>Can preview contents of archive or Package.</li>
                    <li>Doc Browser for Readme files and custom Docs or Notes added to mods.</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td class="feature-name">
                Available translations
                <br>
                <i>(default distribution)</i>
            </td>
            <td id="KMM">
                <ul>
                    <li>English</li>
                    <li>Russian</li>
                </ul>
            </td>
            <td id="MO2">
                <ul>
                    <li>English</li>
                    <li>French</li>
                    <li>Dutch</li>
                    <li>Polish</li>
                    <li>Italian</li>
                    <li>Spanish</li>
                    <li>Russian</li>
                    <li>Portuguese</li>
                    <li>Chinese</li>
                    <li>Japanese</li>
                </ul>
            </td>
            <td id="MO1">
                <ul>
                    <li>English</li>
                    <li>..?</li>
                </ul>
            </td>
            <td id="Vortex">
                <ul>
                    <li>English</li>
                    <li>German</li>
                    <li>Various Extensions</li>
                </ul>
            </td>
            <td id="NMM">
                <ul>
                    <li>English</li>
                </ul>
            </td>
            <td id="WB">
                <ul>
                    <li>English</li>
                    <li>Localization can be easily exported and edited with Notepad++ or Poedit. Language changes are very outdated.</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td class="feature-name">Extensions</td>
            <td id="KMM">
                <div class="cmp-no"/>
            </td>
            <td id="MO2.MO1" colspan="2">
                Bundled with some extensions, additional are available at <b>NexusMods</b> and <b>GitHub</b>.
                Supports C++ and Python extensions.
            </td>
            <td id="Vortex">
                Additional extensions are available at <b>NexusMods</b>.
                Supports JavaScript, C++, and various other electron extensions.
                Extensions are available to install from within the program.
            </td>
            <td id="NMM">
                <div class="cmp-no" />
            </td>
            <td id="WB">
                <div class="cmp-no" />
            </td>
        </tr>
        <tr>
            <td class="feature-name">Current development state and maintainers</td>
            <td id="KMM">
                Open Source, fully released.
                Maintained and not so much actively developed on by Kerber (non-paid developer), with highly irregular feature updates.
            </td>
            <td id="MO2">
                Open Source, fully released.
                Maintained and actively developed on by MO2 Team (community non-paid developers), with regular feature updates.
            </td>
            <td id="MO1">
                Open Source, fully released.
                No longer maintained or updated.
            </td>
            <td id="Vortex">
                Open Source, fully released.
                Maintained and actively developed by NexusMods (paid professionals), with frequent feature updates and fixes.
            </td>
            <td id="NMM">
                Open Source, beta release.
                Discontinued by NexusMods in favor of Vortex.
                Currently maintained by a few members of the community.
            </td>
            <td id="WB">
                Open Source, fully released.
                Maintained and actively developed by Wrye Bash Team (community non-paid developers) with improvement updates.
            </td>
        </tr>
    </tbody>
</table>

## Page contributors
* [Kerber](https://www.nexusmods.com/users/2734453) (Kortex developer)
* [AL](https://www.nexusmods.com/users/6409802) (MO2 Dev)
* [Yggdrasil75](https://www.nexusmods.com/users/25321034) (Vortex virtuoso)
* [Sharlikran](https://www.nexusmods.com/users/25321034) (Wrye Bash contributor)
* We found no one for NMM yet :/
