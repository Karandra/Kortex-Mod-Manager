#include "stdafx.h"
#include "KDownloadManagerView.h"
#include "Network/KNetwork.h"
#include "Network/KNetworkProviderNexus.h"
#include "Network/KNetworkProviderTESALL.h"
#include "Network/KNetworkProviderLoversLab.h"
#include "InstallWizard/KInstallWizardDialog.h"
#include "Profile/KProfile.h"
#include "KApp.h"
#include "KAux.h"
#include "KComparator.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxTaskDialog.h>
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

void KDownloadManagerView::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KDownloadManagerView::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KDownloadManagerView::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KDownloadManagerView::OnContextMenu, this);
	GetView()->Bind(KxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, [this](KxDataViewEvent& event)
	{
		KxMenu menu;
		if (GetView()->CreateColumnSelectionMenu(menu))
		{
			GetView()->OnColumnSelectionMenu(menu);
		}
	});

	KxDataViewColumnFlags flags = KxDV_COL_DEFAULT_FLAGS|KxDV_COL_SORTABLE;
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer, KxDataViewTextEditor>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 300, flags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Version"), ColumnID::Version, KxDATAVIEW_CELL_INERT, 100, flags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Size"), ColumnID::Size, KxDATAVIEW_CELL_INERT, 200, flags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Game"), ColumnID::Game, KxDATAVIEW_CELL_INERT, 100, flags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Network.Provider"), ColumnID::Provider, KxDATAVIEW_CELL_INERT, 100, flags);
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Date"), ColumnID::Date, KxDATAVIEW_CELL_INERT, 125, flags);
		info.GetColumn()->SortDescending();
	}
	{
		auto info = GetView()->AppendColumn<KxDataViewProgressRenderer>(T("Generic.Status"), ColumnID::Status, KxDATAVIEW_CELL_INERT);
		info.GetRenderer()->SetSizeOption(KxDVR_PROGRESS_HEIGHT_FIT);
	}
	RefreshItems();
}

void KDownloadManagerView::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
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
void KDownloadManagerView::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
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
				const KProfile* profile = entry->GetTargetProfile();
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
					static wxString sec = T("Generic.Sec");
					label = wxString::Format("%d%%, %s/%s", percent, KxFile::FormatFileSize(entry->GetSpeed(), 0), sec);
				}
				else
				{
					label = wxString::Format("%d%%", percent);
				}

				value = KxDataViewProgressValue(percent, label, state);
				break;
			}
		};
	}
}
bool KDownloadManagerView::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
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
bool KDownloadManagerView::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
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
bool KDownloadManagerView::CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const
{
	const KDownloadEntry* entry1 = GetDataEntry(row1);
	const KDownloadEntry* entry2 = GetDataEntry(row2);
	using KComparator::KCompare;

	switch (column ? column->GetID() : ColumnID::Date)
	{
		case ColumnID::Name:
		{
			return KCompare(entry1->GetFileInfo().GetName(), entry2->GetFileInfo().GetName()) < 0;
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
			
			return KCompare(name1, name2) < 0;
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

void KDownloadManagerView::OnSelectItem(KxDataViewEvent& event)
{
	const KDownloadEntry* enry = GetDataEntry(GetRow(event.GetItem()));
}
void KDownloadManagerView::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	KDownloadEntry* entry = GetDataEntry(GetRow(item));
	if (entry && column && !m_InstallWizardActive)
	{
		Install(*entry);
	}
}
void KDownloadManagerView::OnContextMenu(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KDownloadEntry* entry = GetDataEntry(GetRow(item));
	KDownloadManager* downloadManager = KDownloadManager::GetInstance();
	bool isRunning = entry && entry->IsRunning();
	bool isEmpty = IsEmpty();

	KxMenu contextMenu;
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Install, T("DownloadManager.Menu.Install")));
		item->SetBitmap(KGetBitmap(KIMG_BOX));
		item->Enable(entry && entry->IsCompleted() && !m_InstallWizardActive);
	}
	contextMenu.AddSeparator();
	
	if constexpr(false)
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Add, T("DownloadManager.Menu.Add")));
		item->SetBitmap(KGetBitmap(KIMG_PLUS_SMALL));
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Pause, T("DownloadManager.Menu.Pause")));
		item->Enable(entry && isRunning && !entry->IsPaused() && !entry->IsCompleted() && !entry->IsFailed());
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Abort, T("DownloadManager.Menu.Abort")));
		item->Enable(entry && isRunning && !entry->IsCompleted() && !entry->IsFailed() && !entry->IsPaused());
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Resume, T("DownloadManager.Menu.Resume")));
		item->Enable(entry && entry->IsPaused() && !entry->IsCompleted());
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Restart, T("DownloadManager.Menu.Restart")));
		item->Enable(entry && entry->CanRestart());
	}
	contextMenu.AddSeparator();

	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Remove, T("DownloadManager.Menu.Remove")));
		item->SetBitmap(KGetBitmap(KIMG_MINUS_SMALL));
		item->Enable(entry && !isEmpty && !isRunning);
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::RemoveAll, T("DownloadManager.Menu.RemoveAll")));
		item->Enable(!isEmpty);
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::RemoveInstalled, T("DownloadManager.Menu.RemoveInstalled")));
		item->Enable(!isEmpty);
	}
	contextMenu.AddSeparator();

	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Hide, T("DownloadManager.Menu.Hide")));
		item->Enable(entry && !isEmpty && !isRunning);
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::HideAll, T("DownloadManager.Menu.HideAll")));
		item->Enable(!isEmpty);
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::HideInstalled, T("DownloadManager.Menu.HideInstalled")));
		item->Enable(!isEmpty);
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::ShowHidden, T("DownloadManager.Menu.ShowHidden"), wxEmptyString, wxITEM_CHECK));
		item->Check(downloadManager->ShouldShowHiddenDownloads());
	}
	contextMenu.AddSeparator();
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::OpenLocation, T("MainMenu.OpenLocation")));
		item->SetBitmap(KGetBitmap(KIMG_FOLDER_OPEN));
	}
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Refresh, T(KxID_REFRESH)));
		item->SetBitmap(KGetBitmap(KIMG_ARROW_CIRCLE_DOUBLE));
	}

	contextMenu.AddSeparator();
	{
		bool assocOK = downloadManager->IsAssociatedWithNXM();
		wxString label = assocOK ? T("DownloadManager.Menu.AssocianedWithNXM") : T("DownloadManager.Menu.AssociateWithNXM");

		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::AssociateWithNXM, label, wxEmptyString, wxITEM_CHECK));
		item->Enable(!assocOK);
		item->Check(assocOK);
		item->SetBitmap(KGetBitmap(KIMG_SITE_NEXUS));
	}
	if (entry && entry->HasProvider() && entry->GetProvider()->GetID() == KNETWORK_PROVIDER_ID_NEXUS)
	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::CopyNXM, T("DownloadManager.Menu.CopyNXM")));
	}

	contextMenu.Bind(KxEVT_MENU_SELECT, [this, entry](KxMenuEvent& event)
	{
		OnContextMenuSelected(event, entry);
	});
	contextMenu.Show(GetView());
}
void KDownloadManagerView::OnContextMenuSelected(KxMenuEvent& event, KDownloadEntry* entry)
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

