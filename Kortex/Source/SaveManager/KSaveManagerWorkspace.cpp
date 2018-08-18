#include "stdafx.h"
#include "KSaveManagerWorkspace.h"
#include "KSaveManager.h"
#include "KSaveManagerListModel.h"
#include "KSMSaveFile.h"
#include "UI/KImageViewerDialog.h"
#include "Profile/KProfile.h"
#include "PluginManager/KPluginManager.h"
#include "PluginManager/KPluginManagerWorkspace.h"
#include "ModManager/KModManager.h"
#include "KThemeManager.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxTextFile.h>
#include <wx/clipbrd.h>

KxSingletonPtr_Define(KSaveManagerWorkspace);

KSaveManagerWorkspace::KSaveManagerWorkspace(KMainWindow* mainWindow, KSaveManager* manager)
	:KWorkspace(mainWindow), m_Manager(manager),
	m_SavesListViewOptions(this, "SavesListView"), m_FileFiltersOptions(this, "FileFilters")
{
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
}
KSaveManagerWorkspace::~KSaveManagerWorkspace()
{
	if (IsWorkspaceCreated())
	{
		KProgramOptionSerializer::SaveDataViewLayout(m_ViewModel->GetView(), m_SavesListViewOptions);

		for (const auto& v: m_Manager->GetProfileSaveManager()->GetFileFilters())
		{
			m_FileFiltersOptions.SetAttribute(v.GetValue(), FiltersMenu_IsFilterActive(v.GetValue()));
		}
	}
}
bool KSaveManagerWorkspace::OnCreateWorkspace()
{
	CreateViewPane();
	m_MainSizer->Add(m_ViewModel->GetView(), 1, wxEXPAND);

	// Load options
	KProgramOptionSerializer::LoadDataViewLayout(m_ViewModel->GetView(), m_SavesListViewOptions);

	const KSaveManagerConfig* pProfileConfig = m_Manager->GetProfileSaveManager();
	m_ActiveFilters.clear();

	KxStringVector tFilters;
	for (const auto& v: pProfileConfig->GetFileFilters())
	{
		const wxString& value = v.GetValue();
		if (m_FileFiltersOptions.GetAttributeBool(value, true))
		{
			m_ActiveFilters.insert(value);
			tFilters.push_back(value);
		}
	}

	ReloadWorkspace();
	return true;
}

