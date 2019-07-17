#include "stdafx.h"
#include "DisplayModel.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/InstallWizard.hpp>
#include <Kortex/NetworkManager.hpp>
#include "Network/ModNetwork/Nexus.h"
#include "Utility/KAux.h"
#include "Utility/MenuSeparator.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxComparator.h>
#include <wx/clipbrd.h>

namespace Kortex::DownloadManager
{
	void DisplayModel::OnContextMenu(Event& event)
	{
		DownloadItem* download = GetItem(event.GetNode());
		const IModNetwork* modNetwork = download ? download->GetModNetwork() : nullptr;

		const bool isRunning = download && download->IsRunning();
		const bool isPaused = download && download->IsPaused();
		const bool isFailed = download && download->IsFailed();
		const bool isCompleted = download && download->IsCompleted();
		const bool isEmpty = m_Nodes.empty();

		KxMenu contextMenu;
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr("DownloadManager.Menu.Install")));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Box));
			item->Enable(isCompleted);
			item->Bind(KxEVT_MENU_SELECT, [this, download](KxMenuEvent& event)
			{
				Install(*download);
			});
		}
		contextMenu.AddSeparator();

		if constexpr (false)
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr("DownloadManager.Menu.Add")));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::PlusSmall));
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr("DownloadManager.Menu.Pause")));
			item->Enable(isRunning);
			item->Bind(KxEVT_MENU_SELECT, [download](KxMenuEvent& event)
			{
				download->Pause();
			});
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr("DownloadManager.Menu.Abort")));
			item->Enable(isRunning || isPaused);
			item->Bind(KxEVT_MENU_SELECT, [download](KxMenuEvent& event)
			{
				download->Stop();
			});
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(isPaused ? KTr("DownloadManager.Menu.Resume") : KTr("DownloadManager.Menu.Start")));
			item->Enable(download && (download->CanStart() || download->CanResume()));
			item->Bind(KxEVT_MENU_SELECT, [download, isPaused](KxMenuEvent& event)
			{
				if (isPaused)
				{
					download->Resume();
				}
				else
				{
					download->Start();
				}
			});
		}

		contextMenu.AddSeparator();
		{
			KxMenu* providerMenu = new KxMenu();
			if (download && !isRunning)
			{
				for (ModNetworkRepository* repository: INetworkManager::GetInstance()->GetModRepositories())
				{
					KxMenuItem* item = providerMenu->Add(new KxMenuItem(repository->GetContainer().GetName(), wxEmptyString, wxITEM_RADIO));
					item->Check(repository == download->GetModRepository());
					item->Enable(!item->IsChecked());
					item->Bind(KxEVT_MENU_SELECT, [download, repository](KxMenuEvent& event)
					{
						download->SetModRepository(*repository);
						download->Save();
					});
				}
			}

			KxMenuItem* item = contextMenu.Add(providerMenu, KTr("DownloadManager.Menu.SetSource"));
			item->Enable(providerMenu->GetMenuItemCount() != 0);
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr("DownloadManager.Menu.QueryInfo")));
			item->Enable(download && download->CanQueryInfo());
			item->Bind(KxEVT_MENU_SELECT, [download](KxMenuEvent& event)
			{
				if (!download->QueryInfo())
				{
					wxString message = KTrf("DownloadManager.Notification.QueryDownloadInfoFailed", download->GetName());
					INotificationCenter::NotifyUsing<IDownloadManager>(message, KxICON_WARNING);
				}
			});
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr("DownloadManager.Menu.ShowChangeLog")));
			item->Enable(download && download->HasChangeLog());
			item->Bind(KxEVT_MENU_SELECT, [this, download](KxMenuEvent& event)
			{
				KxTaskDialog dialog(GetView(), KxID_NONE, download->GetDisplayName(), wxEmptyString, KxBTN_OK, KxICON_NONE);
				dialog.SetMessage(KxString::Format("%1 %2", KTr("Generic.Version"), download->GetVersion()));
				dialog.SetExMessage(download->GetChangeLog());
				dialog.SetMainIcon(KxShell::GetFileIcon(download->GetFullPath()));
				dialog.SetOptionEnabled(KxTD_EXMESSAGE_EXPANDED);
				dialog.ShowModal();
			});
		}
		contextMenu.AddSeparator();

		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr("DownloadManager.Menu.Remove")));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::MinusSmall));
			item->Enable(download && !isEmpty && !isRunning);
			item->Bind(KxEVT_MENU_SELECT, [this, download](KxMenuEvent& event)
			{
				if (m_DownloadManager.RemoveDownload(*download))
				{
					RefreshItems();
				}
			});
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr("DownloadManager.Menu.RemoveAll")));
			item->Enable(!isEmpty);
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				RemoveAll();
			});
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr("DownloadManager.Menu.RemoveInstalled")));
			item->Enable(!isEmpty);
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				RemoveAll(true);
			});
		}
		contextMenu.AddSeparator();

		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr("DownloadManager.Menu.Hide")));
			item->Enable(download && !isEmpty && !isRunning);
			item->Bind(KxEVT_MENU_SELECT, [this, download](KxMenuEvent& event)
			{
				download->SetHidden(true);
				download->Save();
				RefreshItems();
			});
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr("DownloadManager.Menu.HideAll")));
			item->Enable(!isEmpty);
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				SetAllHidden(true);
			});
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr("DownloadManager.Menu.HideInstalled")));
			item->Enable(!isEmpty);
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				SetAllHidden(true, true);
			});
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr("DownloadManager.Menu.ShowHidden"), wxEmptyString, wxITEM_CHECK));
			item->Check(m_DownloadManager.ShouldShowHiddenDownloads());
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				m_DownloadManager.ToggleHiddenDownloads();
				RefreshItems();
			});
		}
		contextMenu.AddSeparator();
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr("MainMenu.OpenLocation")));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderOpen));
			item->Bind(KxEVT_MENU_SELECT, [this, download](KxMenuEvent& event)
			{
				if (download)
				{
					KxShell::OpenFolderAndSelectItem(download->GetFullPath());
				}
				else
				{
					KxShell::Execute(GetView(), m_DownloadManager.GetDownloadsLocation(), "open");
				}
			});
		}
		if (modNetwork)
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTrf("DownloadManager.Menu.VisitOnWebSite", modNetwork->GetName())));
			item->Enable(download && download->CanVisitSource());
			item->SetBitmap(ImageProvider::GetBitmap(modNetwork->GetIcon()));
			item->Bind(KxEVT_MENU_SELECT, [this, download](KxMenuEvent& event)
			{
				KxShell::Execute(GetView(), download->GetModNetwork()->GetModPageURI(download->GetNetworkModInfo()).BuildUnescapedURI(), "open");
			});
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr(KxID_REFRESH)));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::ArrowCircleDouble));
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				m_DownloadManager.LoadDownloads();
				RefreshItems();
			});
		}

		if (Utility::MenuSeparatorBefore separator(contextMenu); true)
		{
			for (ModNetworkRepository* repository: INetworkManager::GetInstance()->GetModRepositories())
			{
				repository->OnDownloadMenu(contextMenu, download);
			}
		}

		contextMenu.Show(GetView());
	}

	void DisplayModel::RemoveAll(bool installedOnly)
	{
		KxTaskDialog dialog(GetView(), KxID_NONE, KTr("DownloadManager.RemoveDownloadsCaption"), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
		dialog.SetMessage(installedOnly ? KTr("DownloadManager.RemoveInstalledDownloadsMessage") : KTr("DownloadManager.RemoveDownloadsMessage"));

		if (dialog.ShowModal() == KxID_YES)
		{
			DownloadItem::RefVector items = IDownloadManager::GetInstance()->GetInactiveDownloads(installedOnly);
			if (!items.empty())
			{
				wxWindowUpdateLocker lock(GetView());
				for (DownloadItem* entry: items)
				{
					IDownloadManager::GetInstance()->RemoveDownload(*entry);
				}
				RefreshItems();
			}
		}
	}
	void DisplayModel::SetAllHidden(bool isHidden, bool installedOnly)
	{
		DownloadItem::RefVector items = IDownloadManager::GetInstance()->GetInactiveDownloads(installedOnly);
		if (!items.empty())
		{
			wxWindowUpdateLocker lock(GetView());
			for (DownloadItem* item: items)
			{
				item->SetHidden(isHidden);
				item->Save();
			}
			RefreshItems();
		}
	}
	void DisplayModel::Install(DownloadItem& item)
	{
		item.Save();
		new InstallWizard::WizardDialog(GetView(), item.GetFullPath());
	}

	DisplayModel::DisplayModel()
		:m_DownloadManager(*IDownloadManager::GetInstance())
	{
	}

	void DisplayModel::CreateView(wxWindow* parent)
	{
		using namespace KxDataView2;
		using ColumnID = DisplayModelNode::ColumnID;

		View* view = new View(parent, KxID_NONE, CtrlStyle::VerticalRules|CtrlStyle::CellFocus|CtrlStyle::FitLastColumn);
		view->AssignModel(this);
		view->SetUniformRowHeight(view->GetDefaultRowHeight(UniformHeight::Explorer));

		// Events
		view->Bind(KxDataView2::EVENT_ITEM_CONTEXT_MENU, &DisplayModel::OnContextMenu, this);
		view->Bind(KxDataView2::EVENT_COLUMN_HEADER_RCLICK, [this](Event& event)
		{
			KxMenu menu;
			if (GetView()->CreateColumnSelectionMenu(menu))
			{
				GetView()->OnColumnSelectionMenu(menu);
			}
		});

		// Columns
		const ColumnStyle columnStyle = ColumnStyle::Sort|ColumnStyle::Move|ColumnStyle::Size;
		view->AppendColumn<BitmapTextRenderer>(KTr("Generic.Name"), ColumnID::Name, {}, columnStyle);
		view->AppendColumn<TextRenderer>(KTr("Generic.Version"), ColumnID::Version, {}, columnStyle);
		{
			auto [column, renderer] = view->AppendColumn<TextRenderer>(KTr("Generic.Size"), ColumnID::Size, {}, columnStyle);
			renderer.SetAlignment(wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT);
		}
		view->AppendColumn<TextRenderer>(KTr("Generic.Game"), ColumnID::Game, {}, columnStyle);
		view->AppendColumn<TextRenderer>(KTr("NetworkManager.ModNetwork"), ColumnID::ModNetwork, {}, columnStyle);
		{
			auto [column, renderer] = view->AppendColumn<TextRenderer>(KTr("Generic.Date"), ColumnID::Date, {}, columnStyle);
			column.SortDescending();
		}
		{
			auto [column, renderer] = view->AppendColumn<ProgressRenderer>(KTr("Generic.Status"), ColumnID::Status, {}, columnStyle);
			renderer.SetAlignment(wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL);
		}

		// Add items
		RefreshItems();
	}

	DisplayModelNode& DisplayModel::OnDonwloadAdded(DownloadItem& item)
	{
		DisplayModelNode& node = m_Nodes.emplace_back(item);
		item.SetDisplayNode(&node);

		GetView()->GetRootNode().AttachChild(node);
		node.OnAttachNode();
		ItemsChanged();

		return node;
	}
	bool DisplayModel::OnDonwloadRemoved(DownloadItem& item)
	{
		auto it = std::find_if(m_Nodes.begin(), m_Nodes.end(), [&item](DisplayModelNode& node)
		{
			return &node.m_Item == &item;
		});
		if (it != m_Nodes.end())
		{
			item.SetDisplayNode(nullptr);
			GetView()->GetRootNode().DetachChild(*it);

			m_Nodes.erase(it);
			ItemsChanged();
			return true;
		}
		return false;
	}
	void DisplayModel::RefreshItems()
	{
		Node& root = GetView()->GetRootNode();

		root.DetachAllChildren();
		m_Nodes.clear();

		for (auto& item: m_DownloadManager.GetDownloads())
		{
			OnDonwloadAdded(*item);
		}
		ItemsChanged();
	}
}
