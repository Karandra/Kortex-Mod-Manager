#include "stdafx.h"
#include "KPluginManagerWorkspace.h"
#include "KPluginManagerBethesda.h"
#include "KPluginManagerBethesda2.h"
#include "KPluginViewBaseModel.h"
#include "LOOT API/KLootAPI.h"
#include "UI/KImageViewerDialog.h"
#include "Profile/KPluginManagerConfig.h"
#include "Profile/KConfigManagerConfig.h"
#include "ProgramManager/KProgramManager.h"
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

enum PluginType: uint32_t
{
	Normal = 0,
	Master = 1 << 0,
	Light = 1 << 1,

	AllTypes = ~PluginType()
};
namespace MenuCounterNS
{
	bool CheckType(const KPluginEntryBethesda& pluginEntry, uint32_t type)
	{
		if (type == PluginType::AllTypes)
		{
			return true;
		}

		bool ok = true;
		if (type & PluginType::Normal)
		{
			ok = ok && pluginEntry.IsNormal();
		}
		if (type & PluginType::Master)
		{
			ok = ok && pluginEntry.IsMaster();
		}
		if (type & PluginType::Light)
		{
			ok = ok && pluginEntry.IsLight();
		}
		return ok;
	}

	class CounterData: public wxClientData
	{
		private:
			uint32_t m_Type = PluginType::Normal;

		public:
			CounterData(uint32_t type)
				:m_Type(type)
			{
			}

		public:
			uint32_t GetType() const
			{
				return m_Type;
			}
	};
}

KImageEnum KPluginManagerWorkspace::GetStatusImageForPlugin(const KPluginEntry* pluginEntry)
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

KPluginManagerWorkspace::KPluginManagerWorkspace(KMainWindow* mainWindow)
	:KWorkspace(mainWindow), m_PluginListViewOptions(this, "PluginListView")
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
	m_ModelView = new KPluginViewBaseModel();
	m_ModelView->Create(this);
	m_ModelView->SetDataVector(KPluginManager::GetInstance()->GetEntries());
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
	KPluginManager::GetInstance()->Load();
	m_ModelView->RefreshItems();
	ProcessSelection();
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
	return T("PluginManager.Name");
}
wxString KPluginManagerWorkspace::GetNameShort() const
{
	return T("PluginManager.NameShort");
}

void KPluginManagerWorkspace::UpdatePluginTypeCounter(KxMenuItem* item)
{
	const KPluginManagerConfig* pluginsConfig = KPluginManagerConfig::GetInstance();
	MenuCounterNS::CounterData* clientData = static_cast<MenuCounterNS::CounterData*>(item->GetClientObject());
	if (clientData)
	{
		int count = 0;
		for (const auto& entry: KPluginManager::GetInstance()->GetEntries())
		{
			if (entry->IsEnabled())
			{
				bool typeOK = false;
				const KPluginEntryBethesda* bethesdaPlugin = NULL;
				if (entry->As(bethesdaPlugin))
				{
					typeOK = MenuCounterNS::CheckType(*bethesdaPlugin, clientData->GetType());
				}

				if (typeOK || clientData->GetType() == PluginType::AllTypes)
				{
					count++;
				}
			}
		}

		if (clientData->GetType() == PluginType::AllTypes && pluginsConfig->HasPluginLimit())
		{
			item->SetItemLabel(wxString::Format("%s: %d/%d", T("Generic.All"), count, pluginsConfig->GetPluginLimit()));
			item->SetBitmap(KGetBitmap(count >= pluginsConfig->GetPluginLimit() ? KIMG_EXCLAMATION : KIMG_TICK_CIRCLE_FRAME));
		}
		else
		{
			wxString label;
			if (clientData->GetType() == PluginType::AllTypes)
			{
				label = T("Generic.All");
			}
			else if (KPluginManagerBethesda* bethesda = dynamic_cast<KPluginManagerBethesda*>(KPluginManager::GetInstance()))
			{
				label = bethesda->GetPluginTypeName(clientData->GetType() & PluginType::Master, clientData->GetType() & PluginType::Light);
			}

			item->SetItemLabel(wxString::Format("%s: %d", label, count));
		}
	}
}

