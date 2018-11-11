#include "stdafx.h"
#include "KVariablesDatabase.h"
#include "KModManager.h"
#include "KModWorkspace.h"
#include "KModManagerModel.h"
#include "KModEntry.h"
#include "KVirtualGameFolderWorkspace.h"
#include "GameInstance/KInstanceManagement.h"
#include "GameInstance/Config/KVirtualizationConfig.h"
#include "VFS/KVFSService.h"
#include "VFS/KVFSConvergence.h"
#include "VFS/KVFSMirror.h"
#include "Events/KVFSEvent.h"
#include "Events/KLogEvent.h"
#include "UI/KMainWindow.h"
#include "UI/KWorkspace.h"
#include "UI/KWorkspaceController.h"
#include "PluginManager/KPluginManager.h"
#include "PluginManager/KPluginManagerWorkspace.h"
#include "IPC/KIPCClient.h"
#include "Network/KNetwork.h"
#include "KOperationWithProgress.h"
#include "KUPtrVectorUtil.h"
#include "KApp.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxProgressDialog.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxString.h>
#include <execution>

void KModManager::DoResortMods(const KProfile& profile)
{
	size_t modIndex = 0;
	for (const KProfileMod& listEntry: profile.GetMods())
	{
		if (modIndex < m_ModEntries.size())
		{
			intptr_t currentElement = -1;
			FindModBySignature(listEntry.GetSignature(), &currentElement);

			if (currentElement != -1)
			{
				m_ModEntries[currentElement]->SetEnabled(listEntry.IsEnabled());
				std::swap(m_ModEntries[currentElement], m_ModEntries[modIndex]);
			}
			modIndex++;
		}
	}

	if (KDispatcher::HasInstance())
	{
		KDispatcher::GetInstance()->InvalidateVirtualTree();
	}
}
void KModManager::DoUninstallMod(KModEntry* modEntry, bool erase, wxWindow* window)
{
	KModEvent event(KEVT_MOD_UNINSTALLING, *modEntry);
	event.Send();

	// If signature is empty, removing this mod can cause removing ALL other mods
	// because mod folder path will point to all mods directory instead of its own.
	// Just ignore this. User can always delete this folder manually.
	if (event.IsAllowed() && !modEntry->GetSignature().IsEmpty())
	{
		// Disable it
		modEntry->SetEnabled(false);

		if (!erase)
		{
			modEntry->SetTime(KME_TIME_UNINSTALL, wxDateTime::Now());
			modEntry->Save();
		}
		wxString path = erase ? modEntry->GetRootDir() : modEntry->GetModFilesDir();

		KOperationWithProgressDialogBase* operation = new KOperationWithProgressDialogBase(true, window);	
		operation->OnRun([path = path.Clone(), modEntry](KOperationWithProgressBase* self)
		{
			KxEvtFile folder(path);
			self->LinkHandler(&folder, KxEVT_FILEOP_REMOVE_FOLDER);
			folder.RemoveFolderTree(true);
		});
		operation->OnEnd([this, modEntry, erase](KOperationWithProgressBase* self)
		{
			Save();
			if (erase)
			{
				NotifyModErased(*modEntry);
			}
			else
			{
				NotifyModUninstalled(*modEntry);
			}
		});
		operation->SetDialogCaption(KTr("ModManager.RemoveMod.RemovingMessage"));
		operation->Run();
	}
}
KModEntry* KModManager::DoLoadMod(const wxString& signature)
{
	if (!signature.IsEmpty())
	{
		KModEntry& entry = *m_ModEntries.emplace_back(new KModEntry());
		entry.CreateFromSignature(signature);
		if (entry.IsOK())
		{
			m_TagManager.LoadTagsFromEntry(entry);
			return &entry;
		}
		m_ModEntries.pop_back();
	}
	return NULL;
}