void KSaveManagerWorkspace::CreateViewPane()
{
	m_ViewModel = new KSaveManagerListModel(m_Manager, this);
	m_ViewModel->Create(this);
	m_ViewModel->SetDataVector();
}
void KSaveManagerWorkspace::CreateContextMenu(KxMenu& menu, const KSMSaveFile* saveEntry)
{
	KPluginManager* pPluginManager = KPluginManager::GetInstance();
	KPluginManagerWorkspace* pPluginWorkspace = KPluginManagerWorkspace::GetInstance();
	bool bMultiSelect = m_ViewModel->GetView()->GetSelectedItemsCount() > 1;

	// Plugins list
	if (pPluginManager && pPluginWorkspace && saveEntry)
	{
		const KxStringVector& list = saveEntry->GetPluginsList();

		KxMenu* pPluginsListMenu = new KxMenu();
		KxMenuItem* pPluginsListMenuItem = menu.Add(pPluginsListMenu, wxString::Format("%s (%zu)", T("SaveManager.Tab.PluginsList"), list.size()));
		pPluginsListMenuItem->Enable(!list.empty());

		for (const wxString& name: list)
		{
			KxMenuItem* item = pPluginsListMenu->Add(new KxMenuItem(name));
			item->SetBitmap(KGetBitmap(pPluginWorkspace->GetStatusImageForPlugin(pPluginManager->FindPluginByName(name))));
		}
	}

	// Basic info
	{
		KxMenu* pBasicInfoMenu = new KxMenu();
		KxMenuItem* pBasicInfoMenuItem = menu.Add(pBasicInfoMenu, T("SaveManager.Tab.BasicInfo"));
		pBasicInfoMenuItem->Enable(saveEntry && !saveEntry->GetBasicInfo().empty());

		if (saveEntry)
		{
			for (const KLabeledValue& entry: saveEntry->GetBasicInfo())
			{
				KxMenuItem* item = pBasicInfoMenu->Add(new KxMenuItem(wxString::Format("%s: %s", entry.GetLabel(), entry.GetValue())));
			}

			pBasicInfoMenu->Bind(KxEVT_MENU_SELECT, [](KxMenuEvent& event)
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
		KxMenuItem* item = menu.Add(new KxMenuItem(KxID_APPLY, T("SaveManager.SyncPluginsList")));
		item->SetBitmap(KGetBitmap(KIMG_PLUG_DISCONNECT));
		item->Enable(!bMultiSelect && saveEntry && saveEntry->HasPluginsList() && pPluginManager && pPluginManager->IsOK());
	}

	// Save
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(KxID_SAVE, T("SaveManager.SavePluginsList")));
		item->SetBitmap(KGetBitmap(KIMG_DISK));
		item->Enable(!bMultiSelect && saveEntry && saveEntry->HasPluginsList());
	}
	menu.AddSeparator();

	// File filter
	{
		const KSaveManagerConfig* pProfileConfig = m_Manager->GetProfileSaveManager();

		KxMenu* pFiltersMenu = new KxMenu();
		menu.Add(pFiltersMenu, T("FileFilter"));

		// All files
		KxMenuItem* pAllFilesItem = pFiltersMenu->Add(new KxMenuItem(T("FileFilter.AllFiles"), wxEmptyString, wxITEM_CHECK));
		pAllFilesItem->Check(FiltersMenu_IsAllFiltersActive());
		pAllFilesItem->Bind(KxEVT_MENU_SELECT, &KSaveManagerWorkspace::FiltersMenu_AllFiles, this);

		pFiltersMenu->AddSeparator();

		// Specific
		for (const KLabeledValue& v: pProfileConfig->GetFileFilters())
		{
			KxMenuItem* item = pFiltersMenu->Add(new KxMenuItem(v.GetLabel(), wxEmptyString, wxITEM_CHECK));
			item->Bind(KxEVT_MENU_SELECT, &KSaveManagerWorkspace::FiltersMenu_SpecificFilter, this);
			item->SetClientData(const_cast<KLabeledValue*>(&v));
			item->Check(FiltersMenu_IsFilterActive(v.GetValue()));
		}
	}
	menu.AddSeparator();

	// Reload data
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(KxID_REFRESH, T(KxID_REFRESH)));
		item->SetBitmap(KGetBitmap(KIMG_ARROW_CIRCLE_DOUBLE));
	}
	// Remove
	{
		menu.Add(new KxMenuItem(KxID_REMOVE, T(KxID_REMOVE)));
	}
}

bool KSaveManagerWorkspace::FiltersMenu_IsAllFiltersActive() const
{
	return m_ActiveFilters.size() == m_Manager->GetProfileSaveManager()->GetFileFilters().size();
}
void KSaveManagerWorkspace::FiltersMenu_AllFiles(KxMenuEvent& event)
{
	if (FiltersMenu_IsAllFiltersActive())
	{
		m_ActiveFilters.clear();
	}
	else
	{
		for (const auto& v: m_Manager->GetProfileSaveManager()->GetFileFilters())
		{
			m_ActiveFilters.insert(v.GetValue());
		}
	}

	LoadData();
}
void KSaveManagerWorkspace::FiltersMenu_SpecificFilter(KxMenuEvent& event)
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

