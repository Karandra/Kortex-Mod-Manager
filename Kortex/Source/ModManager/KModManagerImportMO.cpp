#include "stdafx.h"
#include "KModManagerImportMO.h"
#include "SaveManager/KSaveManager.h"
#include "PluginManager/KPluginManager.h"
#include "PackageProject/KPackageProjectSerializer.h"
#include "GameConfig/KGameConfigWorkspace.h"
#include "ProgramManager/KProgramManager.h"
#include "ProgramManager/KProgramWorkspace.h"
#include "Network/KNetwork.h"
#include "DownloadManager/KDownloadManager.h"
#include "GameInstance/Config/KSaveManagerConfig.h"
#include "GameInstance/Config/KConfigManagerConfig.h"
#include "GameInstance/KGameInstance.h"
#include "Profile/KProfile.h"
#include "KOperationWithProgress.h"
#include "KAux.h"
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxINI.h>

wxString& KModManagerImportMO::DecodeUTF8(wxString& string) const
{
	// Find and replace all '\xABC' 5-char hex patterns to corresponding UTF-8 codes
	for (size_t i = 0; i < string.Length(); i++)
	{
		size_t pos = string.find("\\x", i);
		if (pos != wxString::npos)
		{
			unsigned long value = 0;
			if (string.Mid(pos + 2, 3).ToCULong(&value, 16) && value != 0)
			{
				wxUniChar c(value);
				string.replace(pos, 5, c);
			}
		}
	}
	return string;
}
wxString& KModManagerImportMO::ProcessFilePath(wxString& path) const
{
	DecodeUTF8(path);

	path.Replace("/", "\\");
	path.Replace("\\\\", "\\");
	path.Replace("\\\"", "\"");
	return path;
}
wxString& KModManagerImportMO::ProcessDescription(wxString& path) const
{
	DecodeUTF8(path);

	// Fix line terminators
	path.Replace("\\r\\n", "\r\n");
	path.Replace("\\r", "\r");
	path.Replace("\\n", "\n");

	// Escape mnemonics
	path.Replace("&#39;", "\'");
	path.Replace("&#34;", "\"");
	path.Replace("&#38;", "&");
	path.Replace("&60;", "<");
	path.Replace("&62;", ">");
	path.Replace("&33;", "!");

	// Remove quotes from beginning and at the end
	if (!path.IsEmpty())
	{
		if (path[0] == '\"')
		{
			path.Remove(0, 1);
		}
	}
	if (!path.IsEmpty())
	{
		if (path.Last() == '\"')
		{
			path.RemoveLast(1);
		}
	}

	// Convert BB code
	path = KPackageProjectSerializer::ConvertBBCode(path);
	return path;
}

KGameID KModManagerImportMO::GetGameID(const wxString& name)
{
	if (!name.IsEmpty())
	{
		// TES
		if (name == "Morrowind")
		{
			return KGameIDs::Morrowind;
		}
		if (name == "Oblivion")
		{
			return KGameIDs::Oblivion;
		}
		if (name == "Skyrim")
		{
			return KGameIDs::Skyrim;
		}
		if (name == "Skyrim Special Edition")
		{
			return KGameIDs::SkyrimSE;
		}
		if (name == "Skyrim VR")
		{
			return KGameIDs::SkyrimVR;
		}

		// Fallout
		if (name == "Fallout 3")
		{
			return KGameIDs::Fallout3;
		}
		if (name == "New Vegas" || name == "TTW")
		{
			return KGameIDs::FalloutNV;
		}
		if (name == "Fallout 4")
		{
			return KGameIDs::Fallout4;
		}
		if (name == "Fallout 4 VR")
		{
			return KGameIDs::Fallout4VR;
		}
	}
	return KGameIDs::NullGameID;
}
void KModManagerImportMO::LoadOptions()
{
	// Game name
	m_TargetProfile = GetGameID(m_Options.GetValue("General", "gameName"));
	m_TargetProfileTemplate = KGameInstance::GetTemplate(m_TargetProfile);
	m_ModManagerName = m_TargetProfile.IsOK() ? "Mod Organizer 2.x" : "Mod Organizer 1.x";
	
	// Current mod list
	m_CurrentModList = m_Options.GetValue("General", "selected_profile");

	// Directories
	wxString baseDirectory = m_Options.GetValue("Settings", "base_directory");
	if (!baseDirectory.IsEmpty())
	{
		m_InstanceDirectory = baseDirectory;
	}

	auto LoadAndExpand = [this](wxString& saveInto, const wxString& optionName, const wxString& defaultPath)
	{
		wxString value = m_Options.GetValue("Settings", optionName);
		if (!value.IsEmpty())
		{
			ProcessFilePath(value);
			value.Replace("%BASE_DIR%", m_InstanceDirectory);
			saveInto = value;
		}
		else
		{
			saveInto = m_InstanceDirectory + '\\' + defaultPath;
		}
	};
	LoadAndExpand(m_DownloadsDirectory, "download_directory", "downloads");
	LoadAndExpand(m_ModsDirectory, "mod_directory", "mods");
	LoadAndExpand(m_ProfilesDirectory, "profiles_directory", "profiles");
}
wxString KModManagerImportMO::GetDataFolderName() const
{
	if (m_TargetProfile == KGameIDs::Morrowind)
	{
		return "Data Files";
	}
	else
	{
		return "Data";
	}
}
wxString KModManagerImportMO::GetProfileDirectory() const
{
	return m_ProfilesDirectory + '\\' + GetProfileToImport();
}

