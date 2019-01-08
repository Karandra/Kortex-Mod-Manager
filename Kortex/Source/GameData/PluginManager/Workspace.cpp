#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/PluginManager.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>
#include <Kortex/Events.hpp>
#include "UI/KImageViewerDialog.h"
#include "Utility/KAux.h"
#include "Utility/KOperationWithProgress.h"
#include <KxFramework/KxSearchBox.h>
#include <KxFramework/KxNotebook.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxShell.h>

namespace
{
	using namespace Kortex;
	using namespace Kortex::Application;
	using namespace Kortex::PluginManager;

	auto GetDisplayModelOptions()
	{
		return Application::GetAInstanceOptionOf<IPluginManager>(OName::Workspace, OName::DisplayModel);
	}

	namespace MenuCounter
	{
		enum PluginType: uint32_t
		{
			Normal = 0,
			Master = 1 << 0,
			Light = 1 << 1,

			Total = ~PluginType() - 1,
			Active = ~PluginType()
		};
		bool CheckType(const IBethesdaGamePlugin& plugin, uint32_t type)
		{
			if (type == PluginType::Active)
			{
				return true;
			}

			bool ok = true;
			if (type & PluginType::Normal)
			{
				ok = ok && plugin.IsNormal();
			}
			if (type & PluginType::Master)
			{
				ok = ok && plugin.IsMaster();
			}
			if (type & PluginType::Light)
			{
				ok = ok && plugin.IsLight();
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
}

namespace Kortex::PluginManager
{
	KImageEnum Workspace::GetStatusImageForPlugin(const IGamePlugin* plugin)
	{
		if (plugin)
		{
			return plugin->IsActive() ? KIMG_TICK_CIRCLE_FRAME_EMPTY : KIMG_INFORMATION_FRAME_EMPTY;
		}
		else
		{
			return KIMG_CROSS_CIRCLE_FRAME_EMPTY;
		}
	}

	Workspace::Workspace(KMainWindow* mainWindow)
		:KWorkspace(mainWindow)
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
	}
	Workspace::~Workspace()
	{
		if (IsWorkspaceCreated())
		{
			IGameInstance::GetActive()->GetActiveProfile()->SyncWithCurrentState();
			GetDisplayModelOptions().SaveDataViewLayout(m_ModelView->GetView());
		}
	}
	bool Workspace::OnCreateWorkspace()
	{
		CreateModelView();
		m_MainSizer->Add(m_ModelView->GetView(), 1, wxEXPAND);

		m_SearchBox = new KxSearchBox(this, KxID_NONE);
		m_SearchBox->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &Workspace::OnModSerach, this);
		m_SearchBox->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &Workspace::OnModSerach, this);
		m_MainSizer->Add(m_SearchBox, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);

		GetDisplayModelOptions().LoadDataViewLayout(m_ModelView->GetView());
		return true;
	}

	void Workspace::CreateModelView()
	{
		m_ModelView = new PluginViewModel();
		m_ModelView->Create(this);
		m_ModelView->SetDataVector(IPluginManager::GetInstance()->GetPlugins());
	}

	bool Workspace::OnOpenWorkspace()
	{
		if (IsFirstTimeOpen())
		{
			IPluginManager::GetInstance()->Load();
			m_ModelView->RefreshItems();
		}
		return true;
	}
	bool Workspace::OnCloseWorkspace()
	{
		KMainWindow::GetInstance()->ClearStatus(1);
		return true;
	}
	void Workspace::OnReloadWorkspace()
	{
		m_ModelView->RefreshItems();
		ProcessSelection();
	}

	void Workspace::OnModSerach(wxCommandEvent& event)
	{
		if (m_ModelView->SetSearchMask(event.GetEventType() == wxEVT_SEARCHCTRL_SEARCH_BTN ? event.GetString() : wxEmptyString))
		{
			m_ModelView->RefreshItems();
		}
	}

	wxString Workspace::GetID() const
	{
		return "KPluginManagerWorkspace";
	}
	wxString Workspace::GetName() const
	{
		return KTr("PluginManager.Name");
	}
	wxString Workspace::GetNameShort() const
	{
		return KTr("PluginManager.NameShort");
	}