void KSaveManagerWorkspace::OnSyncPluginsList(const KSMSaveFile* saveEntry)
{
	if (KPluginManager* pPM = KPluginManager::GetInstance())
	{
		pPM->SyncWithPluginsList(saveEntry->GetPluginsList());
		pPM->Save();

		if (KWorkspace* workspace = pPM->GetWorkspace())
		{
			workspace->ScheduleRefresh();
		}
	}
}
void KSaveManagerWorkspace::OnSavePluginsList(const KSMSaveFile* saveEntry)
{
	KxFileBrowseDialog dialog(GetMainWindow(), KxID_NONE, KxFBD_SAVE);
	dialog.SetDefaultExtension("txt");
	dialog.SetFileName(saveEntry->GetFileInfo().GetName().BeforeLast('.'));
	dialog.AddFilter("*.txt", T("FileFilter.Text"));
	dialog.AddFilter("*", T("FileFilter.AllFiles"));

	if (dialog.ShowModal() == KxID_OK)
	{
		KxTextFile::WriteToFile(dialog.GetResult(), saveEntry->GetPluginsList());
	}
}
void KSaveManagerWorkspace::OnRemoveSave(KSMSaveFile* saveEntry)
{
	if (saveEntry)
	{
		const KSaveManagerConfig* pConfig = m_Manager->GetProfileSaveManager();
		const KxFileFinderItem& tPrimaryInfo = saveEntry->GetFileInfo();

		KxFile(tPrimaryInfo.GetFullPath()).RemoveFile(true);
		if (pConfig->HasMultiFileSaveConfig())
		{
			KxFileFinderItem tSecondaryInfo = tPrimaryInfo;
			tSecondaryInfo.SetName(tPrimaryInfo.GetName().BeforeLast('.') + '.' + pConfig->GetSecondarySaveExtension());
			KxFile(tSecondaryInfo.GetFullPath()).RemoveFile(true);
		}
	}
}

bool KSaveManagerWorkspace::OnOpenWorkspace()
{
	if (KPluginManager* pPM = KPluginManager::GetInstance())
	{
		pPM->LoadIfNeeded();
	}
	return true;
}
bool KSaveManagerWorkspace::OnCloseWorkspace()
{
	KMainWindow::GetInstance()->ClearStatus(1);
	return true;
}
void KSaveManagerWorkspace::OnReloadWorkspace()
{
	LoadData();
}

wxString KSaveManagerWorkspace::GetID() const
{
	return "KSaveManagerWorkspace";
}
wxString KSaveManagerWorkspace::GetName() const
{
	return T("ToolBar.SaveManager");
}
wxString KSaveManagerWorkspace::GetNameShort() const
{
	return T("ToolBar.SaveManagerShort");
}

void KSaveManagerWorkspace::LoadData()
{
	KxStringVector tFilters;
	for (const wxString& v: m_ActiveFilters)
	{
		tFilters.push_back(v);
	}
	m_ViewModel->SetDataVector(m_Manager->GetProfileSaveManager()->GetSavesFolder(), tFilters);
}

void KSaveManagerWorkspace::ProcessSelection(const KSMSaveFile* saveEntry)
{
	const int statusIndex = 1;
	KMainWindow* mainWindow = KMainWindow::GetInstance();
	mainWindow->ClearStatus(statusIndex);

	if (saveEntry)
	{
		if (saveEntry->IsOK())
		{
			mainWindow->SetStatus(saveEntry->GetFileInfo().GetName(), statusIndex);
		}
		else
		{
			mainWindow->SetStatus(T("SaveManager.InvalidFile"), statusIndex, KIMG_CROSS_CIRCLE_FRAME);
		}
	}
}
void KSaveManagerWorkspace::ProcessContextMenu(const KSMSaveFile* saveEntry)
{
	KxMenu menu;
	CreateContextMenu(menu, saveEntry);

	switch (menu.Show(this))
	{
		case KxID_APPLY:
		{
			OnSyncPluginsList(saveEntry);
			break;
		}
		case KxID_SAVE:
		{
			OnSavePluginsList(saveEntry);
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
			m_ViewModel->GetView()->GetSelections(items);
			if (!items.empty())
			{
				for (KxDataViewItem& item: items)
				{
					OnRemoveSave(m_ViewModel->GetDataEntry(m_ViewModel->GetRow(item)));
				}
				LoadData();
			}
			break;
		}
	};
}
