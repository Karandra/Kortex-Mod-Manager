#include "stdafx.h"
#include "DefaultModManager.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/ModTagManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/VirtualGameFolder.hpp>

#include "DefaultModManager.h"
#include "Workspace.h"
#include "DisplayModel.h"
#include "BasicGameMod.h"
#include "UI/KMainWindow.h"
#include "UI/KWorkspace.h"
#include "UI/KWorkspaceController.h"
#include "Utility/KOperationWithProgress.h"
#include "Utility/KUPtrVectorUtil.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxProgressDialog.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxString.h>
#include <execution>

namespace Kortex::ModManager
{
	void DefaultModManager::DoResortMods(const IGameProfile& profile)
	{
		size_t modIndex = 0;
		for (const GameInstance::ProfileMod& listEntry: profile.GetMods())
		{
			if (modIndex < m_Mods.size())
			{
				intptr_t currentElement = -1;
				FindModBySignature(listEntry.GetSignature(), &currentElement);

				if (currentElement != -1)
				{
					m_Mods[currentElement]->SetActive(listEntry.IsActive());
					std::swap(m_Mods[currentElement], m_Mods[modIndex]);
				}
				modIndex++;
			}
		}

		if (IModDispatcher::HasInstance())
		{
			IModDispatcher::GetInstance()->InvalidateVirtualTree();
		}
	}
	void DefaultModManager::DoUninstallMod(IGameMod& mod, wxWindow* window, const bool erase)
	{
		ModEvent event(Events::ModUnsinstalling, mod);
		event.Send();

		// If signature is empty, removing this mod can cause removing ALL other mods
		// because mod folder path will point to all mods directory instead of its own.
		// Just ignore this. User can always delete this folder manually.
		if (event.IsAllowed() && !mod.GetSignature().IsEmpty())
		{
			// Disable it
			mod.SetActive(false);

			if (!erase)
			{
				mod.SetUninstallTime(wxDateTime::Now());
				mod.Save();
			}
			const wxString path = erase ? mod.GetRootDir() : mod.GetModFilesDir();

			KOperationWithProgressDialogBase* operation = new KOperationWithProgressDialogBase(true, window);
			operation->OnRun([path = path.Clone()](KOperationWithProgressBase* self)
			{
				KxEvtFile folder(path);
				self->LinkHandler(&folder, KxEVT_FILEOP_REMOVE_FOLDER);
				folder.RemoveFolderTree(true);
			});
			operation->OnEnd([this, &mod, erase](KOperationWithProgressBase* self)
			{
				Save();
				if (erase)
				{
					NotifyModErased(mod);
				}
				else
				{
					NotifyModUninstalled(mod);
				}
			});
			operation->SetDialogCaption(KTr("ModManager.RemoveMod.RemovingMessage"));
			operation->Run();
		}
	}
	IGameMod* DefaultModManager::DoCreateMod(const wxString& signature)
	{
		if (!signature.IsEmpty())
		{
			auto mod = NewMod();
			if (mod->LoadUsingSignature(signature))
			{
				IModTagManager::GetInstance()->LoadTagsFromMod(*mod);
				return &EmplaceMod(std::move(mod));
			}
		}
		return nullptr;
	}