wxBitmap KDownloadManagerView::GetStateBitmap(const KDownloadEntry& entry) const
{
	if (entry.IsCompleted())
	{
		return KGetBitmap(KIMG_TICK_CIRCLE_FRAME);
	}
	else if (entry.IsFailed())
	{
		return KGetBitmap(KIMG_CROSS_CIRCLE_FRAME);
	}
	else if (entry.IsPaused())
	{
		return KGetBitmap(KIMG_EXCLAMATION_CIRCLE_FRAME_EMPTY);
	}
	return KGetBitmap(KIMG_TICK_CIRCLE_FRAME_EMPTY);
}
void KDownloadManagerView::RemoveAll(bool installedOnly)
{
	KxTaskDialog dialog(GetViewTLW(), KxID_NONE, T("DownloadManager.RemoveDownloadsCaption"), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
	dialog.SetMessage(installedOnly ? T("DownloadManager.RemoveInstalledDownloadsMessage") : T("DownloadManager.RemoveDownloadsMessage"));

	if (dialog.ShowModal() == KxID_YES)
	{
		KDownloadEntry::RefContainer items = KDownloadManager::GetInstance()->GetNotRunningItems(installedOnly);
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
void KDownloadManagerView::SetAllHidden(bool isHidden, bool installedOnly)
{
	KDownloadEntry::RefContainer items = KDownloadManager::GetInstance()->GetNotRunningItems(installedOnly);
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
void KDownloadManagerView::Install(KDownloadEntry& entry)
{
	m_InstallWizardActive = true;
	KInstallWizardDialog* installWizard = new KInstallWizardDialog(GetViewTLW(), entry.GetFullPath());

	installWizard->Bind(wxEVT_CLOSE_WINDOW, [this, installWizard, &entry](wxCloseEvent& event)
	{
		entry.SetInstalled(installWizard->IsCompleted());
		entry.Serialize();

		m_InstallWizardActive = false;
		event.Skip();
	});
}

KDownloadManagerView::KDownloadManagerView()
{
	SetDataViewFlags(KxDV_VERT_RULES);
	SetDataVector(&KDownloadManager::GetInstance()->GetDownloads());
}
