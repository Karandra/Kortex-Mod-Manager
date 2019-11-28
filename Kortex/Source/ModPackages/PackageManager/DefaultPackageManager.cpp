#include "stdafx.h"
#include "DefaultPackageManager.h"
#include "PackageCreator/Workspace.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/ModManager.hpp>
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxFileBrowseDialog.h>

namespace Kortex::PackageDesigner
{
	void DefaultPackageManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		LoadRequirementsGroup(m_StandardRequirements, managerNode.GetFirstChildElement("Requirements"));
		
		// Find script extender
		wxString xSEID = instance.GetVariables().GetVariable(Variables::KVAR_SCRIPT_EXTENDER_ID).AsString();
		if (PackageProject::KPPRRequirementEntry* xSE = m_StandardRequirements.FindEntry(xSEID))
		{
			m_WithScriptExtender.Assign(*xSE);
			AddComponent<IWithScriptExtender>(m_WithScriptExtender);
		}
	}
	void DefaultPackageManager::OnInit()
	{
	}
	void DefaultPackageManager::OnExit()
	{
	}
	void DefaultPackageManager::CreateWorkspaces()
	{
		new Workspace();
	}

	IWorkspace::RefVector DefaultPackageManager::EnumWorkspaces() const
	{
		return ToWorkspacesList(Workspace::GetInstance());
	}

	void DefaultPackageManager::OnModListMenu(KxMenu& menu, const std::vector<IGameMod*>& selectedMods, IGameMod* focusedMod)
	{
		const bool isFixedMod = focusedMod->QueryInterface<ModManager::FixedGameMod>();
		const bool isPriorityGroup = focusedMod->QueryInterface<ModManager::PriorityGroup>();
		const bool isNormalMod = !isFixedMod && !isPriorityGroup;
		const bool isPackageExist = isNormalMod && focusedMod->IsPackageFileExist();

		KxMenu* packageMenu = new KxMenu();
		KxMenuItem* packageMenuItem = menu.Add(packageMenu, KTr("ModManager.Menu.Package"));
		if (isNormalMod && (!focusedMod || selectedMods.size() != 1))
		{
			packageMenuItem->Enable(false);
			return;
		}

		{
			KxMenuItem* item = packageMenu->Add(new KxMenuItem(KTr("ModManager.Menu.Package.Open")));
			item->Enable(isPackageExist);
			item->SetDefault();
			item->Bind(KxEVT_MENU_SELECT, [focusedMod](KxMenuEvent& event)
			{
				IModManager::GetInstance()->InstallModFromPackage(focusedMod->GetPackageFile());
			});
		}
		{
			KxMenuItem* item = packageMenu->Add(new KxMenuItem(KTr("ModManager.Menu.Package.OpenLocation")));
			item->Enable(isPackageExist);
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderOpen));
			item->Bind(KxEVT_MENU_SELECT, [focusedMod](KxMenuEvent& event)
			{
				KxShell::OpenFolderAndSelectItem(focusedMod->GetPackageFile());
			});
		}
		packageMenu->AddSeparator();

		{
			KxMenuItem* item = packageMenu->Add(new KxMenuItem(KTr("ModManager.Menu.Package.Assign")));
			item->Enable(!isFixedMod);
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::BoxSearchResult));
			item->Bind(KxEVT_MENU_SELECT, [focusedMod](KxMenuEvent& event)
			{
				KxFileBrowseDialog dialog(event.GetMenu()->GetWindow(), KxID_NONE, KxFBD_OPEN);
				dialog.SetFolder(focusedMod->GetPackageFile().BeforeLast('\\'));
				dialog.AddFilter("*.kmp;*.smi;*.fomod;*.7z;*.zip;", KTr("FileFilter.AllSupportedFormats"));
				dialog.AddFilter("*.kmp", KTr("FileFilter.ModPackage"));
				dialog.AddFilter("*.smi", KTr("FileFilter.ModPackageSMI"));
				dialog.AddFilter("*.fomod", KTr("FileFilter.ModPackageFOMod"));
				dialog.AddFilter("*.7z;*.zip;", KTr("FileFilter.Archives"));
				if (dialog.ShowModal() == KxID_OK)
				{
					focusedMod->SetPackageFile(dialog.GetResult());
					focusedMod->Save();

					BroadcastProcessor::Get().ProcessEvent(ModEvent::EvtChanged, *focusedMod);
				}
			});
		}
		{
			KxMenuItem* item = packageMenu->Add(new KxMenuItem(KTr("ModManager.Menu.Package.Remove")));
			item->Enable(isPackageExist);
			item->Bind(KxEVT_MENU_SELECT, [focusedMod](KxMenuEvent& event)
			{
				KxShell::FileOperation(focusedMod->GetPackageFile(), KxFS_FILE, KxFOF_DELETE, true, false, event.GetMenu()->GetWindow());
			});
		}
		{
			KxMenuItem* item = packageMenu->Add(new KxMenuItem(KTr("ModManager.Menu.Package.Extract")));
			item->Enable(isPackageExist);
			item->Bind(KxEVT_MENU_SELECT, [focusedMod](KxMenuEvent& event)
			{
				KxFileBrowseDialog dialog(event.GetMenu()->GetWindow(), KxID_NONE, KxFBD_OPEN_FOLDER);
				if (dialog.ShowModal() == KxID_OK)
				{
					// Extract archive in mod name folder inside the specified one.
					wxString outPath = dialog.GetResult() + wxS('\\') + focusedMod->GetSafeName();

					ExtractAcrhiveWithProgress(event.GetMenu()->GetWindow(), focusedMod->GetPackageFile(), outPath);
				}
			});
		}
		{
			KxMenuItem* item = packageMenu->Add(new KxMenuItem(KTr("ModManager.Menu.Package.ImportProject")));
			item->Enable(isPackageExist && Workspace::GetInstance() != nullptr);
			item->Bind(KxEVT_MENU_SELECT, [focusedMod](KxMenuEvent& event)
			{
				Workspace* workspace = Workspace::GetInstance();

				workspace->ImportProjectFromPackage(focusedMod->GetPackageFile());
				workspace->SwitchHere();
			});
		}
		{
			KxMenuItem* item = packageMenu->Add(new KxMenuItem(KTr("ModManager.Menu.Package.CreateProject")));
			item->Enable(isPackageExist);
			item->Bind(KxEVT_MENU_SELECT, [focusedMod](KxMenuEvent& event)
			{
				Workspace* workspace = Workspace::GetInstance();

				workspace->CreateProjectFromModEntry(*focusedMod);
				workspace->SwitchHere();
			});
		}
		packageMenu->AddSeparator();

		{
			KxMenuItem* item = packageMenu->Add(new KxMenuItem(KTr("ModManager.Menu.Properties")));
			item->Enable(isPackageExist);
			item->Bind(KxEVT_MENU_SELECT, [focusedMod](KxMenuEvent& event)
			{
				KxShell::Execute(event.GetMenu()->GetWindow(), focusedMod->GetPackageFile(), wxS("properties"));
			});
		}
	}
}
