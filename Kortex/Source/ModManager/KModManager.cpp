#include "stdafx.h"
#include "KVariablesDatabase.h"
#include "KModManager.h"
#include "KModWorkspace.h"
#include "KModManagerModel.h"
#include "KModEntry.h"
#include "KVirtualGameFolderWorkspace.h"
#include "Profile/KProfile.h"
#include "Profile/KVirtualizationConfig.h"
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
#include "KOperationWithProgress.h"
#include "KApp.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxProgressDialog.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxString.h>

KxSingletonPtr_Define(KModManager);

wxString KModManager::GetLocation(KModManagerLocation nLocation, const wxString& signature)
{
	const KProfile* profile = KApp::Get().GetCurrentProfile();
	switch (nLocation)
	{
		case KMM_LOCATION_MODS_ORDER:
		{
			return profile->GetRCPD({"ModOrder.xml"});
		}
		case KMM_LOCATION_MODS_FOLDER:
		{
			return profile->GetVariables().GetVariable(KVAR_MODS_ROOT);
		}
		case KMM_LOCATION_MOD_ROOT:
		{
			return profile->GetVariables().GetVariable(KVAR_MODS_ROOT) + wxS('\\') + signature;
		}
		default:
		{
			wxLogWarning("Invalid location index in KModManager::GetLocation(nLocation = %d)", (int)nLocation);
			break;
		}
	};
	return wxEmptyString;
}