void KModManagerImportMO::ReadExecutables(KOperationWithProgressDialogBase* context)
{
	context->SetDialogCaption(wxString::Format("%s \"%s\"", T("Generic.Import"), KProgramManager::GetInstance()->GetName()));

	KProgramEntry::Vector& programList = KProgramManager::GetInstance()->GetProgramList();
	KProgramEntry* pCurrentEntry = NULL;

	const wxString sectionName("customExecutables");
	long count = -1;
	if (m_Options.GetValue(sectionName, "size").ToCLong(&count) && count > 0)
	{
		for (int i = 1; i <= count; i++)
		{
			if (!context->CanContinue())
			{
				return;
			}

			auto GetValue = [this, &sectionName, i](const auto& name)
			{
				return m_Options.GetValue(sectionName, wxString::Format("%d\\%s", i, name));
			};

			KProgramEntry& entry = programList.emplace_back();
			entry.SetName(GetValue("title"));
			entry.SetExecutable(ProcessFilePath(GetValue("binary")));
			entry.SetArguments(ProcessFilePath(GetValue("arguments")));
			entry.SetWorkingDirectory(ProcessFilePath(GetValue("workingDirectory")));

			if (entry.GetName().IsEmpty())
			{
				entry.SetName(entry.GetExecutable().AfterLast('\\').BeforeLast('.'));
			}
		}
		KProgramManager::GetInstance()->Save();
	}
}
void KModManagerImportMO::CopySaves(KOperationWithProgressDialogBase* context)
{
	if (const KSaveManager* saveManager = KSaveManager::GetInstance())
	{
		context->SetDialogCaption(wxString::Format("%s \"%s\"", T("Generic.Import"), KSaveManager::GetInstance()->GetName()));

		KxEvtFile source(GetProfileDirectory() + "\\Saves");
		context->LinkHandler(&source, KxEVT_FILEOP_COPY_FOLDER);
		source.CopyFolder(KxFile::NullFilter, saveManager->GetSavesLocation(), true, true);
	}
}
void KModManagerImportMO::CopyMods(KOperationWithProgressDialogBase* context)
{
	auto GetModFolder = [this](const wxString& name)
	{
		return m_ModsDirectory + '\\' + name;
	};

	size_t counter = 0;
	KxStringVector modsList = KxTextFile::ReadToArray(GetProfileDirectory() + "\\ModList.txt");
	
	// MO stores mod list in reverse order.
	for (auto it = modsList.rbegin(); it != modsList.rend(); ++it)
	{
		wxString& name = *it;

		counter++;
		if (!context->CanContinue())
		{
			return;
		}

		if (!name.IsEmpty())
		{
			wxUniChar c = name[0];
			if (c == '+' || c == '-')
			{
				bool isModEnabled = c == '+';

				// Remove state char
				name.Remove(0, 1);

				// Notify
				context->SetDialogCaption(wxString::Format("%s \"%s\", %zu/%zu", T("Generic.Import"), name, counter, modsList.size()));

				// Check mod existence
				KModEntry* existingMod = KModManager::Get().FindModByID(name);
				if (existingMod && ShouldSkipExistingMods())
				{
					existingMod->SetEnabled(isModEnabled);
					continue;
				}

				// Load 'meta.ini'
				wxString modFolder = GetModFolder(name);
				KxFileStream metaStream(modFolder + "\\Meta.ini");
				KxINI tModINI(metaStream);

				// Write data
				KModEntry* entry = KModManager::Get().GetEntries().emplace_back(new KModEntry());
				entry->SetName(name);
				entry->SetEnabled(isModEnabled);
				entry->SetVersion(tModINI.GetValue("General", "version"));
				entry->SetInstallPackageFile(ProcessFilePath(tModINI.GetValue("General", "installationFile")));
				entry->SetDescription(ProcessDescription(tModINI.GetValue("General", "nexusDescription")));

				// NexusID
				int64_t nexusID = tModINI.GetValueInt("General", "modid", KNETWORK_SITE_INVALID_MODID);
				if (nexusID > 0)
				{
					entry->SetWebSite(KNETWORK_PROVIDER_ID_NEXUS, nexusID);
				}

				// Install date
				entry->SetTime(isModEnabled ? KME_TIME_INSTALL : KME_TIME_UNINSTALL, KxFile(modFolder).GetFileTime(KxFILETIME_CREATION));

				// If such mod already exist, try create unique ID
				if (existingMod)
				{
					entry->SetID(wxString::Format("[MO] %s", name));
				}
				else
				{
					entry->SetID(name);
				}
				
				// Save
				entry->CreateAllFolders();
				entry->Save();

				// Copy mod contents
				wxString destination = entry->GetModFilesDir() + wxS('\\') + GetDataFolderName();

				KxEvtFile source(modFolder);
				context->LinkHandler(&source, KxEVT_FILEOP_COPY_FOLDER);
				source.CopyFolder(KxFile::NullFilter, destination, true, true);

				// Remove 'meta.ini' file
				KxFile(destination + "\\Meta.ini").RemoveFile();
			}
		}
	}

	// Sort mods. If mod entries was created here, they already sorted,
	// if they was skipped, sort is needed.
	KProfile* profile = KGameInstance::GetActiveProfile();

	KProfileMod::Vector& currentModList = profile->GetMods();
	currentModList.clear();
	for (const wxString& name: modsList)
	{
		if (!context->CanContinue())
		{
			return;
		}

		if (KModEntry* existingMod = KModManager::Get().FindModByID(name))
		{
			currentModList.emplace_back(KProfileMod(*existingMod, existingMod->IsEnabled()));
		}
	}
	profile->Save();
}
void KModManagerImportMO::ReadPlugins(KOperationWithProgressDialogBase* context)
{
	if (KPluginManager* manager = KPluginManager::GetInstance())
	{
		context->SetDialogCaption(wxString::Format("%s \"%s\"", T("Generic.Import"), manager->GetName()));

		// Load entries
		manager->Load();

		KxStringVector activePlugins = KxTextFile::ReadToArray(GetProfileDirectory() + "\\Plugins.txt");

		KProfile* profile = KGameInstance::GetActiveProfile();

		KProfilePlugin::Vector& currentPluginsList = profile->GetPlugins();
		currentPluginsList.clear();
		for (wxString& name: KxTextFile::ReadToArray(GetProfileDirectory() + "\\LoadOrder.txt"))
		{
			if (!context->CanContinue())
			{
				return;
			}

			if (!name.IsEmpty())
			{
				wxUniChar c = name[0];
				if (c == '*')
				{
					name.Remove(0, 1);
				}
			}
			currentPluginsList.emplace_back(KProfilePlugin(name, KAux::IsStringsContain(activePlugins, name, false)));
		}
		profile->Save();
	}
}
void KModManagerImportMO::CopyGameConfig(KOperationWithProgressDialogBase* context)
{
	if (const KConfigManagerConfig* options = KConfigManagerConfig::GetInstance())
	{
		context->SetDialogCaption(wxString::Format("%s \"%s\"", T("Generic.Import"), KGameConfigWorkspace::GetInstance()->GetName()));

		for (size_t i = 0; i < options->GetEntriesCount(); i++)
		{
			if (!context->CanContinue())
			{
				return;
			}

			const KConfigManagerConfigEntry* entry = options->GetEntryAt(i);
			if (entry->IsGameConfigID())
			{
				KxFile file(GetProfileDirectory() + '\\' + entry->GetFileName());
				if (file.IsFileExist())
				{
					file.CopyFile(entry->GetFilePath(), true);
				}
			}
		}
	}
}
void KModManagerImportMO::CopyDownloads(KOperationWithProgressDialogBase* context)
{
	KDownloadManager* manager = KDownloadManager::GetInstance();
	manager->PauseAllActive();

	KxFileFinder finder(m_DownloadsDirectory, "*.meta");
	KxFileItem item = finder.FindNext();
	while (item.IsOK())
	{
		KxEvtFile archiveFile(item.GetFullPath().BeforeLast('.'));
		if (item.IsNormalItem() && item.IsFile())
		{
			KxFileStream stream(item.GetFullPath(), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING);
			KxINI ini(stream);

			KDownloadEntry& entry = *manager->GetDownloads().emplace_back(new KDownloadEntry());
			entry.SetTargetProfile(ini.GetValue("General", "gameName"));
			entry.SetProvider(KNetwork::GetInstance()->FindProvider(ini.GetValue("General", "repository")));
			entry.SetDate(archiveFile.GetFileTime(KxFileTime::KxFILETIME_CREATION));

			entry.GetDownloadInfo().SetURL(ini.GetValue("General", "url").AfterFirst('"').BeforeLast('"'));

			entry.GetFileInfo().SetModID(ini.GetValueInt("General", "modID", -1));
			entry.GetFileInfo().SetID(ini.GetValueInt("General", "fileID", -1));
			entry.GetFileInfo().SetName(item.GetName().BeforeLast('.'));
			entry.GetFileInfo().SetDisplayName(ini.GetValue("General", "modName"));
			entry.GetFileInfo().SetVersion(ini.GetValue("General", "version"));

			int64_t size = archiveFile.GetFileSize();
			entry.GetFileInfo().SetSize(size);
			entry.SetDownloadedSize(size);

			entry.SetPaused(ini.GetValueBool("General", "paused", false));
			entry.SetHidden(ini.GetValueBool("General", "removed", false));

			if (entry.IsOK())
			{
				manager->AutoRenameIncrement(entry);
				entry.Serialize();

				context->LinkHandler(&archiveFile, KxEVT_FILEOP_COPY);
				archiveFile.CopyFile(entry.GetFullPath(), false);
			}
			else
			{
				manager->GetDownloads().pop_back();
			}
		}
		item = finder.FindNext();
	}
}

