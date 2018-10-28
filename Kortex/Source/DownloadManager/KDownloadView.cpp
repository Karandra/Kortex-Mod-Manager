#include "stdafx.h"
#include "KDownloadView.h"
#include "Network/KNetwork.h"
#include "Network/KNetworkProviderNexus.h"
#include "Network/KNetworkProviderTESALL.h"
#include "Network/KNetworkProviderLoversLab.h"
#include "InstallWizard/KInstallWizardDialog.h"
#include "NotificationCenter/KNotificationCenter.h"
#include "GameInstance/KGameInstance.h"
#include "KApp.h"
#include "KAux.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxComparator.h>
#include <wx/clipbrd.h>

enum ColumnID
{
	Name,
	Version,
	Size,
	Game,
	Provider,
	Date,
	Status,
};
enum MenuID
{
	Install,

	Add,
	Pause,
	Abort,
	Resume,
	Restart,

	QueryInfo,
	ShowChangeLog,

	Remove,
	RemoveAll,
	RemoveInstalled,

	Hide,
	HideAll,
	HideInstalled,
	ShowHidden,

	OpenLocation,
	Refresh,

	AssociateWithNXM,
	CopyNXM,
};

void KDownloadView::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KDownloadView::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KDownloadView::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KDownloadView::OnContextMenu, this);
	GetView()->Bind(KxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, [this](KxDataViewEvent& event)
	{
		KxMenu menu;
		if (GetView()->CreateColumnSelectionMenu(menu))
		{
			GetView()->OnColumnSelectionMenu(menu);
		}
	});

	KxDataViewColumnFlags flags = KxDV_COL_DEFAULT_FLAGS|KxDV_COL_SORTABLE;
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer, KxDataViewTextEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 300, flags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Version"), ColumnID::Version, KxDATAVIEW_CELL_INERT, 100, flags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Size"), ColumnID::Size, KxDATAVIEW_CELL_INERT, 200, flags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Game"), ColumnID::Game, KxDATAVIEW_CELL_INERT, 100, flags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Network.Provider"), ColumnID::Provider, KxDATAVIEW_CELL_INERT, 100, flags);
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Date"), ColumnID::Date, KxDATAVIEW_CELL_INERT, 125, flags);
		info.GetColumn()->SortDescending();
	}
	{
		auto info = GetView()->AppendColumn<KxDataViewProgressRenderer>(KTr("Generic.Status"), ColumnID::Status, KxDATAVIEW_CELL_INERT);
		info.GetRenderer()->SetSizeOption(KxDVR_PROGRESS_HEIGHT_FIT);
	}
	RefreshItems();
}

