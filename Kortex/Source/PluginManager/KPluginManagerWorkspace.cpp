#include "stdafx.h"
#include "KPluginManagerWorkspace.h"
#include "KPluginManagerBethesdaGeneric.h"
#include "KPluginManagerListModel.h"
#include "KPMPluginReader.h"
#include "LOOT API/KLootAPI.h"
#include "UI/KImageViewerDialog.h"
#include "Profile/KPluginManagerConfig.h"
#include "Profile/KConfigManagerConfig.h"
#include "RunManager/KRunManager.h"
#include "ModManager/KModManager.h"
#include "ModManager/KModEntry.h"
#include "ModManager/KModManagerWorkspace.h"
#include "KThemeManager.h"
#include "KApp.h"
#include "KAux.h"
#include "KOperationWithProgress.h"
#include <KxFramework/KxSearchBox.h>
#include <KxFramework/KxNotebook.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxFile.h>

KxSingletonPtr_Define(KPluginManagerWorkspace);

KImageEnum KPluginManagerWorkspace::GetStatusImageForPlugin(const KPMPluginEntry* pluginEntry)
{
	if (pluginEntry)
	{
		return pluginEntry->IsEnabled() ? KIMG_TICK_CIRCLE_FRAME_EMPTY : KIMG_INFORMATION_FRAME_EMPTY;
	}
	else
	{
		return KIMG_CROSS_CIRCLE_FRAME_EMPTY;
	}
}

KPluginManagerWorkspace::KPluginManagerWorkspace(KMainWindow* mainWindow, KPluginManagerBethesdaGeneric* manager)
	:KWorkspace(mainWindow), m_Manager(manager), m_PluginListViewOptions(this, "PluginListView")
{
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
}
KPluginManagerWorkspace::~KPluginManagerWorkspace()
{
	if (IsWorkspaceCreated())
	{
		KModManager::GetListManager().SyncCurrentList();
		KProgramOptionSerializer::SaveDataViewLayout(m_ModelView->GetView(), m_PluginListViewOptions);
	}
}
bool KPluginManagerWorkspace::OnCreateWorkspace()
{
	CreateModelView();
	m_MainSizer->Add(m_ModelView->GetView(), 1, wxEXPAND);
	
	m_SearchBox = new KxSearchBox(this, KxID_NONE);
	m_SearchBox->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &KPluginManagerWorkspace::OnModSerach, this);
	m_SearchBox->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &KPluginManagerWorkspace::OnModSerach, this);
	m_MainSizer->Add(m_SearchBox, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);

	KProgramOptionSerializer::LoadDataViewLayout(m_ModelView->GetView(), m_PluginListViewOptions);
	ReloadWorkspace();
	return true;
}

void KPluginManagerWorkspace::CreateModelView()
{
	m_ModelView = new KPluginManagerListModel(this);
	m_ModelView->Create(this);
	m_ModelView->SetDataVector(m_Manager->GetEntries());
}

bool KPluginManagerWorkspace::OnOpenWorkspace()
{
	return true;
}
bool KPluginManagerWorkspace::OnCloseWorkspace()
{
	KMainWindow::GetInstance()->ClearStatus(1);
	return true;
}
void KPluginManagerWorkspace::OnReloadWorkspace()
{
	m_Manager->Load();
	m_ModelView->RefreshItems();
	ProcessSelection(NULL);
}

void KPluginManagerWorkspace::OnModSerach(wxCommandEvent& event)
{
	if (m_ModelView->SetSearchMask(event.GetEventType() == wxEVT_SEARCHCTRL_SEARCH_BTN ? event.GetString() : wxEmptyString))
	{
		m_ModelView->RefreshItems();
	}
}

wxString KPluginManagerWorkspace::GetID() const
{
	return "KPluginManagerWorkspace";
}
wxString KPluginManagerWorkspace::GetName() const
{
	return T("ToolBar.PluginManager");
}
wxString KPluginManagerWorkspace::GetNameShort() const
{
	return T("ToolBar.PluginManagerShort");
}

void KPluginManagerWorkspace::UpdatePluginTypeCounter(KxMenuItem* item)
{
	const KPluginManagerConfig* pluginsConfig = KPluginManagerConfig::GetInstance();

	auto type = (KPMPluginEntryType)(size_t)item->GetClientData();
	if (type != KPMPE_TYPE_INVALID)
	{
		int count = 0;
		for (const auto& entry: m_Manager->GetEntries())
		{
			if (entry->IsEnabled() && (entry->GetFormat() == type || type == KPMPE_TYPE_ALL))
			{
				count++;
			}
		}

		if (type == KPMPE_TYPE_ALL && pluginsConfig->HasPluginLimit())
		{
			item->SetItemLabel(wxString::Format("%s: %d/%d", T("Generic.All"), count, pluginsConfig->GetPluginLimit()));
			item->SetBitmap(KGetBitmap(count >= pluginsConfig->GetPluginLimit() ? KIMG_EXCLAMATION : KIMG_TICK_CIRCLE_FRAME));
		}
		else
		{
			wxString label = type == KPMPE_TYPE_ALL ? T("Generic.All") : m_Manager->GetPluginTypeName(type);
			item->SetItemLabel(wxString::Format("%s: %d", label, count));
		}
	}
}

