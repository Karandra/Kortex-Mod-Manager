#include "stdafx.h"
#include "DefaultModManager.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/ModTagManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/InstallWizard.hpp>

#include "VirtualFileSystem/VirtualFSEvent.h"
#include "DefaultModManager.h"
#include "Workspace.h"
#include "GameMods/VirtualGameFolder/Workspace.h"
#include "DisplayModel.h"
#include "BasicGameMod.h"
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

namespace Kortex::ModManager
{
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
	void DefaultModManager::ProcessInstallMod(IGameMod& mod)
	{
		// Find mod in mod list
		IGameMod* newMod = FindModBySignature(mod.GetSignature());

		// If it's not found that means this is completely new mod,
		// so create its entry now.
		if (newMod == nullptr)
		{
			newMod = DoCreateMod(mod.GetSignature());
		}
		if (newMod)
		{
			BroadcastProcessor::Get().ProcessEvent(ModEvent::EvtInstalled, *newMod);
		}
	}

	void DefaultModManager::DoUninstallMod(IGameMod& mod, const bool erase)
	{
		ModEvent event(mod);
		BroadcastProcessor::Get().ProcessEvent(event, ModEvent::EvtUnsinstalling);

		// If signature is empty, removing this mod can cause removing *ALL* other mods
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

			auto operation = new KOperationWithProgressDialogBase(true, Workspace::GetInstance());
			operation->OnRun([path = path.Clone()](KOperationWithProgressBase* self)
			{
				KxEvtFile folder(path);
				self->LinkHandler(&folder, KxEVT_FILEOP_REMOVE_FOLDER);
				folder.RemoveFolderTree(true);
			});
			operation->OnEnd([this, &mod, erase](KOperationWithProgressBase* self)
			{
				BroadcastProcessor::Get().CallAfter([this, &mod, erase]()
				{
					Save();
					if (erase)
					{
						ProcessEraseMod(mod);
					}
					else
					{
						ProcessUninstallMod(mod);
					}
				});
			});
			operation->SetDialogCaption(KTr("ModManager.RemoveMod.RemovingMessage"));
			operation->Run();
		}
	}
	void DefaultModManager::ProcessUninstallMod(IGameMod& mod)
	{
		BroadcastProcessor::Get().ProcessEvent(ModEvent::EvtUninstalled, mod);
	}
	void DefaultModManager::ProcessEraseMod(IGameMod& mod)
	{
		auto it = std::find_if(m_Mods.begin(), m_Mods.end(), [&mod](const auto& searchedMod)
		{
			return &mod == searchedMod.get();
		});
		if (it != m_Mods.end())
		{
			const wxString modID = mod.GetID();
			const size_t index = std::distance(m_Mods.begin(), it);
			m_Mods.erase(it);
			RecalculatePriority(index);

			BroadcastProcessor::Get().ProcessEvent(ModEvent::EvtUninstalled, modID);
		}
	}

	void DefaultModManager::OnMountPointError(const KxStringVector& locations)
	{
		BroadcastProcessor::Get().QueueEvent(VirtualFSEvent::EvtMainToggleError, m_FileSystem, false);

		KxTaskDialog dialog(Workspace::GetInstance(), KxID_NONE, KTr("VFS.MountPointNotEmpty.Caption"), KTr("VFS.MountPointNotEmpty.Message"), KxBTN_OK, KxICON_ERROR);
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
	void DefaultModManager::OnUpdateModLayoutNeeded(ModEvent& event)
	{
		if (IGameMod* mod = event.GetMod())
		{
			mod->UpdateFileTree();
		}
		for (IGameMod* mod: event.GetModsArray())
		{
			mod->UpdateFileTree();
		}
		ResortMods();
	}
	void DefaultModManager::OnModLayoutSaveNeeded(ModEvent& event)
	{
		Save();
	}
	void DefaultModManager::OnProfileSelected(ProfileEvent& event)
	{
		if (IGameProfile* profile = event.GetProfile())
		{
			if (m_InitialLoadMods)
			{
				m_InitialLoadMods = false;

				// Queue resort to loader thread
				m_SortingProfile = profile;
				IModTagManager::GetInstance()->OnInitialModsLoading();
				Load();
			}
			else
			{
				ResortMods(*profile);
			}
		}
	}

	void DefaultModManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		m_Config.OnLoadInstance(instance, managerNode);
	}
	void DefaultModManager::OnInit()
	{
		// Events
		m_BroadcastReciever.Bind(ModEvent::EvtInstalled, &DefaultModManager::OnModLayoutSaveNeeded, this);
		m_BroadcastReciever.Bind(ModEvent::EvtInstalled, &DefaultModManager::OnUpdateModLayoutNeeded, this);

		m_BroadcastReciever.Bind(ModEvent::EvtUninstalled, &DefaultModManager::OnModLayoutSaveNeeded, this);
		m_BroadcastReciever.Bind(ModEvent::EvtUninstalled, &DefaultModManager::OnUpdateModLayoutNeeded, this);

		m_BroadcastReciever.Bind(ModEvent::EvtFilesChanged, &DefaultModManager::OnModLayoutSaveNeeded, this);
		m_BroadcastReciever.Bind(ModEvent::EvtFilesChanged, &DefaultModManager::OnUpdateModLayoutNeeded, this);
		m_BroadcastReciever.Bind(ModEvent::EvtToggled, &DefaultModManager::OnModLayoutSaveNeeded, this);

		m_BroadcastReciever.Bind(ProfileEvent::EvtSelected, &DefaultModManager::OnProfileSelected, this);

		// Mandatory locations
		for (const MandatoryLocation& location: m_Config.GetMandatoryLocations())
		{
			const int orderIndex = m_BaseGame.GetPriority() + m_MandatoryMods.size() + 1;
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
		m_WriteTarget.SetID(Variables::KVAR_OVERWRITES_DIR);
		m_WriteTarget.SetName(KTr("ModManager.WriteTargetName"));
		m_WriteTarget.SetActive(true);
		// m_WriteTarget.LinkLocation(...) Location will be linked on profile change

		m_InitialLoadMods = true;
	}
	void DefaultModManager::OnExit()
	{
		m_FileSystem.Disable();
	}
	void DefaultModManager::CreateWorkspaces()
	{
		new Workspace();
		new VirtualGameFolder::Workspace();
	}

	DefaultModManager::DefaultModManager()
		:m_FileSystem(*this), m_BaseGame(-65535), m_WriteTarget(65535)
	{
	}

	IWorkspace::RefVector DefaultModManager::EnumWorkspaces() const
	{
		return ToWorkspacesList(Workspace::GetInstance(), VirtualGameFolder::Workspace::GetInstance());
	}
	void DefaultModManager::Load()
	{
		// Clear mods list and update workspace immediately
		m_Mods.clear();
		BroadcastProcessor::Get().ProcessEvent(ModEvent::EvtBeginReload);

		m_ModsLoaderThread = std::make_unique<KxThread>();
		m_ModsLoaderThread->Bind(KxThreadEvent::EvtExecute, [this](KxThreadEvent& event)
		{
			// Load mods from disk
			if (IGameInstance* instnace = IGameInstance::GetActive())
			{
				KxFileFinder finder(instnace->GetModsDir(), wxS('*'));
				for (KxFileItem item = finder.FindNext(); item.IsOK(); item = finder.FindNext())
				{
					if (item.IsNormalItem() && item.IsDirectory())
					{
						DoCreateMod(item.GetName());
					}
				}
			}

			// Build mod file trees for all mods
			IGameMod::RefVector allMods = GetMods(GetModsFlags::Everything);

			size_t processed = 0;
			for (IGameMod* mod: allMods)
			{
				mod->UpdateFileTree();
				processed++;

				m_ModsLoaderThread->CallAfter([processed, total = allMods.size()]()
				{
					IMainWindow::GetInstance()->SetStatusProgress(processed, total);
				});
			}

			// Reset progress
			m_ModsLoaderThread->CallAfter([processed, total = allMods.size()]()
			{
				IMainWindow::GetInstance()->SetStatusProgress(0);
			});
		});
		m_ModsLoaderThread->Bind(KxThreadEvent::EvtFinished, [this](KxThreadEvent& event)
		{
			const IGameProfile* profile = m_SortingProfile ? m_SortingProfile : IGameProfile::GetActive();
			m_SortingProfile = nullptr;
			if (profile)
			{
				ResortMods(*profile);
			}

			IModDispatcher::GetInstance()->InvalidateVirtualTree();
			BroadcastProcessor::Get().ProcessEvent(ModEvent::EvtEndReload);
			m_ModsLoaderThread = nullptr;
		});

		m_ModsLoaderThread->Run();
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
	IGameMod::RefVector DefaultModManager::GetMods(GetModsFlags flags)
	{
		IGameMod::RefVector allMods;
		allMods.reserve(m_Mods.size() + m_MandatoryMods.size() + 2);

		if (flags & GetModsFlags::BaseGame)
		{
			// Add game root as first virtual folder
			allMods.push_back(&m_BaseGame);
		}

		if (flags & GetModsFlags::MandatoryMods)
		{
			// Add mandatory virtual folders
			for (KMandatoryModEntry& mod: m_MandatoryMods)
			{
				allMods.push_back(&mod);
			}
		}

		// Add regular mods
		for (auto& mod: m_Mods)
		{
			if (!(flags & GetModsFlags::ActiveOnly) || mod->IsActive())
			{
				allMods.push_back(mod.get());
			}
		}

		// Add write target
		if (flags & GetModsFlags::WriteTarget)
		{
			allMods.push_back(&m_WriteTarget);
		}

		return allMods;
	}
	size_t DefaultModManager::GetModsCount(ModManager::GetModsFlags flags)
	{
		size_t count = 0;

		if (flags & GetModsFlags::BaseGame)
		{
			count++;
		}

		if (flags & GetModsFlags::MandatoryMods)
		{
			// Add mandatory virtual folders
			for (KMandatoryModEntry& mod: m_MandatoryMods)
			{
				count++;
			}
		}

		// Add regular mods
		for (auto& mod: m_Mods)
		{
			if (!(flags & GetModsFlags::ActiveOnly) || mod->IsActive())
			{
				count++;
			}
		}

		// Add write target
		if (flags & GetModsFlags::WriteTarget)
		{
			count++;
		}

		return count;
	}

	IGameMod* DefaultModManager::FindModByID(const wxString& modID) const
	{
		for (auto& entry: m_Mods)
		{
			if (entry->GetID() == modID)
			{
				return &*entry;
			}
		}
		return nullptr;
	}
	IGameMod* DefaultModManager::FindModByName(const wxString& modName) const
	{
		for (auto& entry: m_Mods)
		{
			if (entry->GetName() == modName)
			{
				return &*entry;
			}
		}
		return nullptr;
	}
	IGameMod* DefaultModManager::FindModBySignature(const wxString& signature) const
	{
		for (auto& entry: m_Mods)
		{
			if (entry->GetSignature() == signature)
			{
				return &*entry;
			}
		}
		return nullptr;
	}
	IGameMod* DefaultModManager::FindModByModNetwork(const wxString& modNetworkName, NetworkModInfo modInfo) const
	{
		for (auto& entry: m_Mods)
		{
			ModSourceItem* item = entry->GetModSourceStore().GetItem(modNetworkName);
			if (item && item->GetModInfo() == modInfo)
			{
				return &*entry;
			}
		}
		return nullptr;
	}
	IGameMod* DefaultModManager::FindModByModNetwork(const IModNetwork& modNetwork, NetworkModInfo modInfo) const
	{
		return FindModByModNetwork(modNetwork.GetName(), modInfo);
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
				BroadcastProcessor::Get().ProcessEvent(ModEvent::EvtFilesChanged, mod);
				return true;
			}
		}
		return false;
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
			modEntry->GetModSourceStore().Visit([&sitesNode](const ModSourceItem& item)
			{
				KxXMLNode linkNode = sitesNode.NewElement("a");
				linkNode.SetValue(item.GetName());
				linkNode.SetAttribute("href", item.GetURI().BuildUnescapedURI());

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

				descriptionNode.NewElement("details").SetValue(description);
			}
		}

		KxFileStream stream(outputFilePath, KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways);
		stream.WriteStringUTF8(xml.GetXML(KxXML_PRINT_HTML5));
	}

	void DefaultModManager::InstallEmptyMod(const wxString& name)
	{
		if (!FindModByID(name))
		{
			IGameMod& mod = IModManager::GetInstance()->EmplaceMod();
			mod.SetID(name);
			mod.SetInstallTime(wxDateTime::Now());
			mod.Save();

			BroadcastProcessor::Get().QueueEvent(ModEvent::EvtInstalled, mod);
		}
	}
	void DefaultModManager::InstallModFromFolder(const wxString& sourcePath, const wxString& name, bool linkLocation)
	{
		if (!FindModByID(name))
		{
			IGameMod& mod = IModManager::GetInstance()->EmplaceMod();
			mod.SetID(name);
			mod.SetInstallTime(wxDateTime::Now());
			if (linkLocation)
			{
				mod.LinkLocation(sourcePath);
			}
			mod.Save();

			if (!linkLocation)
			{
				// Copy files
				wxString destinationPath = mod.GetModFilesDir();
				auto operation = new KOperationWithProgressDialog<KxFileOperationEvent>(true, Workspace::GetInstance());
				operation->OnRun([sourcePath, destinationPath](KOperationWithProgressBase* self)
				{
					KxEvtFile folder(sourcePath);
					self->LinkHandler(&folder, KxEVT_FILEOP_COPY_FOLDER);
					folder.CopyFolder(KxFile::NullFilter, destinationPath, true, true);
				});

				// If canceled, remove entire mod folder
				operation->OnCancel([&mod](KOperationWithProgressBase* self)
				{
					KxFile(mod.GetRootDir()).RemoveFolderTree(true);

					// Still send an event because mod entry was created regardless of
					// whether the process was canceled or not.
					BroadcastProcessor::Get().QueueEvent(ModEvent::EvtInstalled, mod);
				});

				// Reload after task is completed (successfully or not)
				operation->OnEnd([&mod](KOperationWithProgressBase* self)
				{
					BroadcastProcessor::Get().QueueEvent(ModEvent::EvtInstalled, mod);
				});

				// Configure and run
				operation->SetDialogCaption(KTr("ModManager.NewMod.CopyDialogCaption"));
				operation->Run();
			}
			else
			{
				BroadcastProcessor::Get().QueueEvent(ModEvent::EvtInstalled, mod);
			}
		}
	}
	void DefaultModManager::InstallModFromPackage(const wxString& packagePath)
	{
		// Install Wizard's dialog is auto-managed class, it will delete itself when needed
		new InstallWizard::WizardDialog(Workspace::GetInstance(), packagePath);
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