void KDownloadView::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KDownloadEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				break;
			}
		};
	}
	GetValueByRow(value, row, column);
}
void KDownloadView::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KDownloadEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = KxDataViewBitmapTextValue(entry->GetFileInfo().GetName(), GetStateBitmap(*entry));
				break;
			}
			case ColumnID::Version:
			{
				value = entry->GetFileInfo().GetVersion().ToString();
				break;
			}
			case ColumnID::Size:
			{
				int64_t downloadedSize = entry->GetDownloadedSize();
				int64_t totalSize = entry->GetFileInfo().GetSize();
				if (entry->IsCompleted())
				{
					value = KxFile::FormatFileSize(totalSize, 2);
				}
				else
				{
					value = KxFile::FormatFileSize(downloadedSize, 2) + '/' + KxFile::FormatFileSize(totalSize, 2);
				}
				break;
			}
			case ColumnID::Game:
			{
				const KGameInstance* profile = entry->GetTargetProfile();
				if (profile)
				{
					value = profile->GetShortName();
				}
				break;
			}
			case ColumnID::Provider:
			{
				const KNetworkProvider* provider = entry->GetProvider();
				if (provider)
				{
					value = provider->GetName();
				}
				break;
			}
			case ColumnID::Date:
			{
				value = KAux::FormatDateTime(entry->GetDate());
				break;
			}
			case ColumnID::Status:
			{
				// Percent
				int64_t downloadedSize = entry->GetDownloadedSize();
				int64_t totalSize = entry->GetFileInfo().GetSize();
				int percent = 0;
				if (totalSize > 0)
				{
					percent = ((double)downloadedSize / (double)totalSize) * 100;
				}

				// Bar color
				KxDataViewProgressState state = KxDVR_PROGRESS_STATE_NORMAL;
				if (entry->IsPaused())
				{
					state = KxDVR_PROGRESS_STATE_PAUSED;
				}
				else if (entry->IsFailed())
				{
					state = KxDVR_PROGRESS_STATE_ERROR;
				}

				// Label
				wxString label;
				if (entry->IsRunning())
				{
					static wxString sec = KTr("Generic.Sec");
					label = KxFormat("%1%, %2/%3").arg(percent).arg(KxFile::FormatFileSize(entry->GetSpeed(), 0)).arg(sec);
				}
				else
				{
					label = KxFormat("%1%").arg(percent);
				}

				value = KxDataViewProgressValue(percent, label, state);
				break;
			}
		};
	}
}
bool KDownloadView::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	KDownloadEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				return true;
			}
		};
	}
	return false;
}
bool KDownloadView::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	const KDownloadEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				break;
			}
		};
	}
	return false;
}
bool KDownloadView::CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const
{
	const KDownloadEntry* entry1 = GetDataEntry(row1);
	const KDownloadEntry* entry2 = GetDataEntry(row2);

	switch (column ? column->GetID() : ColumnID::Date)
	{
		case ColumnID::Name:
		{
			return KxComparator::IsLess(entry1->GetFileInfo().GetName(), entry2->GetFileInfo().GetName());
		}
		case ColumnID::Version:
		{
			return entry1->GetFileInfo().GetVersion() < entry2->GetFileInfo().GetVersion();
		}
		case ColumnID::Size:
		{
			return entry1->GetFileInfo().GetSize() < entry2->GetFileInfo().GetSize();
		}
		case ColumnID::Game:
		{
			const wxString name1 = entry1->GetTargetProfile() ? entry1->GetTargetProfile()->GetShortName() : wxEmptyString;
			const wxString name2 = entry2->GetTargetProfile() ? entry2->GetTargetProfile()->GetShortName() : wxEmptyString;
			
			return KxComparator::IsLess(name1, name2);
		}
		case ColumnID::Provider:
		{
			KNetworkProviderID id1 = entry1->GetProvider() ? entry1->GetProvider()->GetID() : KNETWORK_PROVIDER_ID_INVALID;
			KNetworkProviderID id2 = entry2->GetProvider() ? entry2->GetProvider()->GetID() : KNETWORK_PROVIDER_ID_INVALID;

			return id1 < id2;
		}
		case Date:
		{
			return entry1->GetDate() < entry2->GetDate();
		}
	};
	return false;
}