bool KModManager::InitMainVirtualFolder()
{
	wxString mountPoint = GetVirtualGameRoot();
	if (CheckMountPoint(mountPoint))
	{
		KxStringVector folders;
		for (const KModEntry* entry: GetAllEntries())
		{
			if (entry->IsEnabled())
			{
				folders.push_back(entry->GetModFilesDir());
			}
		}

		if (KIPCClient::GetInstance()->CreateVFS_Convergence(mountPoint, m_ModEntry_WriteTarget.GetModFilesDir(), folders, true))
		{
			return KIPCClient::GetInstance()->ConvergenceVFS_SetDispatcherIndex();
		}
	}
	return false;
}
bool KModManager::InitMirroredLocations()
{
	KIPCClient::GetInstance()->MirrorVFS_ClearList();

	const KVirtualizationMirroredEntry::Vector& locations = KVirtualizationConfig::GetInstance()->GetMirroredLocations();

	// Check folders first
	for (const KVirtualizationMirroredEntry& entry: locations)
	{
		if (!CheckMountPoint(entry.GetTarget()))
		{
			return false;
		}
	}

	// Initialize them
	for (const KVirtualizationMirroredEntry& entry: locations)
	{
		if (entry.ShouldUseMultiMirror())
		{
			KIPCClient::GetInstance()->CreateVFS_MultiMirror(entry.GetSources(), entry.GetTarget());
		}
		else
		{
			KIPCClient::GetInstance()->CreateVFS_Mirror(entry.GetSource(), entry.GetTarget());
		}
	}
	return true;
}

void KModManager::CreateMountStatusDialog()
{
	if (KMainWindow::HasInstance())
	{
		KMainWindow::GetInstance()->Disable();
	}

	m_MountStatusDialog = new KxProgressDialog(KMainWindow::GetInstance(), KxID_NONE, wxEmptyString, wxDefaultPosition, wxDefaultSize, KxBTN_NONE);
	m_MountStatusDialog->SetCaption(IsVFSMounted() ? KTr("VFS.MountingCaptionDisable") : KTr("VFS.MountingCaptionEnable"));
	m_MountStatusDialog->SetLabel(KTr("VFS.MountingMessage"));
	m_MountStatusDialog->Pulse();
	m_MountStatusDialog->Show();
}
void KModManager::DestroyMountStatusDialog()
{
	if (KMainWindow::HasInstance())
	{
		KMainWindow::GetInstance()->Enable();
	}

	if (m_MountStatusDialog)
	{
		m_MountStatusDialog->Destroy();
		m_MountStatusDialog = NULL;
	}
}

bool KModManager::CheckMountPoint(const wxString& folderPath)
{
	if (!KxFileFinder::IsDirectoryEmpty(folderPath))
	{
		ReportNonEmptyMountPoint(folderPath);
		return false;
	}
	return true;
}
void KModManager::ReportNonEmptyMountPoint(const wxString& folderPath)
{
	KxTaskDialog dialog(KMainWindow::GetInstance(), KxID_NONE, KTr("VFS.MountPointNotEmpty.Caption"), KTr("VFS.MountPointNotEmpty.Message"), KxBTN_OK, KxICON_ERROR);
	dialog.SetOptionEnabled(KxTD_HYPERLINKS_ENABLED);
	dialog.SetOptionEnabled(KxTD_EXMESSAGE_EXPANDED);
	dialog.SetExMessage(wxString::Format("<a href=\"%s\">%s</a>", folderPath, folderPath));

	dialog.Bind(wxEVT_TEXT_URL, [&dialog](wxTextUrlEvent& event)
	{
		KxShell::Execute(&dialog, event.GetString(), "open");
	});
	dialog.ShowModal();
}

void KModManager::OnInit()
{
	// Mandatory locations
	for (const KVirtualizationMandatoryEntry& mandatoryEntry: KVirtualizationConfig::GetInstance()->GetMandatoryLocations())
	{
		int orderIndex = m_ModEntry_BaseGame.GetOrderIndex() + m_ModEntry_Mandatory.size() + 1;
		KFixedModEntry& entry = m_ModEntry_Mandatory.emplace_back(orderIndex);

		entry.SetID(mandatoryEntry.GetName());
		entry.SetEnabled(true);
		entry.SetLinkedModLocation(mandatoryEntry.GetSource());
	}

	// Base game
	m_ModEntry_BaseGame.SetID(KVAR_GAME_ID);
	m_ModEntry_BaseGame.SetName(KVAR_EXP(KVAR_GAME_NAME));
	m_ModEntry_BaseGame.SetEnabled(true);
	m_ModEntry_BaseGame.SetLinkedModLocation(KVAR_EXP(KVAR_ACTUAL_GAME_DIR));

	// Write target
	m_ModEntry_WriteTarget.SetID(KVAR_OVERWRITES_DIR);
	m_ModEntry_WriteTarget.SetName(KTr("ModManager.WriteTargetName"));
	m_ModEntry_WriteTarget.SetEnabled(true);

	m_TagManager.OnInit();
	Load();
}

