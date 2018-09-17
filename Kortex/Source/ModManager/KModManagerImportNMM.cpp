#include "stdafx.h"
#include "KModManagerImportNMM.h"
#include "SaveManager/KSaveManager.h"
#include "PluginManager/KPluginManager.h"
#include "PackageProject/KPackageProjectSerializer.h"
#include "Network/KNetwork.h"
#include "Network/KNetworkProviderNexus.h"
#include "DownloadManager/KDownloadManager.h"
#include "Profile/KProfile.h"
#include "Profile/KConfigManagerConfig.h"
#include "KOperationWithProgress.h"
#include "KAux.h"
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxTextFile.h>

wxString KModManagerImportNMM::ProcessDescription(const wxString& path) const
{
	// Convert BB code
	return KPackageProjectSerializer::ConvertBBCode(path);
}

KProfileID KModManagerImportNMM::GetGameID(const wxString& name)
{
	if (!name.IsEmpty())
	{
		// TES
		if (name == "Morrowind")
		{
			return KProfileIDs::Morrowind;
		}
		if (name == "Oblivion")
		{
			return KProfileIDs::Oblivion;
		}
		if (name == "Skyrim")
		{
			return KProfileIDs::Skyrim;
		}
		if (name == "SkyrimSE")
		{
			return KProfileIDs::SkyrimSE;
		}
		if (name == "SkyrimVR")
		{
			return KProfileIDs::SkyrimVR;
		}

		// Fallout
		if (name == "Fallout3")
		{
			return KProfileIDs::Fallout3;
		}
		if (name == "FalloutNV")
		{
			return KProfileIDs::FalloutNV;
		}
		if (name == "Fallout4")
		{
			return KProfileIDs::Fallout4;
		}
		if (name == "Fallout4VR")
		{
			return KProfileIDs::Fallout4VR;
		}

		/* 
		Other supported:

		BreakingWheel, DarkSouls, DarkSouls2, DragonAge, DragonAge2, DragonsDogma,
		Grimrock, NoMansSky, Starbound, StateOfDecay, TESO, WarThunder, Witcher2,
		Witcher3, WorldOfTanks, XCOM2, XRebirth.
		*/
	}
	return KProfileIDs::NullProfileID;
}
void KModManagerImportNMM::LoadOptions()
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
	m_TargetProfile = GetGameID(gameModeID);
	m_TargetProfileTemplate = KProfile::GetProfileTemplate(m_TargetProfile);
}
wxString KModManagerImportNMM::GetDataFolderName() const
{
	if (m_TargetProfile == KProfileIDs::Morrowind)
	{
		return "Data Files";
	}
	else
	{
		return "Data";
	}
}
wxString KModManagerImportNMM::GetProfileDirectory() const
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

