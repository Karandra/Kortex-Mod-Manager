#include "stdafx.h"
#include "ModImporterNMM.h"
#include <Kortex/Application.hpp>
#include <Kortex/PluginManager.hpp>
#include <Kortex/SaveManager.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/DownloadManager.hpp>
#include "PackageProject/KPackageProjectSerializer.h"
#include "Utility/KOperationWithProgress.h"
#include "Utility/KAux.h"
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxTextFile.h>

namespace Kortex::ModManager
{
	wxString ModImporterNMM::ProcessDescription(const wxString& path) const
	{
		// Convert BB code
		return KPackageProjectSerializer::ConvertBBCode(path);
	}

	GameID ModImporterNMM::GetGameID(const wxString& name)
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
			if (name == "SkyrimSE")
			{
				return GameIDs::SkyrimSE;
			}
			if (name == "SkyrimVR")
			{
				return GameIDs::SkyrimVR;
			}

			// Fallout
			if (name == "Fallout3")
			{
				return GameIDs::Fallout3;
			}
			if (name == "FalloutNV")
			{
				return GameIDs::FalloutNV;
			}
			if (name == "Fallout4")
			{
				return GameIDs::Fallout4;
			}
			if (name == "Fallout4VR")
			{
				return GameIDs::Fallout4VR;
			}