void KModManager::SortEntries()
{
	size_t index = 0;
	for (const KModListModEntry& listEntry: m_ModListManager.GetCurrentList().GetMods())
	{
		if (index < m_ModEntries.size())
		{
			int64_t currentElement = GetModIndex(listEntry.GetMod());
			if (currentElement != -1)
			{
				m_ModEntries[currentElement]->SetEnabled(listEntry.IsEnabled());
				std::swap(m_ModEntries[currentElement], m_ModEntries[index]);
			}
			index++;
		}
	}

	if (KPluginManagerWorkspace* pluginsWorkspace = KPluginManagerWorkspace::GetInstance())
	{
		pluginsWorkspace->ScheduleReload();
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
		wxString path = modEntry->GetLocation(erase ? KMM_LOCATION_MOD_ROOT : KMM_LOCATION_MOD_FILES);

		KOperationWithProgressDialogBase* operation = new KOperationWithProgressDialogBase(true, window);	
		operation->OnRun([path = path.Clone(), modEntry](KOperationWithProgressBase* self)
		{
			KxEvtFile folder(path);
			self->LinkHandler(&folder, KxEVT_FILEOP_REMOVE_FOLDER);
			folder.RemoveFolderTree(true);
		});
		operation->OnEnd([this, modID = modEntry->GetID().Clone()](KOperationWithProgressBase* self)
		{
			Save();

			KModEvent(KEVT_MOD_UNINSTALLED, modID).Send();
		});
		operation->SetDialogCaption(T("ModManager.RemoveMod.RemovingMessage"));
		operation->Run();
	}
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
				folders.push_back(entry->GetLocation(KMM_LOCATION_MOD_FILES));
			}
		}

		if (KIPCClient::GetInstance()->CreateVFS_Convergence(mountPoint, m_ModEntry_WriteTarget.GetLocation(KMM_LOCATION_MOD_FILES), folders, true))
		{
			//return KIPCClient::GetInstance()->ConvergenceVFS_BuildDispatcherIndex();
			return KIPCClient::GetInstance()->ConvergenceVFS_SetDispatcherIndex();
		}
	}
	return false;
}
bool KModManager::InitMirroredLocations()
{
	KIPCClient::GetInstance()->MirrorVFS_ClearList();

	const KVirtualizationConfig* virtualizationConfig = KVirtualizationConfig::GetInstance();

	// Check folders first
	for (size_t i = 0; i < virtualizationConfig->GetEntriesCount(KPVE_MIRRORED); i++)
	{
		const KVirtualizationEntry* entry = virtualizationConfig->GetEntryAt(KPVE_MIRRORED, i);
		if (entry && !CheckMountPoint(entry->GetDestination()))
		{
			return false;
		}
	}

	// Initialize them
	for (size_t i = 0; i < virtualizationConfig->GetEntriesCount(KPVE_MIRRORED); i++)
	{
		const KVirtualizationEntry* entry = virtualizationConfig->GetEntryAt(KPVE_MIRRORED, i);
		if (entry)
		{
			KIPCClient::GetInstance()->CreateVFS_Mirror(entry->GetSource(), entry->GetDestination());
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
	m_MountStatusDialog->SetCaption(IsVFSMounted() ? T("VFS.MountingCaptionDisable") : T("VFS.MountingCaptionEnable"));
	m_MountStatusDialog->SetLabel(T("VFS.MountingMessage"));
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
	KxTaskDialog dialog(KMainWindow::GetInstance(), KxID_NONE, T("VFS.MountPointNotEmpty.Caption"), T("VFS.MountPointNotEmpty.Message"), KxBTN_OK, KxICON_ERROR);
	dialog.SetOptionEnabled(KxTD_HYPERLINKS_ENABLED);
	dialog.SetOptionEnabled(KxTD_EXMESSAGE_EXPANDED);
	dialog.SetExMessage(wxString::Format("<a href=\"%s\">%s</a>", folderPath, folderPath));

	dialog.Bind(wxEVT_TEXT_URL, [&dialog](wxTextUrlEvent& event)
	{
		KxShell::Execute(&dialog, event.GetString(), "open");
	});
	dialog.ShowModal();
}

void KModManager::OnModFilesChnaged(KModEvent& event)
{
	if (event.HasMod())
	{
		event.GetMod()->UpdateFileTree();
	}

	KWorkspace::ScheduleReloadOf<KVirtualGameFolderWorkspace>();
	KWorkspace::ScheduleReloadOf<KPluginManagerWorkspace>();
}
void KModManager::OnModToggled(KModEvent& event)
{
	KWorkspace::ScheduleReloadOf<KVirtualGameFolderWorkspace>();
	KWorkspace::ScheduleReloadOf<KPluginManagerWorkspace>();
}
void KModManager::OnModsReordered(KModEvent& event)
{
	KWorkspace::ScheduleReloadOf<KVirtualGameFolderWorkspace>();
	KWorkspace::ScheduleReloadOf<KModWorkspace>();
	KWorkspace::ScheduleReloadOf<KPluginManagerWorkspace>();
}

void KModManager::OnModInstalled(KModEvent& event)
{
	Load();
	KWorkspace::ScheduleReloadOf<KModWorkspace>();
}
void KModManager::OnModUninstalled(KModEvent& event)
{
	Load();
	KWorkspace::ScheduleReloadOf<KModWorkspace>();
}

void KModManager::OnModListSelected(KModListEvent& event)
{
	Load();

	KWorkspace::ScheduleReloadOf<KVirtualGameFolderWorkspace>();
	KWorkspace::ScheduleReloadOf<KModWorkspace>();
	KWorkspace::ScheduleReloadOf<KPluginManagerWorkspace>();
}

KModManager::KModManager(KWorkspace* workspace)
	:m_Options(this, "General"),
	m_ModEntry_BaseGame(std::numeric_limits<int>::min()), m_ModEntry_WriteTarget(std::numeric_limits<int>::max())
{
	// Mandatory locations
	for (const wxString& folderPath: KVirtualizationConfig::GetInstance()->GetMandatoryVirtualFolders())
	{
		int orderIndex = m_ModEntry_BaseGame.GetOrderIndex() + m_ModEntry_Mandatory.size() + 1;
		KFixedModEntry& entry = m_ModEntry_Mandatory.emplace_back(orderIndex);

		entry.SetID(folderPath.AfterLast('\\'));
		entry.SetEnabled(true);
		entry.SetLinkedModLocation(folderPath);
	}

	// Base game
	m_ModEntry_BaseGame.SetID(V("$(ID)"));
	m_ModEntry_BaseGame.SetName(V("$(Name)"));
	m_ModEntry_BaseGame.SetEnabled(true);
	m_ModEntry_BaseGame.SetLinkedModLocation(V(KVAR(KVAR_GAME_ROOT)));

	// Write target
	m_ModEntry_WriteTarget.SetID("WriteTargetRoot");
	m_ModEntry_WriteTarget.SetName(T("ModManager.WriteTargetName"));
	m_ModEntry_WriteTarget.SetEnabled(true);

	// Events
	KEvent::Bind(KEVT_MOD_FILES_CHANGED, &KModManager::OnModFilesChnaged, this);
	KEvent::Bind(KEVT_MOD_TOGGLED, &KModManager::OnModToggled, this);
	KEvent::Bind(KEVT_MODS_REORDERED, &KModManager::OnModsReordered, this);

	KEvent::Bind(KEVT_MOD_INSTALLED, &KModManager::OnModInstalled, this);
	KEvent::Bind(KEVT_MOD_UNINSTALLED, &KModManager::OnModUninstalled, this);

	KEvent::Bind(KEVT_MODLIST_SELECTED, &KModManager::OnModListSelected, this);
	
	// Load data
	Load();
}
void KModManager::Clear()
{
	for (KModEntry* entry: m_ModEntries)
	{
		delete entry;
	}
	m_ModEntries.clear();
	m_ModListManager.ClearLists();
}
KModManager::~KModManager()
{
	m_ModListManager.SaveLists();

	DestroyMountStatusDialog();
	Clear();
}

wxString KModManager::GetID() const
{
	return "KModManager";
}
wxString KModManager::GetName() const
{
	return T("ModManager.Name");
}
wxString KModManager::GetVersion() const
{
	return "1.3";
}
KWorkspace* KModManager::GetWorkspace() const
{
	return KModWorkspace::GetInstance();
}

KModEntryArray KModManager::GetAllEntries(bool includeWriteTarget)
{
	KModEntryArray entries;
	entries.reserve(m_ModEntries.size() + m_ModEntry_Mandatory.size() + 2);

	// Add mandatory virtual folders
	for (KFixedModEntry& entry: m_ModEntry_Mandatory)
	{
		entries.push_back(&entry);
	}

	// Add game root as first (after mandatory) virtual folder
	entries.push_back(&m_ModEntry_BaseGame);

	// Add mods
	for (KModEntry* entry: m_ModEntries)
	{
		entries.push_back(entry);
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
	KxStringVector filesList = KxFile(GetLocation(KMM_LOCATION_MODS_FOLDER)).Find(KxFile::NullFilter, KxFS_FOLDER, false);
	if (!filesList.empty())
	{
		m_ModEntries.reserve(filesList.size());
		for (size_t i = 0; i < filesList.size(); i++)
		{
			wxString signature = KxFile(filesList[i]).GetFullName();
			if (!signature.IsEmpty())
			{
				auto entry = std::make_unique<KModEntry>();
				entry->CreateFromSignature(signature);
				if (entry->IsOK())
				{
					m_TagManager.LoadTagsFromEntry(entry.get());
					m_ModEntries.push_back(entry.release());
				}
			}
		}
	}

	// Load lists
	for (KModEntry* entry: GetAllEntries(true))
	{
		entry->UpdateFileTree();
	}
	m_ModListManager.ReloadLists();
	SortEntries();

	// Causes rebuild of virtual file tree
	KModEvent(KEVT_MOD_FILES_CHANGED).Send();
}
void KModManager::Save() const
{
	const_cast<KModListManager&>(m_ModListManager).SyncCurrentList();
	const_cast<KModListManager&>(m_ModListManager).SaveLists();
}
bool KModManager::ChangeModListAndResort(const wxString& newModListID)
{
	if (m_ModListManager.SetCurrentListID(newModListID))
	{
		SortEntries();
		return true;
	}
	return false;
}

KModEntry* KModManager::FindModByID(const wxString& modID) const
{
	for (KModEntry* entry: m_ModEntries)
	{
		if (entry->GetID() == modID)
		{
			return entry;
		}
	}
	return NULL;
}
KModEntry* KModManager::FindModByName(const wxString& modName) const
{
	for (KModEntry* entry: m_ModEntries)
	{
		if (entry->GetName() == modName)
		{
			return entry;
		}
	}
	return NULL;
}
KModEntry* KModManager::FindModBySignature(const wxString& signature) const
{
	for (KModEntry* entry: m_ModEntries)
	{
		if (entry->GetSignature() == signature)
		{
			return entry;
		}
	}
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
		wxString oldPath = GetLocation(KMM_LOCATION_MOD_ROOT, entry->GetSignature());
		wxString newPath = GetLocation(KMM_LOCATION_MOD_ROOT, KModEntry::GetSignatureFromID(newID));
		
		if (KxFile(oldPath).Rename(newPath, false))
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

intptr_t KModManager::GetModIndex(const KModEntry* modEntry) const
{
	auto it = std::find(m_ModEntries.begin(), m_ModEntries.end(), modEntry);
	if (it != m_ModEntries.end())
	{
		return std::distance(m_ModEntries.begin(), it);
	}
	return -1;
}
bool KModManager::MoveModsIntoThis(const KModEntryArray& entriesToMove, const KModEntry* anchor, KModManagerModsMoveType moveMode)
{
	// Check if anchor is not one of moved elements
	if (std::find(entriesToMove.begin(), entriesToMove.end(), anchor) != entriesToMove.end())
	{
		return false;
	}

	auto it = std::find(m_ModEntries.begin(), m_ModEntries.end(), anchor);
	if (it != m_ModEntries.end())
	{
		// Remove from existing place
		m_ModEntries.erase(std::remove_if(m_ModEntries.begin(), m_ModEntries.end(), [&entriesToMove](const KModEntry* entry)
		{
			return std::find(entriesToMove.begin(), entriesToMove.end(), entry) != entriesToMove.end();
		}), m_ModEntries.end());

		// Iterator may have been invalidated
		it = std::find(m_ModEntries.begin(), m_ModEntries.end(), anchor);
		if (it != m_ModEntries.end())
		{
			switch (moveMode)
			{
				case KMM_MOVEMOD_BEFORE:
				{
					m_ModEntries.insert(it, entriesToMove.begin(), entriesToMove.end());
					break;
				}
				default:
				{
					size_t index = 1;
					for (auto i = entriesToMove.begin(); i != entriesToMove.end(); ++i)
					{
						m_ModEntries.insert(it + index, *i);
						index++;
					}
					break;
				}
			};

			Save();
			return true;
		}
	}
	return false;
}

wxString KModManager::GetVirtualGameRoot() const
{
	return V(KVAR(KVAR_VIRTUAL_GAME_ROOT));
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
	for (const KModEntry* modEntry: m_ModEntries)
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

	KxFileStream stream(outputFilePath, KxFS_ACCESS_WRITE, KxFS_DISP_CREATE_ALWAYS);
	stream.WriteStringUTF8(xml.GetXML(KxXML_PRINT_HTML5));
}