void KDownloadView::OnSelectItem(KxDataViewEvent& event)
{
	const KDownloadEntry* enry = GetDataEntry(GetRow(event.GetItem()));
}
void KDownloadView::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	KDownloadEntry* entry = GetDataEntry(GetRow(item));
	if (entry && column)
	{
		Install(*entry);
	}
}
void KDownloadView::OnContextMenu(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KDownloadEntry* entry = GetDataEntry(GetRow(item));
	KDownloadManager* downloadManager = KDownloadManager::GetInstance();
	bool isRunning = entry && entry->IsRunning();
	bool isEmpty = IsEmpty();

	KxMenu contextMenu;
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Install, KTr("DownloadManager.Menu.Install")));
		item->SetBitmap(KGetBitmap(KIMG_BOX));
		item->Enable(entry && entry->IsCompleted());
	}
	contextMenu.AddSeparator();
	
	if constexpr(false)
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Add, KTr("DownloadManager.Menu.Add")));
		item->SetBitmap(KGetBitmap(KIMG_PLUS_SMALL));
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Pause, KTr("DownloadManager.Menu.Pause")));
		item->Enable(entry && isRunning && !entry->IsPaused() && !entry->IsCompleted() && !entry->IsFailed());
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Abort, KTr("DownloadManager.Menu.Abort")));
		item->Enable(entry && isRunning && !entry->IsCompleted() && !entry->IsFailed());
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Resume, KTr("DownloadManager.Menu.Resume")));
		item->Enable(entry && entry->IsPaused() && !entry->IsCompleted());
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Restart, KTr("DownloadManager.Menu.Restart")));
		item->Enable(entry && entry->CanRestart());
	}

	contextMenu.AddSeparator();
	{
		KxMenu* providerMenu = new KxMenu();
		if (entry && !entry->IsRunning())
		{
			for (KNetworkProvider* provider: KNetwork::GetInstance()->GetProviders())
			{
				KxMenuItem* item = providerMenu->Add(new KxMenuItem(provider->GetName(), wxEmptyString, wxITEM_CHECK));
				item->Check(provider == entry->GetProvider());
				item->Bind(KxEVT_MENU_SELECT, [entry, provider](KxMenuEvent& event)
				{
					entry->SetProvider(provider);
					entry->Serialize();
				});
			}
		}

		KxMenuItem* item = contextMenu.Add(providerMenu, KTr("DownloadManager.Menu.SetProvider"));
		item->Enable(providerMenu->GetMenuItemCount() != 0);
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::QueryInfo, KTr("DownloadManager.Menu.QueryInfo")));
		item->Enable(entry && !entry->IsRunning() && entry->HasProvider());
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::ShowChangeLog, KTr("DownloadManager.Menu.ShowChangeLog")));
		item->Enable(entry && entry->GetFileInfo().HasChangeLog());
	}
	contextMenu.AddSeparator();

	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Remove, KTr("DownloadManager.Menu.Remove")));
		item->SetBitmap(KGetBitmap(KIMG_MINUS_SMALL));
		item->Enable(entry && !isEmpty && !isRunning);
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::RemoveAll, KTr("DownloadManager.Menu.RemoveAll")));
		item->Enable(!isEmpty);
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::RemoveInstalled, KTr("DownloadManager.Menu.RemoveInstalled")));
		item->Enable(!isEmpty);
	}
	contextMenu.AddSeparator();

	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Hide, KTr("DownloadManager.Menu.Hide")));
		item->Enable(entry && !isEmpty && !isRunning);
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::HideAll, KTr("DownloadManager.Menu.HideAll")));
		item->Enable(!isEmpty);
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::HideInstalled, KTr("DownloadManager.Menu.HideInstalled")));
		item->Enable(!isEmpty);
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::ShowHidden, KTr("DownloadManager.Menu.ShowHidden"), wxEmptyString, wxITEM_CHECK));
		item->Check(downloadManager->ShouldShowHiddenDownloads());
	}
	contextMenu.AddSeparator();
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::OpenLocation, KTr("MainMenu.OpenLocation")));
		item->SetBitmap(KGetBitmap(KIMG_FOLDER_OPEN));
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Refresh, KTr(KxID_REFRESH)));
		item->SetBitmap(KGetBitmap(KIMG_ARROW_CIRCLE_DOUBLE));
	}

	contextMenu.AddSeparator();
	{
		bool assocOK = downloadManager->IsAssociatedWithNXM();
		wxString label = assocOK ? KTr("DownloadManager.Menu.AssocianedWithNXM") : KTr("DownloadManager.Menu.AssociateWithNXM");

		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::AssociateWithNXM, label, wxEmptyString, wxITEM_CHECK));
		item->Enable(!assocOK);
		item->Check(assocOK);
		item->SetBitmap(KGetBitmap(KIMG_SITE_NEXUS));
	}
	if (entry && entry->IsProviderType(KNETWORK_PROVIDER_ID_NEXUS))
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::CopyNXM, KTr("DownloadManager.Menu.CopyNXM")));
	}

	contextMenu.Bind(KxEVT_MENU_SELECT, [this, entry](KxMenuEvent& event)
	{
		OnContextMenuSelected(event, entry);
		event.Skip();
	});
	contextMenu.Show(GetView());
}
void KDownloadView::OnContextMenuSelected(KxMenuEvent& event, KDownloadEntry* entry)
{
	KDownloadManager* downloadManager = KDownloadManager::GetInstance();

	switch (event.GetItem()->GetId())
	{
		case MenuID::Install:
		{
			Install(*entry);
			break;
		}

		case MenuID::Add:
		{
			break;
		}
		case MenuID::Pause:
		{
			entry->Pause();
			break;
		}
		case MenuID::Abort:
		{
			entry->Stop();
			break;
		}
		case MenuID::Resume:
		{
			entry->Resume();
			break;
		}
		case MenuID::Restart:
		{
			entry->Restart();
			break;
		}

		case MenuID::QueryInfo:
		{
			bool isSucceed = false;
			bool isQueryInfo = false;
			if (entry->IsOK())
			{
				isQueryInfo = true;
				isSucceed = entry->QueryInfo();
			}
			else
			{
				isQueryInfo = false;
				isSucceed = entry->RepairBrokedDownload();
			}

			if (!isSucceed)
			{
				wxString message;
				if (isQueryInfo)
				{
					message = KTrf("DownloadManager.Notification.QueryDownloadInfoFailed", entry->GetFileInfo().GetName());
				}
				else
				{
					message = KTrf("DownloadManager.Notification.RestoreDownloadFailed", entry->GetFileInfo().GetName());
				}
				KNotificationCenter::GetInstance()->Notify(KDownloadManager::GetInstance(), message, KxICON_WARNING);
			}
			break;
		}
		case MenuID::ShowChangeLog:
		{
			KxTaskDialog dialog(GetViewTLW(), KxID_NONE, entry->GetFileInfo().GetDisplayName(), wxEmptyString, KxBTN_OK, KxICON_NONE);
			dialog.SetMessage(KxFormat("%1 %2").arg(KTr("Generic.Version")).arg(entry->GetFileInfo().GetVersion()));
			dialog.SetExMessage(entry->GetFileInfo().GetChangeLog());
			dialog.SetMainIcon(KxShell::GetFileIcon(entry->GetFullPath()));
			dialog.SetOptionEnabled(KxTD_EXMESSAGE_EXPANDED);
			dialog.ShowModal();
			break;
		}

		case MenuID::Remove:
		{
			if (downloadManager->RemoveDownload(*entry))
			{
				RefreshItems();
			}
			break;
		}
		case MenuID::RemoveAll:
		{
			RemoveAll();
			break;
		}
		case MenuID::RemoveInstalled:
		{
			RemoveAll(true);
			break;
		}

		case MenuID::Hide:
		{
			entry->SetHidden(true);
			entry->Serialize();
			RefreshItems();
			break;
		}
		case MenuID::HideAll:
		{
			SetAllHidden(true);
			break;
		}
		case MenuID::HideInstalled:
		{
			SetAllHidden(true, true);
			break;
		}
		case MenuID::ShowHidden:
		{
			downloadManager->ShowHiddenDownloads(!downloadManager->ShouldShowHiddenDownloads());
			downloadManager->LoadDownloads();
			RefreshItems();
			break;
		}

		case MenuID::OpenLocation:
		{
			if (entry)
			{
				KxShell::OpenFolderAndSelectItem(entry->GetFullPath());
			}
			else
			{
				KxShell::Execute(GetViewTLW(), downloadManager->GetDownloadsLocation(), "open");
			}
			break;
		}
		case MenuID::Refresh:
		{
			downloadManager->LoadDownloads();
			RefreshItems();
			break;
		}

		case MenuID::AssociateWithNXM:
		{
			downloadManager->AssociateWithNXM();
			break;
		}
		case MenuID::CopyNXM:
		{
			const KNetworkProviderNexus* nexus = static_cast<const KNetworkProviderNexus*>(entry->GetProvider());
			if (wxTheClipboard->Open())
			{
				wxTheClipboard->SetData(new wxTextDataObject(nexus->ConstructNXM(entry->GetFileInfo(), entry->GetTargetProfile())));
				wxTheClipboard->Close();
			}
			break;
		}
	};
}