void KPluginManagerWorkspace::OnCreateViewContextMenu(KxMenu& menu, const KPluginEntry* entry)
{
	/* Plugin type counter */
	auto AddCounter = [this, &menu](uint32_t type)
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(wxEmptyString));
		item->SetClientObject(new MenuCounterNS::CounterData(type));
		item->Enable(false);
	};
	menu.Bind(KxEVT_MENU_OPEN, [this](KxMenuEvent& event)
	{
		for (const auto& item: event.GetMenu()->GetMenuItems())
		{
			UpdatePluginTypeCounter(static_cast<KxMenuItem*>(item));
		}
		event.Skip();
	});

	AddCounter(PluginType::AllTypes);

	if (dynamic_cast<KPluginManagerBethesda*>(KPluginManager::GetInstance()))
	{
		AddCounter(PluginType::Normal);
		AddCounter(PluginType::Master);
	}
	if (dynamic_cast<KPluginManagerBethesda2*>(KPluginManager::GetInstance()))
	{
		AddCounter(PluginType::Light);
	}
}
void KPluginManagerWorkspace::OnCreateSortingToolsMenu(KxMenu& menu, const KPluginEntry* entry)
{
	KxMenu* sortingMenu = NULL;
	const KPluginManagerConfig* options = KPluginManagerConfig::GetInstance();
	if (options->HasSortingTools() || options->HasLootAPI())
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
				auto operation = new KOperationWithProgressDialog<KxFileOperationEvent>(true, this);
				operation->OnRun([operation](KOperationWithProgressBase* self)
				{
					KxStringVector sortedList;
					if (KLootAPI::GetInstance()->SortPlugins(sortedList, operation))
					{
						KPluginManager::GetInstance()->SyncWithPluginsList(sortedList, KPluginManager::SyncListMode::DoNotChange);
						KPluginManager::GetInstance()->Save();
					}
				});
				operation->OnEnd([this](KOperationWithProgressBase* self)
				{
					ReloadWorkspace();
				});
				operation->SetDialogCaption(event.GetItem()->GetItemLabelText());
				operation->Run();
			});
		}

		// Sorting tools
		{
			for (const KPluginManagerConfigSortingToolEntry& entry: options->GetSortingTools())
			{
				if (options->HasLootAPI())
				{
					sortingMenu->AddSeparator();
				}

				KxMenuItem* item = sortingMenu->Add(new KxMenuItem(entry.GetName()));
				item->SetBitmap(KAux::ExtractIconFromBinaryFile(entry.GetExecutable()));
				item->Bind(KxEVT_MENU_SELECT, [this, entry](KxMenuEvent& event)
				{
					KPluginManager::GetInstance()->Save();
					KPluginManager::GetInstance()->RunSortingTool(entry);
				});
			}
		}
	}
}
void KPluginManagerWorkspace::OnCreateImportExportMenu(KxMenu& menu, const KPluginEntry* entry)
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
				KPluginManager::GetInstance()->SyncWithPluginsList(KxTextFile::ReadToArray(dialog.GetResult()));
			}
		});
	}
	{
		KxMenuItem* itemAll = menu.Add(new KxMenuItem(T("PluginManager.Tools.ExportList")));
		KxMenuItem* itemActive = menu.Add(new KxMenuItem(T("PluginManager.Tools.ExportListActive")));

		auto Event = [this, itemActive](KxMenuEvent& event)
		{
			KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_SAVE);
			dialog.AddFilter("*.txt", T("FileFilter.Text"));
			dialog.AddFilter("*", T("FileFilter.AllFiles"));
			dialog.SetDefaultExtension("txt");

			if (dialog.ShowModal() == KxID_OK)
			{
				bool activeOnly = event.GetId() == itemActive->GetId();
				KxTextFile::WriteToFile(dialog.GetResult(), KPluginManager::GetInstance()->GetPluginsList(activeOnly));
			}
		};
		itemAll->Bind(KxEVT_MENU_SELECT, Event);
		itemActive->Bind(KxEVT_MENU_SELECT, Event);
	}
}

void KPluginManagerWorkspace::ProcessSelection(const KPluginEntry* entry)
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
void KPluginManagerWorkspace::HighlightPlugin(const KPluginEntry* entry)
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