	void DefaultModManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		m_Config.OnLoadInstance(instance, managerNode);
	}
	void DefaultModManager::OnInit()
	{
		IEvent::Bind(Events::ModFilesChanged, &DefaultModManager::OnModFilesChanged, this);

		// Mandatory locations
		for (const MandatoryLocation& location: m_Config.GetMandatoryLocations())
		{
			const int orderIndex = m_BaseGame.GetOrderIndex() + m_MandatoryMods.size() + 1;
			FixedGameMod& entry = m_MandatoryMods.emplace_back(orderIndex);

			entry.SetID(location.GetName());
			entry.SetActive(true);
			entry.LinkLocation(location.GetSource());
		}

		// Base game
		m_BaseGame.SetID(Variables::KVAR_GAME_ID);
		m_BaseGame.SetName(ITranslator::GetVariable(Variables::KVAR_GAME_NAME));
		m_BaseGame.SetActive(true);
		m_BaseGame.LinkLocation(ITranslator::GetVariable(Variables::KVAR_ACTUAL_GAME_DIR));

		// Write target
		m_Overwrites.SetID(Variables::KVAR_OVERWRITES_DIR);
		m_Overwrites.SetName(KTr("ModManager.WriteTargetName"));
		m_Overwrites.SetActive(true);
		// Location will be linked on profile change

		Load();
	}
	void DefaultModManager::OnExit()
	{
		m_VFS.Disable();
	}

	void DefaultModManager::OnMountPointsError(const KxStringVector& locations)
	{
		KxTaskDialog dialog(KMainWindow::GetInstance(), KxID_NONE, KTr("VFS.MountPointNotEmpty.Caption"), KTr("VFS.MountPointNotEmpty.Message"), KxBTN_OK, KxICON_ERROR);
		dialog.SetOptionEnabled(KxTD_HYPERLINKS_ENABLED);
		dialog.SetOptionEnabled(KxTD_EXMESSAGE_EXPANDED);

		wxString message;
		for (const wxString& path: locations)
		{
			message += KxString::Format(wxS("<a href=\"%1\">%2</a>\r\n"), path, path);
		}
		dialog.SetExMessage(message);

		dialog.Bind(wxEVT_TEXT_URL, [&dialog](wxTextUrlEvent& event)
		{
			KxShell::Execute(&dialog, event.GetString(), wxS("open"));
		});
		dialog.ShowModal();
	}
	void DefaultModManager::OnModFilesChanged(ModEvent& event)
	{
		if (event.HasMod())
		{
			event.GetMod()->UpdateFileTree();
		}
		for (IGameMod* mod: event.GetModsArray())
		{
			mod->UpdateFileTree();
		}
	}

	DefaultModManager::DefaultModManager()
		:m_VFS(*this),
		m_BaseGame(std::numeric_limits<int>::min()), m_Overwrites(std::numeric_limits<int>::max())
	{
	}

	KWorkspace* DefaultModManager::GetWorkspace() const
	{
		return Workspace::GetInstance();
	}
	void DefaultModManager::Load()
	{
		m_Mods.clear();

		// Load entries
		if (IGameInstance* instnace = IGameInstance::GetActive())
		{
			KxFileFinder finder(instnace->GetModsDir(), wxS('*'));
			for (KxFileItem item = finder.FindNext(); item.IsOK(); item = finder.FindNext())
			{
				if (item.IsNormalItem() && item.IsDirectory())
				{
					const wxString signature = item.GetName();
					DoCreateMod(signature);
				}
			}
		}

		// Build mod file trees
		IGameMod::RefVector allEntries = GetAllMods(false, true);

		// Using 'std::execution::seq/unseq' generates too much strain on IO.
		// And doesn't really improves loading speed. Using 'seq' for now.
		std::for_each(std::execution::seq, allEntries.begin(), allEntries.end(), [](IGameMod* entry)
		{
			entry->UpdateFileTree();
		});

		if (IModDispatcher::HasInstance())
		{
			IModDispatcher::GetInstance()->InvalidateVirtualTree();
		}
	}
	void DefaultModManager::Save() const
	{
		IGameProfile* profile = IGameInstance::GetActiveProfile();
		if (profile)
		{
			profile->SyncWithCurrentState();
			profile->SaveConfig();
		}
	}

	std::unique_ptr<IGameMod> DefaultModManager::NewMod()
	{
		return std::make_unique<BasicGameMod>();
	}
	IGameMod::RefVector DefaultModManager::GetMandatoryMods()
	{
		IGameMod::RefVector refs;
		refs.reserve(m_MandatoryMods.size());
		for (IGameMod& mod: m_MandatoryMods)
		{
			refs.push_back(&mod);
		}
		return refs;
	}
	IGameMod::RefVector DefaultModManager::GetAllMods(bool activeOnly, bool includeWriteTarget)
	{
		IGameMod::RefVector allMods;
		allMods.reserve(m_Mods.size() + m_MandatoryMods.size() + 2);

		// Add game root as first virtual folder
		allMods.push_back(&m_BaseGame);

		// Add mandatory virtual folders
		for (KMandatoryModEntry& mod: m_MandatoryMods)
		{
			allMods.push_back(&mod);
		}

		// Add mods
		for (auto& mod: m_Mods)
		{
			if (!activeOnly || (activeOnly && mod->IsActive()))
			{
				allMods.push_back(mod.get());
			}
		}

		// Add write target
		if (includeWriteTarget)
		{
			allMods.push_back(&m_Overwrites);
		}

		return allMods;
	}

	void DefaultModManager::ResortMods()
	{
		DoResortMods(*IGameInstance::GetActiveProfile());
	}
	void DefaultModManager::ResortMods(const IGameProfile& profile)
	{
		DoResortMods(profile);
	}

	IGameMod* DefaultModManager::FindModByID(const wxString& modID, intptr_t* index) const
	{
		intptr_t i = 0;
		for (auto& entry: m_Mods)
		{
			if (entry->GetID() == modID)
			{
				KxUtility::SetIfNotNull(index, i);
				return &*entry;
			}
			i++;
		}

		KxUtility::SetIfNotNull(index, -1);
		return nullptr;
	}
	IGameMod* DefaultModManager::FindModByName(const wxString& modName, intptr_t* index) const
	{
		intptr_t i = 0;
		for (auto& entry: m_Mods)
		{
			if (entry->GetName() == modName)
			{
				KxUtility::SetIfNotNull(index, i);
				return &*entry;
			}
			i++;
		}

		KxUtility::SetIfNotNull(index, -1);
		return nullptr;
	}
	IGameMod* DefaultModManager::FindModBySignature(const wxString& signature, intptr_t* index) const
	{
		intptr_t i = 0;
		for (auto& entry: m_Mods)
		{
			if (entry->GetSignature() == signature)
			{
				KxUtility::SetIfNotNull(index, i);
				return &*entry;
			}
			i++;
		}

		KxUtility::SetIfNotNull(index, -1);
		return nullptr;
	}
	IGameMod* DefaultModManager::FindModByNetworkID(NetworkProviderID providerID, NetworkModInfo modInfo, intptr_t* index) const
	{
		if (INetworkProvider* provider = INetworkManager::GetInstance()->GetProvider(providerID))
		{
			intptr_t i = 0;
			for (auto& entry: m_Mods)
			{
				ModProviderItem* item = entry->GetProviderStore().GetItem(*provider);
				if (item && item->GetModInfo() == modInfo)
				{
					KxUtility::SetIfNotNull(index, i);
					return &*entry;
				}
				i++;
			}
		}

		KxUtility::SetIfNotNull(index, -1);
		return nullptr;
	}

	bool DefaultModManager::IsModActive(const wxString& modID) const
	{
		const IGameMod* mod = FindModByID(modID);
		if (mod)
		{
			return mod->IsActive();
		}
		return false;
	}
	bool DefaultModManager::ChangeModID(IGameMod& mod, const wxString& newID)
	{
		if (FindModByID(newID) == nullptr)
		{
			wxString oldPath = mod.GetRootDir();
			BasicGameMod tempEntry;
			tempEntry.SetID(newID);

			if (KxFile(oldPath).Rename(tempEntry.GetRootDir(), false))
			{
				mod.SetID(newID);
				mod.Save();

				// Save new mod order with changed signature.
				// Reloading is not needed
				Save();

				// This will take care of file tree
				ModEvent(Events::ModFilesChanged, mod).Send();
				return true;
			}
		}
		return false;
	}

	bool DefaultModManager::MoveModsBefore(const IGameMod::RefVector& toMove, const IGameMod& anchor)
	{
		return KUPtrVectorUtil::MoveBefore(m_Mods, toMove, anchor);
	}
	bool DefaultModManager::MoveModsAfter(const IGameMod::RefVector& toMove, const IGameMod& anchor)
	{
		return KUPtrVectorUtil::MoveAfter(m_Mods, toMove, anchor);
	}

	void DefaultModManager::ExportModList(const wxString& outputFilePath) const
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
		for (const auto& modEntry: m_Mods)
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
			AddCheckBox(rowNode.GetFirstChildElement().GetNextSiblingElement(), modEntry->IsActive(), "Is active");

			// Add sites
			KxXMLNode sitesNode = rowNode.NewElement("td");
			modEntry->GetProviderStore().Visit([&sitesNode](const ModProviderItem& item)
			{
				KxXMLNode linkNode = sitesNode.NewElement("a");
				linkNode.SetValue(item.GetName());
				linkNode.SetAttribute("href", item.GetURL());

				sitesNode.NewElement("br");
				return true;
			});

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

	void DefaultModManager::NotifyModInstalled(IGameMod& mod)
	{
		// Find mod in mod list
		IGameMod* newMod = FindModBySignature(mod.GetSignature());

		// If it's not not found that means this is completely new mod,
		// so create its entry now.
		if (newMod == nullptr)
		{
			newMod = DoCreateMod(mod.GetSignature());
		}

		if (newMod)
		{
			newMod->UpdateFileTree();
			ResortMods();

			IModDispatcher::GetInstance()->InvalidateVirtualTree();
			IEvent::MakeSend<ModEvent>(Events::ModInstalled, *newMod);
			ScheduleReloadWorkspace();

			Save();
		}
	}
	void DefaultModManager::NotifyModUninstalled(IGameMod& mod)
	{
		mod.UpdateFileTree();
		IModDispatcher::GetInstance()->InvalidateVirtualTree();

		Save();
		IEvent::MakeSend<ModEvent>(Events::ModUninstalled, mod);
	}
	void DefaultModManager::NotifyModErased(IGameMod& mod)
	{
		intptr_t index = GetOrderIndex(mod);
		if (index != -1)
		{
			const wxString modID = mod.GetID();
			m_Mods.erase(m_Mods.begin() + index);

			IModDispatcher::GetInstance()->InvalidateVirtualTree();
			Workspace::GetInstance()->ReloadWorkspace();

			Save();
			IEvent::MakeSend<ModEvent>(Events::ModUninstalled, modID);
		}
	}
}

