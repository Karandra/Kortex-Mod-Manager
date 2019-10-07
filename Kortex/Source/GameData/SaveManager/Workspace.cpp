#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/PluginManager.hpp>
#include <Kortex/SaveManager.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include "GameMods/ModManager/Workspace.h"
#include "UI/ImageViewerDialog.h"
#include "Utility/KAux.h"
#include "Utility/Common.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxCrypto.h>
#include <KxFramework/KxShell.h>

namespace Kortex::Application::OName
{
	KortexDefOption(FileFilters);
}

namespace
{
	using namespace Kortex;
	using namespace Kortex::Application;

	wxString FilterNameToSignature(const KLabeledValue& filter)
	{
		wxStringInputStream stream(filter.GetValue());
		return wxS("Filter-") + KxCrypto::CRC32(stream);
	}

	auto GetDisplayModelOptions()
	{
		return GetAInstanceOptionOf<ISaveManager>(OName::Workspace, OName::DisplayModel);
	}
	auto GetFiltersOptions()
	{
		return GetAInstanceOptionOf<ISaveManager>(OName::Workspace, OName::FileFilters);
	}
	auto GetFilterOption(const KLabeledValue& filter)
	{
		return GetAInstanceOptionOf<ISaveManager>(OName::Workspace, OName::FileFilters, FilterNameToSignature(filter));
	}
}

namespace Kortex::SaveManager
{
	bool Workspace::OnCreateWorkspace()
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(m_MainSizer);

		CreateViewPane();
		m_MainSizer->Add(m_DisplayModel->GetView(), 1, wxEXPAND);

		// Load options
		GetDisplayModelOptions().LoadDataViewLayout(m_DisplayModel->GetView());
		m_ActiveFilters.clear();

		// Load filters
		auto filterOptions = GetFiltersOptions();
		for (const auto& filter: ISaveManager::GetInstance()->GetConfig().GetFileFilters())
		{
			if (GetFilterOption(filter).GetAttributeBool(OName::Enabled, true))
			{
				m_ActiveFilters.insert(filter.GetValue());
			}
		}