void KModManager::OnModFilesChanged(KModEvent& event)
{
	if (event.HasMod())
	{
		event.GetMod()->UpdateFileTree();
	}
	for (KModEntry* mod: event.GetModsArray())
	{
		mod->UpdateFileTree();
	}
}

KModManager::KModManager(KWorkspace* workspace)
	:m_Options(this, "General"),
	m_ModEntry_BaseGame(std::numeric_limits<int>::min()), m_ModEntry_WriteTarget(std::numeric_limits<int>::max())
{
	KEvent::Bind(KEVT_MOD_FILES_CHANGED, &KModManager::OnModFilesChanged, this);
}
void KModManager::Clear()
{
	m_ModEntries.clear();
}
KModManager::~KModManager()
{
	DestroyMountStatusDialog();
	Clear();
}

wxString KModManager::GetID() const
{
	return "KModManager";
}
wxString KModManager::GetName() const
{
	return KTr("ModManager.Name");
}
wxString KModManager::GetVersion() const
{
	return "1.3";
}
KWorkspace* KModManager::GetWorkspace() const
{
	return KModWorkspace::GetInstance();
}

KModEntry::RefVector KModManager::GetAllEntries(bool includeWriteTarget)
{
	KModEntry::RefVector entries;
	entries.reserve(m_ModEntries.size() + m_ModEntry_Mandatory.size() + 2);

	// Add game root as first virtual folder
	entries.push_back(&m_ModEntry_BaseGame);

	// Add mandatory virtual folders
	for (KMandatoryModEntry& entry: m_ModEntry_Mandatory)
	{
		entries.push_back(&entry);
	}

	// Add mods
	for (auto& entry: m_ModEntries)
	{
		entries.push_back(&*entry);
	}

	// Add write target
	if (includeWriteTarget)
	{
		entries.push_back(&m_ModEntry_WriteTarget);
	}

	return entries;
}

void KModManager::Load()
{
	Clear();

	// Load entries
	if (KActiveGameInstance* instnace = KGameInstance::GetActive())
	{
		for (const wxString& path: KxFile(instnace->GetModsDir()).Find(KxFile::NullFilter, KxFS_FOLDER, false))
		{
			wxString signature = KxFile(path).GetFullName();
			DoLoadMod(signature);
		}
	}

	// Build mod file trees
	KModEntry::RefVector allEntries = GetAllEntries(true);
	std::for_each(std::execution::par_unseq, allEntries.begin(), allEntries.end(), [](KModEntry* entry)
	{
		entry->UpdateFileTree();
	});

	if (KDispatcher::HasInstance())
	{
		KDispatcher::GetInstance()->InvalidateVirtualTree();
	}
}
void KModManager::Save() const
{
	KProfile* profile = KGameInstance::GetActiveProfile();
	if (profile)
	{
		profile->SyncWithCurrentState();
		profile->Save();
	}
}

void KModManager::ResortMods()
{
	ResortMods(*KGameInstance::GetActiveProfile());
}
void KModManager::ResortMods(const KProfile& profile)
{
	DoResortMods(profile);

	// Causes rebuild of virtual file tree
	KModEvent(KEVT_MOD_FILES_CHANGED).Send();
}

KModEntry* KModManager::FindModByID(const wxString& modID, intptr_t* index) const
{
	intptr_t i = 0;
	for (auto& entry: m_ModEntries)
	{
		if (entry->GetID() == modID)
		{
			KxUtility::SetIfNotNull(index, i);
			return &*entry;
		}
		i++;
	}

	KxUtility::SetIfNotNull(index, -1);
	return NULL;
}
KModEntry* KModManager::FindModByName(const wxString& modName, intptr_t* index) const
{
	intptr_t i = 0;
	for (auto& entry: m_ModEntries)
	{
		if (entry->GetName() == modName)
		{
			KxUtility::SetIfNotNull(index, i);
			return &*entry;
		}
		i++;
	}

	KxUtility::SetIfNotNull(index, -1);
	return NULL;
}
KModEntry* KModManager::FindModBySignature(const wxString& signature, intptr_t* index) const
{
	intptr_t i = 0;
	for (auto& entry: m_ModEntries)
	{
		if (entry->GetSignature() == signature)
		{
			KxUtility::SetIfNotNull(index, i);
			return &*entry;
		}
		i++;
	}

	KxUtility::SetIfNotNull(index, -1);
	return NULL;
}
KModEntry* KModManager::FindModByNetworkModID(KNetworkProviderID providerID, KNetworkModID id, intptr_t* index) const
{
	if (KNetwork::GetInstance()->IsValidProviderID(providerID))
	{
		intptr_t i = 0;
		for (auto& entry: m_ModEntries)
		{
			if (entry->GetFixedWebSites()[providerID] == id)
			{
				KxUtility::SetIfNotNull(index, i);
				return &*entry;
			}
			i++;
		}
	}

	KxUtility::SetIfNotNull(index, -1);
	return NULL;
}

