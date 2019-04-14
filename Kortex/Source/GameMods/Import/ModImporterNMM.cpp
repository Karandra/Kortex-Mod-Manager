#include "stdafx.h"
#include "ModImporterNMM.h"
#include <Kortex/Application.hpp>
#include <Kortex/PluginManager.hpp>
#include <Kortex/SaveManager.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/DownloadManager.hpp>
#include <Kortex/GameConfig.hpp>
#include "Network/ModNetwork/Nexus.h"
#include "Network/ModNetwork/LoversLab.h"
#include "Network/ModNetwork/TESALL.h"
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
			return v.second == GetSelectedProfile();
		});
		if (it != m_ProfilesList.end())
		{
			return m_InstanceDirectory + "\\ModProfiles\\" + it->first;
		}
		return wxEmptyString;
	}

	void ModImporterNMM::CopySavesAndConfig(KOperationWithProgressDialogBase* context)
	{
		context->SetDialogCaption(KxString::Format("%1 \"%2\"", KTr("Generic.Import"), ISaveManager::GetInstance()->GetManagerInfo().GetName()));

		// Saves
		if (const ISaveManager* saveManager = ISaveManager::GetInstance())
		{
			KxEvtFile savesSource(ITranslator::GetVariable(Variables::KVAR_ACTUAL_SAVES_DIR));
			context->LinkHandler(&savesSource, KxEVT_FILEOP_COPY_FOLDER);
			savesSource.CopyFolder(KxFile::NullFilter, saveManager->GetConfig().GetLocation(), true, true);

			saveManager->ScheduleReloadWorkspace();
		}

		// Config
		if (IGameConfigManager* configManager = IGameConfigManager::GetInstance())
		{
			configManager->ForEachGroup([this](GameConfig::ItemGroup& group)
			{
				GameConfig::IFileSource* fileSource = nullptr;
				GameConfig::ISource& source = group.GetSource();
				if (source.QueryInterface(fileSource))
				{
					KxFile file(ITranslator::GetVariable(Variables::KVAR_ACTUAL_CONFIG_DIR) + '\\' + fileSource->GetExpandedFileName());
					if (file.IsFileExist())
					{
						source.Close();
						file.CopyFile(fileSource->GetResolvedFilePath(), true);
					}
				}
				return true;
			});
			configManager->Load();
			configManager->ScheduleReloadWorkspace();
		}
	}
	void ModImporterNMM::CopyMods(KOperationWithProgressDialogBase* context)
	{
		IGameProfile* profile = IGameInstance::GetActive()->GetActiveProfile();
		GameInstance::ProfileMod::Vector& currentModList = profile->GetMods();
		currentModList.clear();

		auto AddMod = [&currentModList](IGameMod& mod, bool isActive)
		{
			currentModList.emplace_back(mod, isActive);
		};

		KxFileStream installConfigStream(m_InstanceDirectory + "\\VirtualInstall\\VirtualModConfig.xml", KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting);
		KxXMLDocument installConfig(installConfigStream);

		KxXMLNode modListNode = installConfig.QueryElement("virtualModActivator/modList");
		size_t modsProcessed = 0;
		size_t modsTotal = modListNode.GetChildrenCount();

		for (KxXMLNode modInfoNode = modListNode.GetFirstChildElement("modInfo"); modInfoNode.IsOK(); modInfoNode = modInfoNode.GetNextSiblingElement("modInfo"))
		{
			ModID modID = modInfoNode.GetAttributeInt("modId", ModID::GetInvalidValue());
			wxString modName = modInfoNode.GetAttribute("modName");
			wxString modFileName = modInfoNode.GetAttribute("modFileName");
			wxString modBaseFolder = modInfoNode.GetAttribute("modFilePath") + "\\VirtualInstall";

			// Notify
			context->SetDialogCaption(KxString::Format("%1 \"%2\", %3/%4", KTr("Generic.Import"), modName, modsProcessed, modsTotal));

			// Check mod existence
			IGameMod* existingModEntry = IModManager::GetInstance()->FindModByID(modName);
			if (existingModEntry && ShouldSkipExistingMods())
			{
				AddMod(*existingModEntry, true);
				continue;
			}

			// Write data
			IGameMod& mod = IModManager::GetInstance()->EmplaceMod();

			KxFileStream infoStream(m_InstanceDirectory + "\\cache\\" + modFileName.BeforeLast('.') + "\\Data\\fomod\\info.xml", KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting);
			KxXMLDocument info(infoStream);
			KxXMLNode infoNode = info.QueryElement("fomod");

			mod.SetName(modName);
			mod.SetActive(true);
			mod.SetVersion(infoNode.GetFirstChildElement("Version").GetValue());
			mod.SetAuthor(infoNode.GetFirstChildElement("Author").GetValue());
			mod.SetDescription(ProcessDescription(infoNode.GetFirstChildElement("Description").GetValue()));

			// Get nexus mod ID from FOMod file if we don't have it already
			if (!modID.HasValue())
			{
				modID = infoNode.GetFirstChildElement("Id").GetValueInt(modID.GetValue());
			}
			mod.GetModSourceStore().AssignWith<NetworkManager::NexusModNetwork>(modID);

			// Install date
			mod.SetInstallTime(KxFile(infoStream.GetFileName()).GetFileTime(KxFILETIME_CREATION));

			// If such mod already exist, try create unique ID
			if (existingModEntry)
			{
				mod.SetID(KxString::Format("[NMM %1] %2", modID.GetValue(), modName));
			}
			else
			{
				mod.SetID(modName);
			}

			// Save entry
			AddMod(mod, true);
			mod.Save();

			// Copy mod contents.
			// NMM can stores mod files in folder named after mod's name or its ID and there is no way to know that.
			// So I will check first file in the list, get folder from its path and construct final mod path.
			wxString modFolder = modBaseFolder + wxS('\\') + modInfoNode.GetFirstChildElement("fileLink").GetAttribute("realPath").BeforeFirst('\\');

			KxEvtFile source(modFolder);
			wxString destination = mod.GetModFilesDir() + wxS('\\') + GetDataFolderName();
			context->LinkHandler(&source, KxEVT_FILEOP_COPY_FOLDER);
			source.CopyFolder(KxFile::NullFilter, destination, true, true);

			// Update mod content
			mod.UpdateFileTree();
		}

		profile->SaveConfig();
		IModDispatcher::GetInstance()->UpdateVirtualTree();
	}
	void ModImporterNMM::ReadPlugins(KOperationWithProgressDialogBase* context)
	{
		if (IPluginManager* pluginManager = IPluginManager::GetInstance())
		{
			context->SetDialogCaption(KxString::Format("%1 \"%2\"", KTr("Generic.Import"), pluginManager->GetManagerInfo().GetName()));

			IGameProfile* profile = IGameInstance::GetActive()->GetActiveProfile();
			GameInstance::ProfilePlugin::Vector& profilePluginsList = profile->GetPlugins();
			profilePluginsList.clear();

			for (const wxString& value: KxTextFile::ReadToArray(GetProfileDirectory() + '\\' + "LoadOrder.txt"))
			{
				if (!context->CanContinue())
				{
					return;
				}

				wxString enabledValue = value.AfterFirst('=');
				const bool isEnabled = !enabledValue.IsEmpty() && enabledValue[0] == '1';

				profilePluginsList.emplace_back(value.BeforeFirst('='), isEnabled);
			}
			profile->SaveConfig();

			pluginManager->Load();
			pluginManager->ScheduleReloadWorkspace();
		}
	}
	void ModImporterNMM::CopyDownloads(KOperationWithProgressDialogBase* context)
	{
		IDownloadManager* manager = IDownloadManager::GetInstance();
		manager->PauseAllActive();

		KxFileFinder fileFinder(m_InstanceDirectory);
		for (KxFileItem fileItem = fileFinder.FindNext(); fileItem.IsOK(); fileItem = fileFinder.FindNext())
		{
			if (fileItem.IsNormalItem() && fileItem.IsFile())
			{
				KxEvtFile archiveFile(fileItem.GetFullPath());

				KxFileStream stream(m_InstanceDirectory + "\\cache\\" + fileItem.GetName().BeforeLast('.') + "\\Data\\fomod\\info.xml", KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting);
				KxXMLDocument info(stream);
				KxXMLNode infoNode = info.QueryElement("fomod");

				IDownloadEntry& entry = manager->NewDownload();
				entry.SetTargetGame(m_TargetGame);
				entry.SetModNetwork(*NetworkManager::NexusModNetwork::GetInstance());
				entry.SetDate(archiveFile.GetFileTime(KxFileTime::KxFILETIME_CREATION));

				entry.GetDownloadInfo().URL = infoNode.GetFirstChildElement("Website").GetValue();

				entry.GetFileInfo().ModID = infoNode.GetFirstChildElement("Id").GetValueInt(-1);
				entry.GetFileInfo().ID = infoNode.GetFirstChildElement("DownloadId").GetValueInt(-1);
				entry.GetFileInfo().Name = fileItem.GetName();
				entry.GetFileInfo().DisplayName = infoNode.GetFirstChildElement("Name").GetValue();
				entry.GetFileInfo().Version = infoNode.GetFirstChildElement("Version").GetValue();

				const int64_t size = archiveFile.GetFileSize();
				entry.GetFileInfo().Size = size;
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
	KxStringVector ModImporterNMM::GetAvailableProfiles() const
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
