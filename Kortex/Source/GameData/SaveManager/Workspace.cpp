#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/PluginManager.hpp>
#include <Kortex/SaveManager.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include "UI/KImageViewerDialog.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxTextFile.h>
#include <wx/clipbrd.h>

namespace Kortex::SaveManager
{
	Workspace::Workspace(KMainWindow* mainWindow)
		:KWorkspace(mainWindow), m_Manager(ISaveManager::GetInstance())
		//m_SavesListViewOptions(this, "SavesListView"), m_FileFiltersOptions(this, "FileFilters")
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
	}
	Workspace::~Workspace()
	{
		if (IsWorkspaceCreated())
		{
			//KProgramOptionSerializer::SaveDataViewLayout(m_DisplayModel->GetView(), m_SavesListViewOptions);

			for (const auto& v: ISaveManager::GetInstance()->GetConfig().GetFileFilters())
			{
				//m_FileFiltersOptions.SetAttribute(v.GetValue(), FiltersMenu_IsFilterActive(v.GetValue()));
			}
		}
	}
	bool Workspace::OnCreateWorkspace()
	{
		CreateViewPane();
		m_MainSizer->Add(m_DisplayModel->GetView(), 1, wxEXPAND);

		// Load options
		//KProgramOptionSerializer::LoadDataViewLayout(m_DisplayModel->GetView(), m_SavesListViewOptions);
		m_DisplayModel->UpdateRowHeight();
		m_ActiveFilters.clear();

		KxStringVector filters;
		for (const auto& v: m_Manager->GetConfig().GetFileFilters())
		{
			const wxString& value = v.GetValue();
			#if 0
			if (m_FileFiltersOptions.GetAttributeBool(value, true))
			{
				m_ActiveFilters.insert(value);
				filters.push_back(value);
			}
			#endif
		}

		ReloadWorkspace();
		return true;
	}

	void Workspace::CreateViewPane()
	{
		m_DisplayModel = new DisplayModel(m_Manager, this);
		m_DisplayModel->Create(this);
		m_DisplayModel->SetDataVector();
	}
	void Workspace::CreateContextMenu(KxMenu& menu, const IGameSave* save)
	{
		IPluginManager* pluginManager = IPluginManager::GetInstance();
		PluginManager::Workspace* pluginWorkspace = PluginManager::Workspace::GetInstance();

		const size_t selectedItemCount = m_DisplayModel->GetView()->GetSelectedItemsCount();
		const bool isMultiSelect = selectedItemCount > 1;

		const IBethesdaGameSave* bethesdaSave = save->QueryInterface<IBethesdaGameSave>();
		const bool hasPlugins = bethesdaSave && bethesdaSave->HasPlugins();

		// Plugins list
		if (pluginManager && pluginWorkspace && bethesdaSave)
		{
			const KxStringVector pluginsList = bethesdaSave->GetPlugins();

			KxMenu* pluginsMenu = new KxMenu();
			KxMenuItem* pluginsMenuItem = menu.Add(pluginsMenu, wxString::Format("%s (%zu)", KTr("SaveManager.Tab.PluginsList"), pluginsList.size()));
			pluginsMenuItem->Enable(!pluginsList.empty());

			if (!pluginManager->HasPlugins())
			{
				pluginManager->Load();
			}

			for (const wxString& name: pluginsList)
			{
				KxMenuItem* item = pluginsMenu->Add(new KxMenuItem(name));
				item->SetBitmap(KGetBitmap(pluginWorkspace->GetStatusImageForPlugin(pluginManager->FindPluginByName(name))));
			}
		}

		// Basic info
		{
			KxMenu* basicInfoMenu = new KxMenu();
			KxMenuItem* basicInfoMenuItem = menu.Add(basicInfoMenu, KTr("SaveManager.Tab.BasicInfo"));
			basicInfoMenuItem->Enable(false);

			if (save)
			{
				const KLabeledValue::Vector basicInfo = save->GetBasicInfo();
				basicInfoMenuItem->Enable(!basicInfo.empty());

				for (const KLabeledValue& entry: basicInfo)
				{
					KxMenuItem* item = basicInfoMenu->Add(new KxMenuItem(KxString::Format("%1: %2", entry.GetLabel(), entry.GetValue())));
				}

				basicInfoMenu->Bind(KxEVT_MENU_SELECT, [](KxMenuEvent& event)
				{
					if (wxTheClipboard->Open())
					{
						KxMenuItem* item = event.GetItem();
						wxTheClipboard->SetData(new wxTextDataObject(item->GetItemLabelText()));
						wxTheClipboard->Close();
					}
				});
			}
		}
		menu.AddSeparator();

		// Sync
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(KxID_APPLY, KTr("SaveManager.SyncPluginsList")));
			item->SetBitmap(KGetBitmap(KIMG_PLUG_DISCONNECT));
			item->Enable(!isMultiSelect && save && hasPlugins && pluginManager);
		}

		// Save
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(KxID_SAVE, KTr("SaveManager.SavePluginsList")));
			item->SetBitmap(KGetBitmap(KIMG_DISK));
			item->Enable(!isMultiSelect && save && hasPlugins);
		}
		menu.AddSeparator();

		// File filter
		{
			KxMenu* filtersMenu = new KxMenu();
			menu.Add(filtersMenu, KTr("FileFilter"));

			// All files
			KxMenuItem* allFilesItem = filtersMenu->Add(new KxMenuItem(KTr("FileFilter.AllFiles"), wxEmptyString, wxITEM_CHECK));
			allFilesItem->Check(FiltersMenu_IsAllFiltersActive());
			allFilesItem->Bind(KxEVT_MENU_SELECT, &Workspace::FiltersMenu_AllFiles, this);

			filtersMenu->AddSeparator();

			// Specific
			for (const KLabeledValue& filter: m_Manager->GetConfig().GetFileFilters())
			{
				KxMenuItem* item = filtersMenu->Add(new KxMenuItem(filter.GetLabel(), wxEmptyString, wxITEM_CHECK));
				item->Bind(KxEVT_MENU_SELECT, &Workspace::FiltersMenu_SpecificFilter, this);
				item->SetClientData(const_cast<KLabeledValue*>(&filter));
				item->Check(FiltersMenu_IsFilterActive(filter.GetValue()));
			}
		}
		menu.AddSeparator();

		// Reload data
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(KxID_REFRESH, KTr(KxID_REFRESH)));
			item->SetBitmap(KGetBitmap(KIMG_ARROW_CIRCLE_DOUBLE));
		}
		// Remove
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(KxID_REMOVE, KTr(KxID_REMOVE)));
			item->Enable(selectedItemCount != 0);
		}
	}

	bool Workspace::FiltersMenu_IsAllFiltersActive() const
	{
		return m_ActiveFilters.size() == m_Manager->GetConfig().GetFileFilters().size();
	}
	void Workspace::FiltersMenu_AllFiles(KxMenuEvent& event)
	{
		if (FiltersMenu_IsAllFiltersActive())
		{
			m_ActiveFilters.clear();
		}
		else
		{
			for (const KLabeledValue& v: m_Manager->GetConfig().GetFileFilters())
			{
				m_ActiveFilters.insert(v.GetValue());
			}
		}

		LoadData();
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

		LoadData();
	}

	void Workspace::OnSyncPluginsList(const IBethesdaGameSave* saveEntry)
	{
		if (IPluginManager* manager = IPluginManager::GetInstance())
		{
			manager->SyncWithPluginsList(saveEntry->GetPlugins(), PluginManager::SyncListMode::ActivateAll);
			manager->Save();
			KWorkspace::ScheduleReloadOf<PluginManager::Workspace>();
		}
	}
	void Workspace::OnSavePluginsList(const IBethesdaGameSave* saveEntry)
	{
		KxFileBrowseDialog dialog(GetMainWindow(), KxID_NONE, KxFBD_SAVE);
		dialog.SetDefaultExtension("txt");
		dialog.SetFileName(saveEntry->GetFileItem().GetName().BeforeLast('.'));
		dialog.AddFilter("*.txt", KTr("FileFilter.Text"));
		dialog.AddFilter("*", KTr("FileFilter.AllFiles"));

		if (dialog.ShowModal() == KxID_OK)
		{
			KxTextFile::WriteToFile(dialog.GetResult(), saveEntry->GetPlugins());
		}
	}
	void Workspace::OnRemoveSave(IGameSave* saveEntry)
	{
		if (saveEntry)
		{
			const Config& config = m_Manager->GetConfig();
			const KxFileItem& primaryInfo = saveEntry->GetFileItem();

			KxFile(primaryInfo.GetFullPath()).RemoveFile(true);
			if (config.HasMultiFileSaveConfig())
			{
				KxFileItem secondaryInfo = primaryInfo;
				secondaryInfo.SetName(primaryInfo.GetName().BeforeLast('.') + '.' + config.GetSecondarySaveExtension());
				KxFile(secondaryInfo.GetFullPath()).RemoveFile(true);
			}
		}
	}

	bool Workspace::OnOpenWorkspace()
	{
		return true;
	}
	bool Workspace::OnCloseWorkspace()
	{
		KMainWindow::GetInstance()->ClearStatus(1);
		return true;
	}
	void Workspace::OnReloadWorkspace()
	{
		LoadData();
	}

	wxString Workspace::GetID() const
	{
		return "KSaveManagerWorkspace";
	}
	wxString Workspace::GetName() const
	{
		return KTr("SaveManager.Name");
	}
	wxString Workspace::GetNameShort() const
	{
		return KTr("SaveManager.NameShort");
	}

	void Workspace::LoadData()
	{
		KxStringVector filterList;
		for (const wxString& filter: m_ActiveFilters)
		{
			filterList.push_back(filter);
		}
		m_DisplayModel->SetDataVector(m_Manager->GetConfig().GetLocation(), filterList);
	}

	void Workspace::ProcessSelection(const IGameSave* saveEntry)
	{
		const int statusIndex = 1;
		KMainWindow* mainWindow = KMainWindow::GetInstance();
		mainWindow->ClearStatus(statusIndex);

		if (saveEntry)
		{
			if (saveEntry->IsOK())
			{
				mainWindow->SetStatus(saveEntry->GetFileItem().GetName(), statusIndex);
			}
			else
			{
				mainWindow->SetStatus(KTr("SaveManager.InvalidFile"), statusIndex, KIMG_CROSS_CIRCLE_FRAME);
			}
		}
	}
	void Workspace::ProcessContextMenu(const IGameSave* saveEntry)
	{
		KxMenu menu;
		CreateContextMenu(menu, saveEntry);

		switch (menu.Show(this))
		{
			case KxID_APPLY:
			{
				OnSyncPluginsList(saveEntry->QueryInterface<IBethesdaGameSave>());
				break;
			}
			case KxID_SAVE:
			{
				OnSavePluginsList(saveEntry->QueryInterface<IBethesdaGameSave>());
				break;
			}
			case KxID_REFRESH:
			{
				ReloadWorkspace();
				break;
			}
			case KxID_REMOVE:
			{
				KxDataViewItem::Vector items;
				m_DisplayModel->GetView()->GetSelections(items);
				if (!items.empty())
				{
					for (KxDataViewItem& item: items)
					{
						OnRemoveSave(m_DisplayModel->GetDataEntry(m_DisplayModel->GetRow(item)));
					}
					LoadData();
				}
				break;
			}
		};
	}
}