bool KModManager::IsModActive(const wxString& modID) const
{
	const KModEntry* entry = FindModByID(modID);
	if (entry)
	{
		return entry->IsEnabled();
	}
	return false;
}
bool KModManager::ChangeModID(KModEntry* entry, const wxString& newID)
{
	if (FindModByID(newID) == NULL)
	{
		wxString oldPath = entry->GetRootDir();
		KModEntry tempEntry;
		tempEntry.SetID(newID);

		if (KxFile(oldPath).Rename(tempEntry.GetRootDir(), false))
		{
			entry->SetID(newID);
			entry->Save();

			// Save new mod order with changed signature.
			// Reloading manager data is not needed
			Save();

			// This will take care of file tree
			KModEvent(KEVT_MOD_FILES_CHANGED, *entry).Send();
			return true;
		}
	}
	return false;
}

intptr_t KModManager::GetModOrderIndex(const KModEntry* modEntry) const
{
	auto it = std::find_if(m_ModEntries.begin(), m_ModEntries.end(), [modEntry](const auto& mod)
	{
		return mod.get() == modEntry;
	});
	if (it != m_ModEntries.end())
	{
		return std::distance(m_ModEntries.begin(), it);
	}
	return -1;
}
bool KModManager::MoveModsIntoThis(const KModEntry::RefVector& entriesToMove, const KModEntry& anchor, MoveMode moveMode)
{
	if (moveMode == MoveMode::Before)
	{
		return KUPtrVectorUtil::MoveBefore(m_ModEntries, entriesToMove, anchor);
	}
	else
	{
		return KUPtrVectorUtil::MoveAfter(m_ModEntries, entriesToMove, anchor);
	}
}

wxString KModManager::GetVirtualGameRoot() const
{
	return KVarExp(KVAR(KVAR_VIRTUAL_GAME_DIR));
}

void KModManager::MountVFS()
{
	CreateMountStatusDialog();
	KApp::Get().CallAfter([this]()
	{
		// Init and mount
		bool ok = InitMainVirtualFolder() && InitMirroredLocations();
		if (ok)
		{
			KIPCClient::GetInstance()->ToggleVFS();
		}
		else
		{
			DestroyMountStatusDialog();
		}
	});
}
void KModManager::UnMountVFS()
{
	CreateMountStatusDialog();
	KApp::Get().CallAfter([]()
	{
		KIPCClient::GetInstance()->DisableVFS();
	});
}