	void Workspace::UpdatePluginTypeCounter(KxMenuItem* item)
	{
		using namespace MenuCounter;

		const Config& pluginsConfig = IPluginManager::GetInstance()->GetConfig();
		CounterData* clientData = static_cast<CounterData*>(item->GetClientObject());
		if (clientData)
		{
			size_t count = 0;
			if (clientData->GetType() == PluginType::Total)
			{
				count = IPluginManager::GetInstance()->GetPlugins().size();
			}
			else
			{
				for (const auto& plugin: IPluginManager::GetInstance()->GetPlugins())
				{
					if (plugin->IsActive())
					{
						bool typeOK = false;
						const IBethesdaGamePlugin* bethesdaPlugin = nullptr;
						if (plugin->QueryInterface(bethesdaPlugin))
						{
							typeOK = CheckType(*bethesdaPlugin, clientData->GetType());
						}

						if (typeOK || clientData->GetType() == PluginType::Active)
						{
							count++;
						}
					}
				}
			}

			if (clientData->GetType() == PluginType::Total)
			{
				item->SetItemLabel(KxString::Format("%1: %2", KTr("PluginManager.PluginCounter.Total"), count));
			}
			else if (clientData->GetType() == PluginType::Active)
			{
				if (pluginsConfig.HasPluginLimit())
				{
					item->SetItemLabel(KxString::Format("%1: %2/%3", KTr("PluginManager.PluginCounter.Active"), count, pluginsConfig.GetPluginLimit()));
					item->SetBitmap(KGetBitmap(count >= static_cast<size_t>(pluginsConfig.GetPluginLimit()) ? KIMG_EXCLAMATION : KIMG_TICK_CIRCLE_FRAME));
				}
			}
			else
			{
				wxString label;
				if (clientData->GetType() == PluginType::Active)
				{
					label = KTr("Generic.All");
				}
				else if (BethesdaPluginManager* bethesda = IPluginManager::GetInstance()->QueryInterface<BethesdaPluginManager>())
				{
					label = bethesda->GetPluginTypeName(clientData->GetType() & PluginType::Master, clientData->GetType() & PluginType::Light);
				}
				item->SetItemLabel(KxString::Format("%1: %2", label, count));
			}
		}
	}
	void Workspace::OnRunLootAPI(KxMenuEvent& event)
	{
		auto operation = new KOperationWithProgressDialog<KxFileOperationEvent>(true, this);
		operation->OnRun([operation](KOperationWithProgressBase* self)
		{
			KxStringVector sortedList;
			if (LibLoot::GetInstance()->SortPlugins(sortedList, operation))
			{
				IPluginManager::GetInstance()->SyncWithPluginsList(sortedList, SyncListMode::DoNotChange);
				IPluginManager::GetInstance()->Save();
			}
		});
		operation->OnEnd([this](KOperationWithProgressBase* self)
		{
			PluginEvent(Events::PluginsReordered).Send();
		});
		operation->SetDialogCaption(event.GetItem()->GetItemLabelText());
		operation->Run();
	}

	void Workspace::OnCreateViewContextMenu(KxMenu& menu, const IGamePlugin* plugin)
	{
		using namespace MenuCounter;

		// Plugin type counter
		auto AddCounter = [this, &menu](uint32_t type)
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(wxEmptyString));
			item->SetClientObject(new MenuCounter::CounterData(type));
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

		AddCounter(PluginType::Total);
		AddCounter(PluginType::Active);

