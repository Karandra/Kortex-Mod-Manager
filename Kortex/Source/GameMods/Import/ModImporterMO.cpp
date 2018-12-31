#include "stdafx.h"
#include "ModImporterMO.h"
#include <Kortex/Application.hpp>
#include <Kortex/PluginManager.hpp>
#include <Kortex/SaveManager.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/DownloadManager.hpp>
#include <Kortex/ProgramManager.hpp>
#include "PackageProject/KPackageProjectSerializer.h"
#include "Utility/KOperationWithProgress.h"
#include "Utility/KAux.h"
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxINI.h>

namespace Kortex::ModManager
{
	wxString& ModImporterMO::DecodeUTF8(wxString& string) const
	{
		// Find and replace all '\xABC' 5-char hex patterns to corresponding UTF-8 codes
		for (size_t i = 0; i < string.Length(); i++)
		{
			size_t pos = string.find(wxS("\\x"), i);
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
	wxString& ModImporterMO::ProcessFilePath(wxString& path) const
	{
		DecodeUTF8(path);

		path.Replace(wxS("/"), wxS("\\"));
		path.Replace(wxS("\\\\"), wxS("\\"));
		path.Replace(wxS("\\\""), wxS("\""));
		return path;
	}
	wxString& ModImporterMO::ProcessDescription(wxString& path) const
	{
		DecodeUTF8(path);

		// Fix line terminators
		path.Replace(wxS("\\r\\n"), wxS("\r\n"));
		path.Replace(wxS("\\r"), wxS("\r"));
		path.Replace(wxS("\\n"), wxS("\n"));

		// Escape mnemonics
		path.Replace(wxS("&#39;"), wxS("\'"));
		path.Replace(wxS("&#34;"), wxS("\""));
		path.Replace(wxS("&#38;"), wxS("&"));
		path.Replace(wxS("&60;"), wxS("<"));
		path.Replace(wxS("&62;"), wxS(">"));
		path.Replace(wxS("&33;"), wxS("!"));

		// Remove quotes from beginning and at the end
		if (!path.IsEmpty())
		{
			if (path[0] == wxS('\"'))
			{
				path.Remove(0, 1);
			}
		}
		if (!path.IsEmpty())
		{
			if (path.Last() == wxS('\"'))
			{
				path.RemoveLast(1);
			}
		}

		// Convert BB code
		path = KPackageProjectSerializer::ConvertBBCode(path);
		return path;
	}

	GameID ModImporterMO::GetGameID(const wxString& name)
	{
		if (!name.IsEmpty())
		{
			// TES
			if (name == "Morrowind")
			{
				return GameIDs::Morrowind;
			}
			if (name == "Oblivion")
			{
				return GameIDs::Oblivion;
			}
			if (name == "Skyrim")
			{
				return GameIDs::Skyrim;
			}
			if (name == "Skyrim Special Edition")
			{
				return GameIDs::SkyrimSE;
			}
			if (name == "Skyrim VR")
			{
				return GameIDs::SkyrimVR;
			}

			// Fallout
			if (name == "Fallout 3")
			{
				return GameIDs::Fallout3;
			}
			if (name == "New Vegas" || name == "TTW")
			{
				return GameIDs::FalloutNV;
			}
			if (name == "Fallout 4")
			{
				return GameIDs::Fallout4;
			}
			if (name == "Fallout 4 VR")
			{
				return GameIDs::Fallout4VR;
			}
		}
		return GameIDs::NullGameID;
	}
	void ModImporterMO::LoadOptions()
	{
		// Game name
		m_TargetProfile = GetGameID(m_Options.GetValue("General", "gameName"));
		m_TargetInstance = IGameInstance::GetTemplate(m_TargetProfile);
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
	wxString ModImporterMO::GetDataFolderName() const
	{
		if (m_TargetProfile == GameIDs::Morrowind)
		{
			return "Data Files";
		}
		else
		{
			return "Data";
		}
	}
	wxString ModImporterMO::GetProfileDirectory() const
	{
		return m_ProfilesDirectory + '\\' + GetProfileToImport();
	}

	void ModImporterMO::ReadExecutables(KOperationWithProgressDialogBase* context)
	{
		context->SetDialogCaption(wxString::Format("%s \"%s\"", KTr("Generic.Import"), IProgramManager::GetInstance()->GetManagerInfo().GetName()));

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

				IProgramEntry& entry = IProgramManager::GetInstance()->EmplaceProgram();
				entry.SetName(GetValue("title"));
				entry.SetExecutable(ProcessFilePath(GetValue("binary")));
				entry.SetArguments(ProcessFilePath(GetValue("arguments")));
				entry.SetWorkingDirectory(ProcessFilePath(GetValue("workingDirectory")));

				if (entry.GetName().IsEmpty())
				{
					entry.SetName(entry.GetExecutable().AfterLast('\\').BeforeLast('.'));
				}
			}
		}
	}
	void ModImporterMO::CopySaves(KOperationWithProgressDialogBase* context)
	{
		if (const ISaveManager* saveManager = ISaveManager::GetInstance())
		{
			context->SetDialogCaption(wxString::Format("%s \"%s\"", KTr("Generic.Import"), saveManager->GetManagerInfo().GetName()));

			KxEvtFile source(GetProfileDirectory() + "\\Saves");
			context->LinkHandler(&source, KxEVT_FILEOP_COPY_FOLDER);
			source.CopyFolder(KxFile::NullFilter, saveManager->GetConfig().GetLocation(), true, true);
		}
	}
	void ModImporterMO::CopyMods(KOperationWithProgressDialogBase* context)
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
					context->SetDialogCaption(wxString::Format("%s \"%s\", %zu/%zu", KTr("Generic.Import"), name, counter, modsList.size()));

					// Check mod existence
					IGameMod* existingMod = IModManager::GetInstance()->FindModByID(name);
					if (existingMod && ShouldSkipExistingMods())
					{
						existingMod->SetActive(isModEnabled);
						continue;
					}

					// Load 'meta.ini'
					wxString modFolder = GetModFolder(name);
					KxFileStream metaStream(modFolder + "\\Meta.ini");
					KxINI modINI(metaStream);

					// Write data
					IGameMod& entry = IModManager::GetInstance()->EmplaceMod();
					entry.SetName(name);
					entry.SetActive(isModEnabled);
					entry.SetVersion(modINI.GetValue("General", "version"));
					entry.SetPackageFile(ProcessFilePath(modINI.GetValue("General", "installationFile")));
					entry.SetDescription(ProcessDescription(modINI.GetValue("General", "nexusDescription")));

					// NexusID
					ModID nexusID = modINI.GetValueInt("General", "modid", ModID::GetInvalidValue());
					if (nexusID)
					{
						entry.GetProviderStore().AssignWith<NetworkManager::NexusProvider>(nexusID);
					}

					// Install date
					wxDateTime date = KxFile(modFolder).GetFileTime(KxFILETIME_CREATION);
					if (isModEnabled)
					{
						entry.SetInstallTime(date);
					}
					else
					{
						entry.SetUninstallTime(date);
					}

					// If such mod already exist, try create unique ID
					if (existingMod)
					{
						entry.SetID(wxString::Format("[MO] %s", name));
					}
					else
					{
						entry.SetID(name);
					}

					// Save
					entry.Save();

					// Copy mod contents
					wxString destination = entry.GetModFilesDir() + wxS('\\') + GetDataFolderName();

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
		IGameProfile* profile = IGameInstance::GetActiveProfile();

		GameInstance::ProfileMod::Vector& currentModList = profile->GetMods();
		currentModList.clear();
		for (const wxString& name: modsList)
		{
			if (!context->CanContinue())
			{
				return;
			}

			if (IGameMod* existingMod = IModManager::GetInstance()->FindModByID(name))
			{
				currentModList.emplace_back(GameInstance::ProfileMod(*existingMod, existingMod->IsActive()));
			}
		}
		profile->SaveConfig();
	}
	void ModImporterMO::ReadPlugins(KOperationWithProgressDialogBase* context)
	{
		if (IPluginManager* manager = IPluginManager::GetInstance())
		{
			context->SetDialogCaption(wxString::Format("%s \"%s\"", KTr("Generic.Import"), manager->GetManagerInfo().GetName()));

			// Load entries
			manager->Load();

			KxStringVector activePlugins = KxTextFile::ReadToArray(GetProfileDirectory() + "\\Plugins.txt");

			IGameProfile* profile = IGameInstance::GetActiveProfile();

			GameInstance::ProfilePlugin::Vector& currentPluginsList = profile->GetPlugins();
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
				currentPluginsList.emplace_back(GameInstance::ProfilePlugin(name, KAux::IsStringsContain(activePlugins, name, false)));
			}
			profile->SaveConfig();
		}
	}
	void ModImporterMO::CopyGameConfig(KOperationWithProgressDialogBase* context)
	{
		#if 0
		if (const KConfigManagerConfig* options = KConfigManagerConfig::GetInstance())
		{
			context->SetDialogCaption(wxString::Format("%s \"%s\"", KTr("Generic.Import"), KGameConfigWorkspace::GetInstance()->GetName()));

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
		#endif
	}
	void ModImporterMO::CopyDownloads(KOperationWithProgressDialogBase* context)
	{
		IDownloadManager* manager = IDownloadManager::GetInstance();
		manager->PauseAllActive();

		KxFileFinder finder(m_DownloadsDirectory, "*.meta");
		KxFileItem item = finder.FindNext();
		while (item.IsOK())
		{
			KxEvtFile archiveFile(item.GetFullPath().BeforeLast('.'));
			if (item.IsNormalItem() && item.IsFile())
			{
				KxFileStream stream(item.GetFullPath(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting);
				KxINI ini(stream);

				IDownloadEntry& entry = manager->NewDownload();
				entry.SetTargetGameID(ini.GetValue("General", "gameName"));
				entry.SetProvider(INetworkManager::GetInstance()->FindProvider(ini.GetValue("General", "repository")));
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
					entry.Save();

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

	void ModImporterMO::SetDirectory(const wxString& path)
	{
		m_InstanceDirectory = path;
		if (!KxFileFinder::IsDirectoryEmpty(m_InstanceDirectory))
		{
			KxFileStream stream(m_InstanceDirectory + '\\' + "ModOrganizer.ini", KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting);
			m_Options.Load(stream);
			if (m_Options.IsOK())
			{
				LoadOptions();
				m_CanImport = true;
			}
		}
	}
	void ModImporterMO::Import(KOperationWithProgressDialogBase* context)
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

	bool ModImporterMO::CanImport() const
	{
		if (m_CanImport)
		{
			// Allow import from unknown managed game, as MO1 doesn't store its name.
			if (m_TargetProfile.IsOK())
			{
				return m_TargetInstance != nullptr;
			}
			return true;
		}
		return false;
	}
	wxString ModImporterMO::GetAdditionalInfo() const
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
		info << KTr("Generic.Version") << ": " << version;

		// Target profile
		if (m_TargetInstance)
		{
			info << "\r\n" << KTr("ModManager.Import.ManagedGame") << ": " << m_TargetInstance->GetName();
		}

		return info;
	}
	KxStringVector ModImporterMO::GetProfilesList() const
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
}