void KModManager::ExportModList(const wxString& outputFilePath) const
{
	KxXMLDocument xml;
	KxXMLNode bodyNode = xml.NewElement("html").NewElement("body");
	KxXMLNode tableNode = bodyNode.NewElement("table");
	tableNode.SetAttribute("frame", "border");
	tableNode.SetAttribute("rules", "all");

	auto AddRow = [&tableNode](const std::initializer_list<wxString>& list, bool isHeader = false)
	{
		KxXMLNode rowNode = tableNode.NewElement("tr");
		rowNode.SetAttribute("align", "left");

		for (const wxString& s: list)
		{
			rowNode.NewElement(isHeader ? "th" : "td").SetValue(s);
		}
		return rowNode;
	};
	auto AddCheckBox = [](KxXMLNode& node, bool value, const wxString& altText = wxEmptyString)
	{
		node.SetAttribute("align", "center");

		KxXMLNode checkBoxNode = node.NewElement("input", KxXML_INSERT_AS_FIRST_CHILD);
		checkBoxNode.SetAttribute("type", "checkbox");
		checkBoxNode.SetAttribute("readonly", "true");
		checkBoxNode.SetAttribute("disabled", "true");

		if (value)
		{
			checkBoxNode.SetAttribute("checked", "true");
		}

		if (!altText.IsEmpty())
		{
			checkBoxNode.SetAttribute("alt", altText);
		}

		return checkBoxNode;
	};

	AddRow({"Installed", "Active", "Name (ID)", "Version", "Author", "Sites", "Description"}, true);
	for (const auto& modEntry: m_ModEntries)
	{
		wxString name;
		if (modEntry->GetName() != modEntry->GetID())
		{
			name = wxString::Format("%s (%s)", modEntry->GetName(), modEntry->GetID());
		}
		else
		{
			name = modEntry->GetName();
		}

		wxString version = modEntry->GetVersion().IsOK() ? modEntry->GetVersion().ToString() : wxEmptyString;
		wxString author = !modEntry->GetAuthor().IsEmpty() ? modEntry->GetAuthor() : wxEmptyString;

		KxXMLNode rowNode = AddRow({wxEmptyString, wxEmptyString, name, version, author});

		// Prepend state
		AddCheckBox(rowNode.GetFirstChildElement(), modEntry->IsInstalled(), "Is installed");
		AddCheckBox(rowNode.GetFirstChildElement().GetNextSiblingElement(), modEntry->IsEnabled(), "Is active");

		// Add sites
		KxXMLNode sitesNode = rowNode.NewElement("td");
		for (int i = 0; i < KNETWORK_PROVIDER_ID_MAX; i++)
		{
			if (modEntry->HasWebSite((KNetworkProviderID)i))
			{
				const KLabeledValue& value = modEntry->GetWebSite((KNetworkProviderID)i);

				KxXMLNode linkNode = sitesNode.NewElement("a");
				linkNode.SetValue(value.GetLabel());
				linkNode.SetAttribute("href", value.GetValue());

				sitesNode.NewElement("br");
			}
		}
		for (const auto& value: modEntry->GetWebSites())
		{
			KxXMLNode linkNode = sitesNode.NewElement("a");
			linkNode.SetValue(value.GetLabel());
			linkNode.SetAttribute("href", value.GetValue());

			sitesNode.NewElement("br");
		}

		// Description
		KxXMLNode descriptionNode = rowNode.NewElement("td");
		wxString description = modEntry->GetDescription();
		if (!description.IsEmpty())
		{
			description.Replace("\r\n", "<br/>", true);
			description.Replace("\r", "<br/>", true);
			description.Replace("\n", "<br/>", true);

			descriptionNode.NewElement("details").SetValue(description, true);
		}
	}

	KxFileStream stream(outputFilePath, KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways);
	stream.WriteStringUTF8(xml.GetXML(KxXML_PRINT_HTML5));
}

void KModManager::NotifyModInstalled(KModEntry& modEntry)
{
	KModEntry* newMod = FindModBySignature(modEntry.GetSignature());
	if (newMod == NULL)
	{
		newMod = DoLoadMod(modEntry.GetSignature());
	}

	if (newMod)
	{
		newMod->UpdateFileTree();
		ResortMods();

		KDispatcher::GetInstance()->InvalidateVirtualTree();
		KModWorkspace::GetInstance()->ReloadWorkspace();
		KEvent::MakeQueue<KModEvent>(KEVT_MOD_INSTALLED, *newMod);

		Save();
	}
}
void KModManager::NotifyModUninstalled(KModEntry& modEntry)
{
	modEntry.UpdateFileTree();
	KDispatcher::GetInstance()->InvalidateVirtualTree();

	Save();
	KEvent::MakeSend<KModEvent>(KEVT_MOD_UNINSTALLED, modEntry);
}
void KModManager::NotifyModErased(KModEntry& modEntry)
{
	intptr_t index = GetModOrderIndex(&modEntry);
	if (index != -1)
	{
		const wxString modID = modEntry.GetID();
		m_ModEntries.erase(m_ModEntries.begin() + index);

		KDispatcher::GetInstance()->InvalidateVirtualTree();
		KModWorkspace::GetInstance()->ReloadWorkspace();
		
		Save();
		KEvent::MakeSend<KModEvent>(KEVT_MOD_UNINSTALLED, modID);
	}
}
