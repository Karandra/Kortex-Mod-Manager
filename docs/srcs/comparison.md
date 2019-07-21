# Features and comparison to other mod managers

There's a few mod managers around these days and it's may be a bit difficult to choose the one you need. This page is designed to highlight key features of known mod managers.

## Legend

* **KMM** - Kortex Mod Manager.
* **MO1** - [Mod Organizer 1](https://www.nexusmods.com/skyrim/mods/1334) (version 1.x).
* **MO2** - [Mod Organizer 2](https://www.nexusmods.com/skyrimspecialedition/mods/6194) (version 2.x).
* **NMM** - [Nexus Mod Manager](https://www.nexusmods.com/site/mods/4).
* [**Vortex**](https://www.nexusmods.com/about/vortex) - a successor to **NMM** by Nexus Mods.

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
		</tr>
		<tr>
			<td class="feature-name">Supported games</td>
			<td id="KMM">
				TES games starting from Morrowind.
				<br>
				Fallout series starting from <b>Fallout 3</b> and <b>Sacred 2</b>.
			</td>
			<td id="MO2">
				TES games starting from Morrowind.
				<br>
				Fallout series starting from <b>Fallout 3</b>.
			</td>
			<td id="MO1">
				Oblivion, Skyrim, Fallout 3, Fallout: New Vegas.
			</td>
			<td id="Vortex">
				Supports over 50 games including TES and Fallout games.
			</td>
			<td id="NMM">
				Supports many games including TES and Fallout games.
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
			<td id="Vortex">
				Multiple profiles per game
			</td>
			<td id="NMM">
				Multiple profiles per game.
			</td>
		</tr>
		<tr>
			<td class="feature-name">Assets conflict management</td>
			<td id="KMM">
				<div class="cmp-no" />
			</td>
			<td id="MO1">
				<div class="cmp-yes" />
			</td>
			<td id="MO2">
				<div class="cmp-yes" />
			</td>
			<td id="Vortex">
				<div class="cmp-yes" />
			</td>
			<td id="NMM">
				<div class="cmp-no" />
			</td>
		</tr>
		<tr>
			<td class="feature-name">Saves management</td>
			<td id="KMM">
				<div class="cmp-yes" />
			</td>
			<td id="MO1">
				<div class="cmp-yes" />
			</td>
			<td id="MO2">
				<div class="cmp-yes" />
			</td>
			<td id="Vortex">
				<div class="cmp-yes" />
			</td>
			<td id="NMM">
				<div class="cmp-no" />
			</td>
		</tr>
		<tr>
			<td class="feature-name">Config (INI) management</td>
			<td id="KMM">
				<div class="cmp-yes" />
			</td>
			<td id="MO1">
				<div class="cmp-yes" />
			</td>
			<td id="MO2">
				<div class="cmp-yes" />
			</td>
			<td id="Vortex">
				<div class="cmp-no" />
			</td>
			<td id="NMM">
				<div class="cmp-no" />
			</td>
		</tr>
		<tr>
			<td class="feature-name">Game archives (BSA) management</td>
			<td id="KMM">
				<div class="cmp-no" />
			</td>
			<td id="MO2">
				<div class="cmp-no" />
			</td>
			<td id="MO1">
				<div class="cmp-yes" />
			</td>
			<td id="Vortex">
				<div class="cmp-no" />
			</td>
			<td id="NMM">
				<div class="cmp-no" />
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
				Mods ..?
			</td>
		</tr>
		<tr>
			<td class="feature-name">Integrated LOOT support</td>
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
		</tr>
		<tr>
			<td class="feature-name">Program list</td>
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
		</tr>
		<tr>
			<td class="feature-name">Supported mod package formats</td>
			<td id="KMM">FOMod (XML), KMP.</td>
			<td id="MO2.MO1" colspan="2">
				FOMod (XML, C#), freeform<sup>1</sup>, simple<sup>2</sup>.
			</td>
			<td id="Vortex.NMM" colspan="2">FOMod (XML, C#), simple.</td>
		</tr>
		<tr>
			<td class="feature-name">Virtualization scheme</td>
			<td id="KMM">
				<b><a href="https://github.com/KerberX/KxVirtualFileSystem">KxVFS</a></b> - based on <b><a href="https://github.com/dokan-dev/dokany">Dokany</a></b> - kernel-mode file system driver.
			</td>
			<td id="MO2">
				<b><a href="https://github.com/ModOrganizer2/usvfs">USVFS</a></b> - user-mode WinAPI hooks, a successor to the <b>hook.dll</b>.
			</td>
			<td id="MO1">
				<b>Hook.dll</b> - a user-mode WinAPI hooks.
			</td>
			<td id="Vortex">
				Hardlinks by default, symlinks as option (not recommended), <b>USVFS</b> as an extension and files move on certain specific games.
			</td>
			<td id="NMM">
				Symlinks
			</td>
		</tr>
		<tr>
			<td class="feature-name">Game root directory virtualization</td>
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
		</tr>
		<tr>
			<td class="feature-name">VFS persistence</td>
			<td id="KMM.MO2.MO1" colspan="3">
				Mod manager runtime.
			</td>
			<td id="Vortex">
				For hardlinks and symlinks it persists until purged.
				For USVFS - mod manager runtime.
			</td>
			<td id="NMM">
				Symlinks - persists until purged.
			</td>
		</tr>
		<tr>
			<td class="feature-name">Web-integration</td>
			<td id="KMM"><b>NexusMods</b>, partially <b>LoversLab</b> and <b>tesall.ru</b></td>
			<td id="MO2.MO1.Vortex.NMM" colspan="4">
				<b>NexusMods</b>
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
		</tr>
		<tr>
			<td class="feature-name">
				Auto-update check
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
		</tr>
		<tr>
			<td class="feature-name" align="left">Integrated download manager</td>
			<td id="KMM">
				<div class="cmp-yes" />
			</td>
			<td id="MO1">
				<div class="cmp-yes" />
			</td>
			<td id="MO2">
				<div class="cmp-yes" />
			</td>
			<td id="Vortex">
				<div class="cmp-yes" />
			</td>
			<td id="NMM">
				<div class="cmp-yes" />
			</td>
		</tr>
		<tr class="tooltip">
			<td class="feature-name">
				Available translation
				<br>
				<i>default distribution</i>
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
				</ul>
			</td>
			<td id="NMM">
				<ul>
					<li>English</li>
				</ul>
			</td>
		</tr>
		<tr>
			<td class="feature-name">Extensions</td>
			<td id="KMM">
				<div class="cmp-no" />
			</td>
			<td id="MO2.MO1" colspan="2">
				Bundeled with some extensions, additional are available at <b>NexusMods</b>.
			</td>
			<td id="Vortex">
				Additional extensions are available at <b>NexusMods</b>.
			</td>
			<td id="NMM">
				<div class="cmp-no" />
			</td>
		</tr>
	</tbody>
</table>

1. Any content inside the mod archive. This installation mode usually requires user input to some extent.
2. Just some files such as **.esm**/**.esp** and/or **Data** folder inside the mod archive.