		if (dynamic_cast<BethesdaPluginManager*>(IPluginManager::GetInstance()))
		{
			AddCounter(PluginType::Normal);
			AddCounter(PluginType::Master);
		}
		if (dynamic_cast<BethesdaPluginManager2*>(IPluginManager::GetInstance()))
		{
			AddCounter(PluginType::Light);
		}
	}
	void Workspace::OnCreateSortingToolsMenu(KxMenu& menu, const IGamePlugin* plugin)
	{
		KxMenu* sortingMenu = nullptr;
		const Config& pluginsConfig = IPluginManager::GetInstance()->GetConfig();
		if (pluginsConfig.HasSortingTools() || LibLoot::HasInstance())
		{
			sortingMenu = new KxMenu();
			menu.AddSeparator();

			KxMenuItem* item = menu.Add(sortingMenu, KTr("PluginManager.Sorting"));
			item->Enable(IModManager::GetInstance()->GetVFS().IsEnabled());
		}

		if (sortingMenu && IModManager::GetInstance()->GetVFS().IsEnabled())
		{
			// LibLoot
			bool hasLoot = false;
			if (LibLoot::GetInstance() != nullptr)
			{
				hasLoot = true;

				KxMenuItem* item = sortingMenu->Add(new KxMenuItem("LOOT API"));
				item->Bind(KxEVT_MENU_SELECT, &Workspace::OnRunLootAPI, this);
			}

			// Sorting tools
			{
				const auto& sortingTools = pluginsConfig.GetSortingTools();
				if (hasLoot && !sortingTools.empty())
				{
					sortingMenu->AddSeparator();
				}

				for (const SortingToolEntry& entry: sortingTools)
				{
					KxMenuItem* item = sortingMenu->Add(new KxMenuItem(entry.GetName()));
					item->SetBitmap(KxShell::GetFileIcon(entry.GetExecutable(), true));
					item->Bind(KxEVT_MENU_SELECT, [this, entry](KxMenuEvent& event)
					{
						IPluginManager::GetInstance()->Save();
						IPluginManager::GetInstance()->RunSortingTool(entry);
					});
				}
			}
		}
	}
	void Workspace::OnCreateImportExportMenu(KxMenu& menu, const IGamePlugin* plugin)
	{
		menu.AddSeparator();

		{
			KxMenuItem* item = menu.Add(new KxMenuItem(KTr("PluginManager.Tools.ImportList")));
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_OPEN);
				dialog.AddFilter("*.txt", KTr("FileFilter.Text"));
				dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
				if (dialog.ShowModal() == KxID_OK)
				{
					IPluginManager::GetInstance()->SyncWithPluginsList(KxTextFile::ReadToArray(dialog.GetResult()));
				}
			});
		}
		{
			KxMenuItem* itemAll = menu.Add(new KxMenuItem(KTr("PluginManager.Tools.ExportList")));
			KxMenuItem* itemActive = menu.Add(new KxMenuItem(KTr("PluginManager.Tools.ExportListActive")));

			auto Event = [this, itemActive](KxMenuEvent& event)
			{
				KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_SAVE);
				dialog.AddFilter("*.txt", KTr("FileFilter.Text"));
				dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
				dialog.SetDefaultExtension("txt");

				if (dialog.ShowModal() == KxID_OK)
				{
					bool activeOnly = event.GetId() == itemActive->GetId();
					KxTextFile::WriteToFile(dialog.GetResult(), IPluginManager::GetInstance()->GetPluginsList(activeOnly));
				}
			};
			itemAll->Bind(KxEVT_MENU_SELECT, Event);
			itemActive->Bind(KxEVT_MENU_SELECT, Event);
		}
	}

	void Workspace::ProcessSelection(const IGamePlugin* plugin)
	{
		const int statusIndex = 1;
		KMainWindow* mainWindow = KMainWindow::GetInstance();
		mainWindow->ClearStatus(statusIndex);

		if (plugin)
		{
			if (plugin->IsOK())
			{
				mainWindow->SetStatus(plugin->GetName(), statusIndex);
			}
			else
			{
				mainWindow->SetStatus(wxEmptyString, statusIndex, KIMG_CROSS_CIRCLE_FRAME);
			}
		}
	}
	void Workspace::HighlightPlugin(const IGamePlugin* plugin)
	{
		if (plugin)
		{
			KxDataViewItem item = m_ModelView->GetItemByEntry(plugin);
			m_ModelView->GetView()->Select(item);
			m_ModelView->GetView()->EnsureVisible(item);
		}
		else
		{
			m_ModelView->GetView()->UnselectAll();
		}
	}
}