		ScheduleReload();
		return true;
	}
	bool Workspace::OnOpenWorkspace()
	{
		if (!OpenedOnce())
		{
			OnReloadWorkspace();
		}
		return true;
	}
	bool Workspace::OnCloseWorkspace()
	{
		IMainWindow::GetInstance()->ClearStatus(1);
		return true;
	}
	void Workspace::OnReloadWorkspace()
	{
		UpdateFilters();
	}

	Workspace::~Workspace()
	{
		if (IsCreated())
		{
			GetDisplayModelOptions().SaveDataViewLayout(m_DisplayModel->GetView());

			KxXMLNode filterNode = GetFiltersOptions().GetNode();
			filterNode.ClearNode();

			for (const auto& filter: ISaveManager::GetInstance()->GetConfig().GetFileFilters())
			{
				KxXMLNode node = filterNode.NewElement(FilterNameToSignature(filter));
				node.SetAttribute(OName::Enabled, FiltersMenu_IsFilterActive(filter.GetValue()));
			}
		}
	}

	void Workspace::CreateViewPane()
	{
		m_DisplayModel = new DisplayModel();
		m_DisplayModel->CreateView(this);
	}
	void Workspace::UpdateFilters()
	{
		KxStringVector filterList;
		for (const wxString& filter: m_ActiveFilters)
		{
			filterList.push_back(filter);
		}
		ISaveManager::GetInstance()->UpdateActiveFilters(filterList);
	}

	bool Workspace::FiltersMenu_IsAllFiltersActive() const
	{
		return m_ActiveFilters.size() == ISaveManager::GetInstance()->GetConfig().GetFileFilters().size();
	}
	void Workspace::FiltersMenu_AllFiles(KxMenuEvent& event)
	{
		if (FiltersMenu_IsAllFiltersActive())
		{
			m_ActiveFilters.clear();
		}
		else
		{
			for (const KLabeledValue& filter: ISaveManager::GetInstance()->GetConfig().GetFileFilters())
			{
				m_ActiveFilters.insert(filter.GetValue());
			}
		}
		ScheduleReload();
	}
	void Workspace::FiltersMenu_SpecificFilter(KxMenuEvent& event)
	{
		// Add or remove this filter
		const KxMenuItem* item = event.GetItem();
		const KLabeledValue* pFilter = static_cast<const KLabeledValue*>(item->GetClientData());
		if (item->IsChecked())
		{
			m_ActiveFilters.insert(pFilter->GetValue());
		}
		else
		{
			m_ActiveFilters.erase(pFilter->GetValue());
		}
		ScheduleReload();
	}

	void Workspace::OnSyncPluginsList(const IBethesdaGameSave& save)
	{
		if (IPluginManager* manager = IPluginManager::GetInstance())
		{
			manager->SyncWithPluginsList(save.GetPlugins(), PluginManager::SyncListMode::ActivateAll);
			manager->Save();
			manager->ScheduleWorkspacesReload();
		}
	}
	void Workspace::OnSavePluginsList(const IBethesdaGameSave& save)
	{
		KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_SAVE);
		dialog.SetDefaultExtension("txt");
		dialog.SetFileName(save.GetFileItem().GetName().BeforeLast('.'));
		dialog.AddFilter("*.txt", KTr("FileFilter.Text"));
		dialog.AddFilter("*", KTr("FileFilter.AllFiles"));

		if (dialog.ShowModal() == KxID_OK)
		{
			KxTextFile::WriteToFile(dialog.GetResult(), save.GetPlugins());
		}
	}
	bool Workspace::OnRemoveSave(IGameSave& save)
	{
		if (BroadcastProcessor::Get().ProcessEventEx(SaveEvent::EvtRemoving, save).Do().IsAllowed())
		{
			const Config& config = ISaveManager::GetInstance()->GetConfig();
			const KxFileItem& primaryInfo = save.GetFileItem();

			KxFile(primaryInfo.GetFullPath()).RemoveFile(true);
			if (config.HasMultiFileSaveConfig())
			{
				KxFileItem secondaryInfo = primaryInfo;
				secondaryInfo.SetName(primaryInfo.GetName().BeforeLast('.') + '.' + config.GetSecondarySaveExtension());
				KxFile(secondaryInfo.GetFullPath()).RemoveFile(true);
			}

			BroadcastProcessor::Get().ProcessEvent(SaveEvent::EvtRemoved);
			return true;
		}
		return false;
	}

	wxString Workspace::GetID() const
	{
		return "SaveManager::Workspace";
	}
	wxString Workspace::GetName() const
	{
		return KTr("SaveManager.NameShort");
	}
	IWorkspaceContainer* Workspace::GetPreferredContainer() const
	{
		IWorkspaceContainer* result = nullptr;
		IWorkspace::CallIfCreated<ModManager::Workspace>([&](ModManager::Workspace& workspace)
		{
			result = &workspace.GetWorkspaceContainer();
		});
		return result;
	}

	void Workspace::OnSelection(const IGameSave* save)
	{
		const int statusIndex = 1;
		IMainWindow* mainWindow = IMainWindow::GetInstance();
		mainWindow->ClearStatus(statusIndex);

		if (save)
		{
			if (save->IsOK())
			{
				mainWindow->SetStatus(save->GetDisplayName(), statusIndex);
			}
			else
			{
				mainWindow->SetStatus(KTr("SaveManager.InvalidFile"), statusIndex, ImageResourceID::CrossCircleFrame);
			}
		}
	}
	void Workspace::OnContextMenu(const IGameSave* save)
	{
		KxMenu menu;

		IPluginManager* pluginManager = IPluginManager::GetInstance();
		PluginManager::Workspace* pluginWorkspace = PluginManager::Workspace::GetInstance();

		const bool isMultiSelect = m_DisplayModel->GetView()->GetSelectedCount() > 1;
		const IBethesdaGameSave* bethesdaSave = save ? save->QueryInterface<IBethesdaGameSave>() : nullptr;
		const bool hasPlugins = bethesdaSave && bethesdaSave->HasPlugins();

		// Plugins list
		if (pluginManager && pluginWorkspace && bethesdaSave)
		{
			const KxStringVector pluginsList = bethesdaSave->GetPlugins();

			KxMenu* pluginsMenu = new KxMenu();
			KxMenuItem* pluginsMenuItem = menu.Add(pluginsMenu, KxString::Format(wxS("%1 (%2)"), KTr("SaveManager.Tab.PluginsList"), pluginsList.size()));
			pluginsMenuItem->Enable(!pluginsList.empty());

			if (!pluginManager->HasPlugins())
			{
				pluginManager->Load();
			}

			for (const wxString& name: pluginsList)
			{
				KxMenuItem* item = pluginsMenu->AddItem(name);
				item->SetBitmap(ImageProvider::GetBitmap(pluginWorkspace->GetStatusImageForPlugin(pluginManager->FindPluginByName(name))));
			}
		}

		// Basic info
		{
			KxMenu* basicInfoMenu = new KxMenu();
			KxMenuItem* basicInfoMenuItem = menu.Add(basicInfoMenu, KTr("SaveManager.Tab.BasicInfo"));
			basicInfoMenuItem->Enable(false);

			if (save)
			{
				const auto& basicInfo = save->GetBasicInfo();
				basicInfoMenuItem->Enable(!basicInfo.empty());

				for (const KLabeledValue& entry: basicInfo)
				{
					KxMenuItem* item = basicInfoMenu->AddItem(KxString::Format(wxS("%1: %2"), entry.GetLabel(), entry.GetValue()));
				}
				basicInfoMenu->Bind(KxEVT_MENU_SELECT, [](KxMenuEvent& event)
				{
					Utility::CopyTextToClipboard(event.GetItem()->GetItemLabelText());
				});
			}
		}
		menu.AddSeparator();

		// Sync
		{
			KxMenuItem* item = menu.AddItem(KTr("SaveManager.SyncPluginsList"));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::PlugDisconnect));
			item->Enable(!isMultiSelect && bethesdaSave && hasPlugins && pluginManager);
			item->Bind(KxEVT_MENU_SELECT, [this, bethesdaSave](KxMenuEvent& event)
			{
				OnSyncPluginsList(*bethesdaSave);
			});
		}

		// Save
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(KxID_SAVE, KTr("SaveManager.SavePluginsList")));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Disk));
			item->Enable(!isMultiSelect && bethesdaSave && hasPlugins);
			item->Bind(KxEVT_MENU_SELECT, [this, bethesdaSave](KxMenuEvent& event)
			{
				OnSavePluginsList(*bethesdaSave);
			});
		}
		menu.AddSeparator();

		// File filter
		{
			KxMenu* filtersMenu = new KxMenu();
			menu.Add(filtersMenu, KTr("FileFilter"));

			// All files
			{
				KxMenuItem* item = filtersMenu->Add(new KxMenuItem(KTr("FileFilter.AllFiles"), wxEmptyString, wxITEM_CHECK));
				item->Check(FiltersMenu_IsAllFiltersActive());
				item->Bind(KxEVT_MENU_SELECT, &Workspace::FiltersMenu_AllFiles, this);
			}

			filtersMenu->AddSeparator();

			// Specific
			for (const KLabeledValue& filter: ISaveManager::GetInstance()->GetConfig().GetFileFilters())
			{
				KxMenuItem* item = filtersMenu->Add(new KxMenuItem(filter.GetLabel(), wxEmptyString, wxITEM_CHECK));
				item->Bind(KxEVT_MENU_SELECT, &Workspace::FiltersMenu_SpecificFilter, this);
				item->SetClientData(const_cast<KLabeledValue*>(&filter));
				item->Check(FiltersMenu_IsFilterActive(filter.GetValue()));
			}
		}
		menu.AddSeparator();

		// Open location
		{
			KxMenuItem* item = menu.AddItem(save ? KTr("Generic.FileLocation") : KTr("Generic.FolderLocation"));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderOpen));
			item->Bind(KxEVT_MENU_SELECT, [this, save](KxMenuEvent& event)
			{
				if (save)
				{
					KxShell::OpenFolderAndSelectItem(save->GetFileItem().GetFullPath());
				}
				else
				{
					KxShell::Execute(this, ISaveManager::GetInstance()->GetConfig().GetLocation(), wxS("open"));
				}
			});
		}

		// Reload items
		{
			KxMenuItem* item = menu.AddItem(KTr(KxID_REFRESH));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::ArrowCircleDouble));
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				ScheduleReload();
			});
		}

		// Remove
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(KxID_REMOVE, KTr(KxID_REMOVE)));
			item->Enable(save && !IModManager::GetInstance()->GetFileSystem().IsEnabled());
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				for (KxDataView2::Node* node: m_DisplayModel->GetView()->GetSelections())
				{
					OnRemoveSave(m_DisplayModel->GetItem(*node));
				}
				ScheduleReload();
			});
		}

		menu.Show(m_DisplayModel->GetView());
	}
}