namespace Kortex::ModManager
{
	MirroredLocation::MirroredLocation(const KxXMLNode& parentNode)
	{
		for (KxXMLNode node = parentNode.GetFirstChildElement("Sources").GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			m_Sources.emplace_back(node.GetValue());
		}
		m_Target = parentNode.GetFirstChildElement("Target").GetValue();
	}

	KxStringVector MirroredLocation::GetSources() const
	{
		return KAux::ExpandVariablesInVector(m_Sources);
	}
	wxString MirroredLocation::GetSource() const
	{
		return !m_Sources.empty() ? KVarExp(m_Sources.front()) : KxNullWxString;
	}
	wxString MirroredLocation::GetTarget() const
	{
		return KVarExp(m_Target);
	}

	MandatoryLocation::MandatoryLocation(const KxXMLNode& parentNode)
	{
		// Folder path will not be expanded here
		m_Source = parentNode.GetValue();
		m_Name = KVarExp(parentNode.GetAttribute("Name"));
	}
}

namespace Kortex::ModManager
{
	wxString MandatoryLocation::GetSource() const
	{
		return KVarExp(m_Source);
	}
	wxString MandatoryLocation::GetName() const
	{
		return KVarExp(m_Name);
	}
}

namespace Kortex::ModManager
{
	 void Config::OnLoadInstance(IGameInstance& profile, const KxXMLNode& node)
	{
		auto ReadEntries = [](auto& items, const KxXMLNode& rootNode, const wxString& name)
		{
			for (KxXMLNode node = rootNode.GetFirstChildElement(name).GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
			{
				if (!items.emplace_back(node).IsOK())
				{
					items.pop_back();
				}
			}
		};

		ReadEntries(m_MirroredLocations, node, "MirroredLocations");
		ReadEntries(m_MandatoryLocations, node, "MandatoryLocations");
	}
}