void KModManagerImportMO::SetDirectory(const wxString& path)
{
	m_InstanceDirectory = path;
	if (!KxFileFinder::IsDirectoryEmpty(m_InstanceDirectory))
	{
		KxFileStream stream(m_InstanceDirectory + '\\' + "ModOrganizer.ini", KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING);
		m_Options.Load(stream);
		if (m_Options.IsOK())
		{
			LoadOptions();
			m_CanImport = true;
		}
	}
}
void KModManagerImportMO::Import(KOperationWithProgressDialogBase* context)
{
	if (context->CanContinue())
	{
		ReadExecutables(context);
	}
	if (context->CanContinue())
	{
		CopyGameConfig(context);
	}
	if (context->CanContinue())
	{
		CopySaves(context);
	}
	if (context->CanContinue())
	{
		CopyDownloads(context);
	}

	// Mods
	if (context->CanContinue())
	{
		CopyMods(context);
	}

	// This should go after mods
	if (context->CanContinue())
	{
		ReadPlugins(context);
	}
}

bool KModManagerImportMO::CanImport() const
{
	if (m_CanImport)
	{
		// Allow import from unknown managed game, as MO1 doesn't store its name.
		if (m_TargetProfile.IsOK())
		{
			return m_TargetProfileTemplate != NULL;
		}
		return true;
	}
	return false;
}
wxString KModManagerImportMO::GetAdditionalInfo() const
{
	wxString info;

	// Version
	wxString version = m_Options.GetValue("General", "version");
	if (version.IsEmpty())
	{
		version = KxFile(m_InstanceDirectory + '\\' + "ModOrganizer.exe").GetVersionInfo().GetString("FileVersion");
		if (version.IsEmpty())
		{
			version = "<unknown>";
		}
	}
	info << T("Generic.Version") << ": " << version;

	// Target profile
	if (m_TargetProfileTemplate)
	{
		info << "\r\n" << T("ModManager.Import.ManagedGame") << ": " << m_TargetProfileTemplate->GetName();
	}

	return info;
}
KxStringVector KModManagerImportMO::GetProfilesList() const
{
	KxStringVector list;

	KxFileFinder finder(m_InstanceDirectory + "\\profiles");
	KxFileItem item = finder.FindNext();
	while (item.IsOK())
	{
		if (item.IsNormalItem() && item.IsDirectory() && !KxFileFinder::IsDirectoryEmpty(item.GetFullPath()))
		{
			list.push_back(item.GetName());
		}
		item = finder.FindNext();
	}
	return list;
}