			/*
			Other supported:

			BreakingWheel, DarkSouls, DarkSouls2, DragonAge, DragonAge2, DragonsDogma,
			Grimrock, NoMansSky, Starbound, StateOfDecay, TESO, WarThunder, Witcher2,
			Witcher3, WorldOfTanks, XCOM2, XRebirth.
			*/
		}
		return GameIDs::NullGameID;
	}
	void ModImporterNMM::LoadOptions()
	{
		wxString gameModeID;

		// Profiles list
		KxXMLNode node = m_ProfileManagerXML.QueryElement("profileManager/profileList");
		for (node = node.GetFirstChildElement("profile"); node.IsOK(); node = node.GetNextSiblingElement("profile"))
		{
			wxString id = node.GetAttribute("profileId");
			wxString name = node.GetAttribute("profileName");
			if (!name.IsEmpty() && !id.IsEmpty())
			{
				gameModeID = node.GetFirstChildElement("gameModeId").GetValue();
				m_ProfilesList.push_back(std::make_pair(id, name));
			}
		}

		// Game name
		m_TargetGameID = GetGameID(gameModeID);
		m_TargetGame = IGameInstance::GetTemplate(m_TargetGameID);
	}
	wxString ModImporterNMM::GetDataFolderName() const
	{
		if (m_TargetGameID == GameIDs::Morrowind)
		{
			return "Data Files";
		}
		else
		{
			return "Data";
		}
	}
	wxString ModImporterNMM::GetProfileDirectory() const
	{
		auto it = std::find_if(m_ProfilesList.begin(), m_ProfilesList.end(), [this](const auto& v)
		{
			return v.second == GetProfileToImport();
		});
		if (it != m_ProfilesList.end())
		{
			return m_InstanceDirectory + "\\ModProfiles\\" + it->first;
		}
		return wxEmptyString;
	}

	void ModImporterNMM::CopySavesAndConfig(KOperationWithProgressDialogBase* context)
	{
		context->SetDialogCaption(wxString::Format("%s \"%s\"", KTr("Generic.Import"), ISaveManager::GetInstance()->GetManagerInfo().GetName()));

		// Copy saves from real saves folder to virtual
		KxEvtFile savesSource(ITranslator::GetVariable(Variables::KVAR_ACTUAL_CONFIG_DIR));
		context->LinkHandler(&savesSource, KxEVT_FILEOP_COPY_FOLDER);
		savesSource.CopyFolder(KxFile::NullFilter, ITranslator::GetVariable(Variables::KVAR_CONFIG_DIR), true, true);
	}
	void ModImporterNMM::CopyMods(KOperationWithProgressDialogBase* context)
	{
		KxFileStream installConfigStream(m_InstanceDirectory + "\\VirtualInstall\\VirtualModConfig.xml", KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting);
		KxXMLDocument installConfig(installConfigStream);

		KxXMLNode modListNode = installConfig.QueryElement("virtualModActivator/modList");
		size_t modsProcessed = 0;
		size_t modsTotal = modListNode.GetChildrenCount();

		KxStringVector addedMods;
		for (KxXMLNode modInfoNode = modListNode.GetFirstChildElement("modInfo"); modInfoNode.IsOK(); modInfoNode = modInfoNode.GetNextSiblingElement("modInfo"))
		{
			int64_t modID = modInfoNode.GetAttributeInt("modId", -1);
			wxString modName = modInfoNode.GetAttribute("modName");
			wxString modFileName = modInfoNode.GetAttribute("modFileName");
			wxString modBaseFolder = modInfoNode.GetAttribute("modFilePath") + "\\VirtualInstall";

			// Notify
			context->SetDialogCaption(wxString::Format("%s \"%s\", %zu/%zu", KTr("Generic.Import"), modName, modsProcessed, modsTotal));

			// Check mod existence
			IGameMod* existingModEntry = IModManager::GetInstance()->FindModByID(modName);
			if (existingModEntry && ShouldSkipExistingMods())
			{
				existingModEntry->SetActive(true);
				continue;
			}

			// Write data
			IGameMod& modEntry = IModManager::GetInstance()->EmplaceMod();

			KxFileStream infoStream(m_InstanceDirectory + "\\cache\\" + modFileName.BeforeLast('.') + "\\Data\\fomod\\info.xml", KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting);
			KxXMLDocument info(infoStream);
			KxXMLNode infoNode = info.QueryElement("fomod");

			modEntry.SetName(modName);
			modEntry.SetActive(true);
			modEntry.SetVersion(infoNode.GetFirstChildElement("Version").GetValue());
			modEntry.SetAuthor(infoNode.GetFirstChildElement("Author").GetValue());
			modEntry.SetDescription(ProcessDescription(infoNode.GetFirstChildElement("Description").GetValue()));
			modEntry.GetProviderStore().AssignWith<NetworkManager::NexusProvider>(infoNode.GetFirstChildElement("Id").GetValueInt(modInfoNode.GetAttributeInt("modId", ModID::GetInvalidValue())));

			// Install date
			modEntry.SetInstallTime(KxFile(infoStream.GetFileName()).GetFileTime(KxFILETIME_CREATION));

			// If such mod already exist, try create unique ID
			if (existingModEntry)
			{
				modEntry.SetID(wxString::Format("[NMM %lld] %s", modID, modName));
			}
			else
			{
				modEntry.SetID(modName);
			}
			addedMods.push_back(modEntry.GetID());

			// Save entry
			modEntry.Save();

			// Copy mod contents
			// Do some trick:
			// NMM can store mod files in folder named after mod's name or its ID and there is no way to
			// know that. So I will check first file in the list, get folder from its path and construct final mod path.
			wxString modFolder = modBaseFolder + wxS('\\') + modInfoNode.GetFirstChildElement("fileLink").GetAttribute("realPath").BeforeFirst('\\');

			KxEvtFile source(modFolder);
			wxString destination = modEntry.GetModFilesDir() + wxS('\\') + GetDataFolderName();
			context->LinkHandler(&source, KxEVT_FILEOP_COPY_FOLDER);
			source.CopyFolder(KxFile::NullFilter, destination, true, true);
		}

		// Add mods to mods list
		IGameProfile* profile = IGameInstance::GetActive()->GetActiveProfile();

		GameInstance::ProfileMod::Vector& modList = profile->GetMods();
		modList.clear();
		for (const wxString& name: addedMods)
		{
			if (!context->CanContinue())
			{
				return;
			}

			if (IGameMod* existingMod = IModManager::GetInstance()->FindModByID(name))
			{
				modList.emplace_back(GameInstance::ProfileMod(*existingMod, existingMod->IsActive()));
			}
		}

		// Save lists, without sync
		profile->SaveConfig();
	}
	void ModImporterNMM::ReadPlugins(KOperationWithProgressDialogBase* context)
	{
		if (IPluginManager* pluginManager = IPluginManager::GetInstance())
		{
			context->SetDialogCaption(wxString::Format("%s \"%s\"", KTr("Generic.Import"), pluginManager->GetManagerInfo().GetName()));

			// Load entries
			pluginManager->Load();
			KxStringVector pluginsList = KxTextFile::ReadToArray(GetProfileDirectory() + '\\' + "LoadOrder.txt");

			IGameProfile* profile = IGameInstance::GetActive()->GetActiveProfile();

			GameInstance::ProfilePlugin::Vector& profilePluginsList = profile->GetPlugins();
			profilePluginsList.clear();
			for (const wxString& value: pluginsList)
			{
				if (!context->CanContinue())
				{
					return;
				}

				wxString enabledValue = value.AfterFirst('=');
				bool enabled = !enabledValue.IsEmpty() && enabledValue[0] == '1';

				profilePluginsList.emplace_back(GameInstance::ProfilePlugin(value.BeforeFirst('='), enabled));
			}
			profile->SaveConfig();
		}
	}
	void ModImporterNMM::CopyDownloads(KOperationWithProgressDialogBase* context)
	{
		IDownloadManager* manager = IDownloadManager::GetInstance();
		manager->PauseAllActive();

		KxFileFinder fileFinder(m_InstanceDirectory);
		KxFileItem fileItem = fileFinder.FindNext();
		while (fileItem.IsOK())
		{
			if (fileItem.IsNormalItem() && fileItem.IsFile())
			{
				KxEvtFile archiveFile(fileItem.GetFullPath());

				KxFileStream stream(m_InstanceDirectory + "\\cache\\" + fileItem.GetName().BeforeLast('.') + "\\Data\\fomod\\info.xml", KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting);
				KxXMLDocument info(stream);
				KxXMLNode infoNode = info.QueryElement("fomod");

				IDownloadEntry& entry = manager->NewDownload();
				entry.SetTargetGame(m_TargetGame);
				entry.SetProvider(NetworkManager::NexusProvider::GetInstance());
				entry.SetDate(archiveFile.GetFileTime(KxFileTime::KxFILETIME_CREATION));

				entry.GetDownloadInfo().SetURL(infoNode.GetFirstChildElement("Website").GetValue());

				entry.GetFileInfo().SetModID(infoNode.GetFirstChildElement("Id").GetValueInt(-1));
				entry.GetFileInfo().SetID(infoNode.GetFirstChildElement("DownloadId").GetValueInt(-1));
				entry.GetFileInfo().SetName(fileItem.GetName());
				entry.GetFileInfo().SetDisplayName(infoNode.GetFirstChildElement("Name").GetValue());
				entry.GetFileInfo().SetVersion(infoNode.GetFirstChildElement("Version").GetValue());

				int64_t size = archiveFile.GetFileSize();
				entry.GetFileInfo().SetSize(size);
				entry.SetDownloadedSize(size);

				entry.SetPaused(false);
				entry.SetHidden(false);

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
			fileItem = fileFinder.FindNext();
		}
	}

	void ModImporterNMM::SetDirectory(const wxString& path)
	{
		m_InstanceDirectory = path;
		if (!KxFileFinder::IsDirectoryEmpty(m_InstanceDirectory))
		{
			KxFileStream stream(m_InstanceDirectory + "\\ModProfiles\\ProfileManagerCfg.xml", KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting);
			m_ProfileManagerXML.Load(stream);
			if (m_ProfileManagerXML.IsOK())
			{
				LoadOptions();
				m_CanImport = true;
			}
		}
	}
	void ModImporterNMM::Import(KOperationWithProgressDialogBase* context)
	{
		if (context->CanContinue())
		{
			CopySavesAndConfig(context);
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

	bool ModImporterNMM::CanImport() const
	{
		return m_CanImport && m_TargetGame != nullptr;
	}
	wxString ModImporterNMM::GetAdditionalInfo() const
	{
		wxString additionalInfo;

		// Target profile
		if (m_TargetGame)
		{
			additionalInfo << "\r\n" << KTr("ModManager.Import.ManagedGame") << ": " << m_TargetGame->GetGameName();
		}
		return additionalInfo;
	}
	KxStringVector ModImporterNMM::GetProfilesList() const
	{
		KxStringVector profilesList;
		profilesList.reserve(m_ProfilesList.size());

		for (const auto&v: m_ProfilesList)
		{
			profilesList.push_back(v.second);
		}
		return profilesList;
	}
}