wxBitmap KDownloadView::GetStateBitmap(const KDownloadEntry& entry) const
{
	if (!entry.IsFileInfoOK())
	{
		return KGetBitmap(KIMG_EXCLAMATION_CIRCLE_FRAME);
	}
	if (entry.IsCompleted())
	{
		return KGetBitmap(KIMG_TICK_CIRCLE_FRAME);
	}
	if (entry.IsFailed())
	{
		return KGetBitmap(KIMG_CROSS_CIRCLE_FRAME);
	}
	if (entry.IsPaused())
	{
		return KGetBitmap(KIMG_EXCLAMATION_CIRCLE_FRAME_EMPTY);
	}
	return KGetBitmap(KIMG_TICK_CIRCLE_FRAME_EMPTY);
}
void KDownloadView::RemoveAll(bool installedOnly)
{
	KxTaskDialog dialog(GetViewTLW(), KxID_NONE, KTr("DownloadManager.RemoveDownloadsCaption"), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
	dialog.SetMessage(installedOnly ? KTr("DownloadManager.RemoveInstalledDownloadsMessage") : KTr("DownloadManager.RemoveDownloadsMessage"));

	if (dialog.ShowModal() == KxID_YES)
	{
		KDownloadEntry::RefVector items = KDownloadManager::GetInstance()->GetNotRunningDownloads(installedOnly);
		if (!items.empty())
		{
			for (KDownloadEntry& entry: items)
			{
				KDownloadManager::GetInstance()->RemoveDownload(entry);
			}
			RefreshItems();
		}
	}
}
void KDownloadView::SetAllHidden(bool isHidden, bool installedOnly)
{
	KDownloadEntry::RefVector items = KDownloadManager::GetInstance()->GetNotRunningDownloads(installedOnly);
	if (!items.empty())
	{
		for (KDownloadEntry& entry: items)
		{
			entry.SetHidden(isHidden);
			entry.Serialize();
		}

		KDownloadManager::GetInstance()->LoadDownloads();
		RefreshItems();
	}
}
void KDownloadView::Install(KDownloadEntry& entry)
{
	entry.Serialize();
	new KInstallWizardDialog(GetViewTLW(), entry.GetFullPath());
}

KDownloadView::KDownloadView()
{
	SetDataViewFlags(KxDV_VERT_RULES);
	SetDataVector(&KDownloadManager::GetInstance()->GetDownloads());
}