void KModManagerImportNMM::CopySavesAndConfig(KOperationWithProgressDialogBase* context)
{
	context->SetDialogCaption(wxString::Format("%s \"%s\"", T("Generic.Import"), KSaveManager::GetInstance()->GetName()));

	// Copy saves from real saves folder to virtual
	KxEvtFile savesSource(KVAR_EXP(KVAR_CONFIG_ROOT));
	context->LinkHandler(&savesSource, KxEVT_FILEOP_COPY_FOLDER);
	savesSource.CopyFolder(KxFile::NullFilter, KVAR_EXP(KVAR_VIRTUAL_CONFIG_ROOT), true, true);
}
void KModManagerImportNMM::CopyMods(KOperationWithProgressDialogBase* context)
{
	KxFileStream installConfigStream(m_InstanceDirectory + "\\VirtualInstall\\VirtualModConfig.xml", KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING);
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
		context->SetDialogCaption(wxString::Format("%s \"%s\", %zu/%zu", T("Generic.Import"), modName, modsProcessed, modsTotal));

		// Check mod existence
		KModEntry* existingModEntry = KModManager::Get().FindModByID(modName);
		if (existingModEntry && ShouldSkipExistingMods())
		{
			existingModEntry->SetEnabled(true);
			continue;
		}

		// Write data
		KModEntry* modEntry = KModManager::Get().GetEntries().emplace_back(new KModEntry());

		KxFileStream infoStream(m_InstanceDirectory + "\\cache\\" + modFileName.BeforeLast('.') + "\\Data\\fomod\\info.xml", KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING);
		KxXMLDocument info(infoStream);
		KxXMLNode infoNode = info.QueryElement("fomod");

		modEntry->SetName(modName);
		modEntry->SetEnabled(true);
		modEntry->SetVersion(infoNode.GetFirstChildElement("Version").GetValue());
		modEntry->SetAuthor(infoNode.GetFirstChildElement("Author").GetValue());
		modEntry->SetDescription(ProcessDescription(infoNode.GetFirstChildElement("Description").GetValue()));
		modEntry->SetWebSite(KNETWORK_PROVIDER_ID_NEXUS, infoNode.GetFirstChildElement("Id").GetValueInt(modInfoNode.GetAttributeInt("modId")));

		// Install date
		modEntry->SetTime(KME_TIME_INSTALL, KxFile(infoStream.GetFileName()).GetFileTime(KxFILETIME_CREATION));

		// If such mod already exist, try create unique ID
		if (existingModEntry)
		{
			modEntry->SetID(wxString::Format("[NMM %lld] %s", modID, modName));
		}
		else
		{
			modEntry->SetID(modName);
		}
		addedMods.push_back(modEntry->GetID());

		// Save entry
		modEntry->CreateAllFolders();
		modEntry->Save();

		// Copy mod contents
		// Do some trick:
		// NMM can store mod files in folder named after mod's name or its ID and there is no way to
		// know that. So I will check first file in the list, get folder from its path and construct final mod path.
		wxString modFolder = modBaseFolder + '\\' + modInfoNode.GetFirstChildElement("fileLink").GetAttribute("realPath").BeforeFirst('\\');
		
		KxEvtFile source(modFolder);
		wxString destination = modEntry->GetLocation(KMM_LOCATION_MOD_FILES) + '\\' + GetDataFolderName();
		context->LinkHandler(&source, KxEVT_FILEOP_COPY_FOLDER);
		source.CopyFolder(KxFile::NullFilter, destination, true, true);
	}

	// Add mods to mods list
	KModList::ModEntryVector& tCurrentModList = KModManager::GetListManager().GetCurrentList().GetMods();
	tCurrentModList.clear();
	for (const wxString& name: addedMods)
	{
		if (!context->CanContinue())
		{
			return;
		}

		if (KModEntry* existingMod = KModManager::Get().FindModByID(name))
		{
			tCurrentModList.emplace_back(KModListModEntry(existingMod, existingMod->IsEnabled()));
		}
	}

	// Save lists, without sync
	KModManager::GetListManager().SaveLists();
}
void KModManagerImportNMM::ReadPlugins(KOperationWithProgressDialogBase* context)
{
	if (KPluginManager* pluginManager = KPluginManager::GetInstance())
	{
		context->SetDialogCaption(wxString::Format("%s \"%s\"", T("Generic.Import"), pluginManager->GetName()));

		// Load entries
		pluginManager->Load();
		KxStringVector pluginsList = KxTextFile::ReadToArray(GetProfileDirectory() + '\\' + "LoadOrder.txt");

		KModList::PluginEntryVector& currentPluginsList = KModManager::GetListManager().GetCurrentList().GetPlugins();
		currentPluginsList.clear();
		for (const wxString& value: pluginsList)
		{
			if (!context->CanContinue())
			{
				return;
			}

			wxString enabledValue = value.AfterFirst('=');
			bool enabled = !enabledValue.IsEmpty() && enabledValue[0] == '1';

			currentPluginsList.emplace_back(KModListPluginEntry(value.BeforeFirst('='), enabled));
		}
		KModManager::GetListManager().SaveLists();
	}
}
void KModManagerImportNMM::CopyDownloads(KOperationWithProgressDialogBase* context)
{
	KDownloadManager* manager = KDownloadManager::GetInstance();
	manager->PauseAllActive();

	KxFileFinder fileFinder(m_InstanceDirectory);
	KxFileFinderItem fileItem = fileFinder.FindNext();
	while (fileItem.IsOK())
	{
		if (fileItem.IsNormalItem() && fileItem.IsFile())
		{
			KxEvtFile archiveFile(fileItem.GetFullPath());

			KxFileStream stream(m_InstanceDirectory + "\\cache\\" + fileItem.GetName().BeforeLast('.') + "\\Data\\fomod\\info.xml", KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING);
			KxXMLDocument info(stream);
			KxXMLNode infoNode = info.QueryElement("fomod");

			KDownloadEntry& entry = *manager->GetDownloads().emplace_back(new KDownloadEntry());
			entry.SetTargetProfile(m_TargetProfileTemplate);
			entry.SetProvider(KNetworkProviderNexus::GetInstance());
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

			entry.SetInstalled(false);
			entry.SetPaused(false);
			entry.SetHidden(false);

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
		fileItem = fileFinder.FindNext();
	}
}

void KModManagerImportNMM::SetDirectory(const wxString& path)
{
	m_InstanceDirectory = path;
	if (!KxFileFinder::IsDirectoryEmpty(m_InstanceDirectory))
	{
		KxFileStream stream(m_InstanceDirectory + "\\ModProfiles\\ProfileManagerCfg.xml", KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING);
		m_ProfileManagerXML.Load(stream);
		if (m_ProfileManagerXML.IsOK())
		{
			LoadOptions();
			m_CanImport = true;
		}
	}
}
void KModManagerImportNMM::Import(KOperationWithProgressDialogBase* context)
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

bool KModManagerImportNMM::CanImport() const
{
	return m_CanImport && m_TargetProfileTemplate != NULL;
}
wxString KModManagerImportNMM::GetAdditionalInfo() const
{
	wxString additionalInfo;

	// Target profile
	if (m_TargetProfileTemplate)
	{
		additionalInfo << "\r\n" << T("ModManager.Import.ManagedGame") << ": " << m_TargetProfileTemplate->GetName();
	}
	return additionalInfo;
}
KxStringVector KModManagerImportNMM::GetProfilesList() const
{
	KxStringVector profilesList;
	profilesList.reserve(m_ProfilesList.size());

	for (const auto&v: m_ProfilesList)
	{
		profilesList.push_back(v.second);
	}
	return profilesList;
}