void KPluginManagerWorkspace::OnCreateViewContextMenu(KxMenu& menu, const KPMPluginEntry* entry)
{
	/* Plugin type counter */
	auto AddCounter = [this, &menu](KPMPluginEntryType type)
	{
		if (m_Manager->IsEntryTypeSupported(type) || type == KPMPE_TYPE_ALL)
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(wxEmptyString));
			item->SetClientData(reinterpret_cast<void*>(type));
			item->Enable(false);
		}
	};
	menu.Bind(KxEVT_MENU_OPEN, [this](KxMenuEvent& event)
	{
		for (const auto& item: event.GetMenu()->GetMenuItems())
		{
			UpdatePluginTypeCounter(static_cast<KxMenuItem*>(item));
		}
		event.Skip();
	});

	AddCounter(KPMPE_TYPE_ALL);
	AddCounter(KPMPE_TYPE_NORMAL);
	AddCounter(KPMPE_TYPE_MASTER);
	AddCounter(KPMPE_TYPE_NORMAL|KPMPE_TYPE_LIGHT);
	AddCounter(KPMPE_TYPE_MASTER|KPMPE_TYPE_LIGHT);
}
void KPluginManagerWorkspace::OnCreateSortingToolsMenu(KxMenu& menu, const KPMPluginEntry* entry)
{
	KxMenu* sortingMenu = NULL;
	const KPluginManagerConfig* pOptions = KPluginManagerConfig::GetInstance();
	if (pOptions->HasSortingTools() || pOptions->HasLootAPI())
	{
		sortingMenu = new KxMenu();
		menu.AddSeparator();

		KxMenuItem* item = menu.Add(sortingMenu, T("PluginManager.Sorting"));
		item->Enable(KModManager::Get().IsVFSMounted());
	}

	if (sortingMenu && KModManager::Get().IsVFSMounted())
	{
		// LootAPI
		{
			KxMenuItem* item = sortingMenu->Add(new KxMenuItem("LOOT API"));
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				auto pOperation = new KOperationWithProgressDialog<KxFileOperationEvent>(true, this);
				pOperation->OnRun([pOperation](KOperationWithProgressBase* self)
				{
					KxStringVector sortedList;
					if (KLootAPI::GetInstance()->SortPlugins(sortedList, pOperation))
					{
						KPluginManager::GetInstance()->SyncWithPluginsList(sortedList, KPluginManager::DoNotChange);
						KPluginManager::GetInstance()->Save();
					}
				});
				pOperation->OnEnd([this](KOperationWithProgressBase* self)
				{
					ReloadWorkspace();
				});
				pOperation->SetDialogCaption(event.GetItem()->GetItemLabelText());
				pOperation->Run();
			});
		}

		// Sorting tools
		{
			for (const KPluginManagerConfigSortingToolEntry& entry: pOptions->GetSortingTools())
			{
				if (pOptions->HasLootAPI())
				{
					sortingMenu->AddSeparator();
				}

				KxMenuItem* item = sortingMenu->Add(new KxMenuItem(entry.GetName()));
				item->SetBitmap(KAux::ExtractIconFromBinaryFile(entry.GetExecutable()));
				item->Bind(KxEVT_MENU_SELECT, [this, entry](KxMenuEvent& event)
				{
					m_Manager->Save();
					m_Manager->RunSortingTool(entry);
				});
			}
		}
	}
}
void KPluginManagerWorkspace::OnCreateImportExportMenu(KxMenu& menu, const KPMPluginEntry* entry)
{
	menu.AddSeparator();

	{
		KxMenuItem* item = menu.Add(new KxMenuItem(T("PluginManager.Tools.ImportList")));
		item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_OPEN);
			dialog.AddFilter("*.txt", T("FileFilter.Text"));
			dialog.AddFilter("*", T("FileFilter.AllFiles"));
			if (dialog.ShowModal() == KxID_OK)
			{
				m_Manager->SyncWithPluginsList(KxTextFile::ReadToArray(dialog.GetResult()));
			}
		});
	}
	{
		KxMenuItem* pItemAll = menu.Add(new KxMenuItem(T("PluginManager.Tools.ExportList")));
		KxMenuItem* pItemActive = menu.Add(new KxMenuItem(T("PluginManager.Tools.ExportListActive")));

		auto Event = [this, pItemActive](KxMenuEvent& event)
		{
			KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_SAVE);
			dialog.AddFilter("*.txt", T("FileFilter.Text"));
			dialog.AddFilter("*", T("FileFilter.AllFiles"));
			dialog.SetDefaultExtension("txt");

			if (dialog.ShowModal() == KxID_OK)
			{
				bool bActiveOnly = event.GetId() == pItemActive->GetId();
				KxTextFile::WriteToFile(dialog.GetResult(), m_Manager->GetPluginsList(bActiveOnly));
			}
		};
		pItemAll->Bind(KxEVT_MENU_SELECT, Event);
		pItemActive->Bind(KxEVT_MENU_SELECT, Event);
	}
}

void KPluginManagerWorkspace::ProcessSelection(const KPMPluginEntry* entry)
{
	const int statusIndex = 1;
	KMainWindow* mainWindow = KMainWindow::GetInstance();
	mainWindow->ClearStatus(statusIndex);

	if (entry)
	{
		if (entry->IsOK())
		{
			mainWindow->SetStatus(entry->GetName(), statusIndex);
		}
		else
		{
			mainWindow->SetStatus(wxEmptyString, statusIndex, KIMG_CROSS_CIRCLE_FRAME);
		}
	}
}
void KPluginManagerWorkspace::HighlightPlugin(const KPMPluginEntry* entry)
{
	if (entry)
	{
		KxDataViewItem item = m_ModelView->GetItemByEntry(entry);
		m_ModelView->GetView()->Select(item);
		m_ModelView->GetView()->EnsureVisible(item);
	}
	else
	{
		m_ModelView->GetView()->UnselectAll();
	}
}
