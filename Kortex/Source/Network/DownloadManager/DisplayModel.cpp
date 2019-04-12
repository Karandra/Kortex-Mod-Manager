#include "stdafx.h"
#include "DisplayModel.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/NetworkManager.hpp>
#include "InstallWizard/KInstallWizardDialog.h"
#include "Utility/KAux.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxComparator.h>
#include <wx/clipbrd.h>

namespace
{
	enum ColumnID
	{
		Name,
		Version,
		Size,
		Game,
		ModSource,
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
}

namespace Kortex::DownloadManager
{
	void DisplayModel::OnInitControl()
	{
		GetView()->SetUniformRowHeight(GetView()->GetDefaultRowHeight(KxDVC_ROW_HEIGHT_EXPLORER));
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &DisplayModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &DisplayModel::OnSelectItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &DisplayModel::OnContextMenu, this);
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
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Network.ModSource"), ColumnID::ModSource, KxDATAVIEW_CELL_INERT, 100, flags);
		{
			auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Date"), ColumnID::Date, KxDATAVIEW_CELL_INERT, 125, flags);
			info.GetColumn()->SortDescending();
		}
		{
			auto info = GetView()->AppendColumn<KxDataViewProgressRenderer>(KTr("Generic.Status"), ColumnID::Status, KxDATAVIEW_CELL_INERT);
			info.GetRenderer()->SetSizeOption(KxDVR_PROGRESS_HEIGHT_AUTO);
		}
		RefreshItems();
	}

	void DisplayModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		const IDownloadEntry* entry = GetDataEntry(row);
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
	void DisplayModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		const IDownloadEntry* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = KxDataViewBitmapTextValue(entry->GetFileInfo().Name, GetStateBitmap(*entry));
					break;
				}
				case ColumnID::Version:
				{
					value = entry->GetFileInfo().Version.ToString();
					break;
				}
				case ColumnID::Size:
				{
					value = KxFile::FormatFileSize(entry->GetFileInfo().Size, 2);
					break;
				}
				case ColumnID::Game:
				{
					const IGameInstance* instance = entry->GetTargetGame();
					if (instance)
					{
						value = instance->GetGameShortName();
					}
					break;
				}
				case ColumnID::ModSource:
				{
					const IModSource* modSource = entry->GetModSource();
					if (modSource)
					{
						value = modSource->GetName();
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
					const int64_t downloadedSize = entry->GetDownloadedSize();
					const int64_t totalSize = entry->GetFileInfo().Size;
					int percent = 0;
					if (totalSize > 0)
					{
						percent = ((float)downloadedSize / (float)totalSize) * 100;
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
						static wxString sec = KTr(wxS("Generic.Sec"));
						label = KxString::Format(wxS("%1%, %2/%3"), percent, KxFile::FormatFileSize(entry->GetSpeed(), 0), sec);
					}
					else
					{
						label = KxString::Format(wxS("%1%"), percent);
					}

					if (!entry->IsCompleted())
					{
						label += wxS(", ");
						label += KxFile::FormatFileSize(downloadedSize, 2) + wxS('/') + KxFile::FormatFileSize(totalSize, 2);
					}

					value = KxDataViewProgressValue(percent, label, state);
					break;
				}
			};
		}
	}
	bool DisplayModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
	{
		IDownloadEntry* entry = GetDataEntry(row);
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
	bool DisplayModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
	{
		const IDownloadEntry* entry = GetDataEntry(row);
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
	bool DisplayModel::GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attribute, KxDataViewCellState cellState) const
	{
		const IDownloadEntry* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Size:
				{
					attribute.SetAlignment(wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT);
					return true;
				}
			};
		}
		return false;
	}
	bool DisplayModel::CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const
	{
		const IDownloadEntry* left = GetDataEntry(row1);
		const IDownloadEntry* right = GetDataEntry(row2);

		switch (column ? column->GetID() : ColumnID::Date)
		{
			case ColumnID::Name:
			{
				return KxComparator::IsLess(left->GetFileInfo().Name, right->GetFileInfo().Name);
			}
			case ColumnID::Version:
			{
				return left->GetFileInfo().Version < right->GetFileInfo().Version;
			}
			case ColumnID::Size:
			{
				return left->GetFileInfo().Size < right->GetFileInfo().Size;
			}
			case ColumnID::Game:
			{
				const wxString nameLeft = left->GetTargetGame() ? left->GetTargetGame()->GetGameShortName() : wxEmptyString;
				const wxString nameRight = right->GetTargetGame() ? right->GetTargetGame()->GetGameShortName() : wxEmptyString;

				return KxComparator::IsLess(nameLeft, nameRight);
			}
			case ColumnID::ModSource:
			{
				using namespace NetworkManager;
				wxString nameLeft = left->GetModSource() ? left->GetModSource()->GetName() : wxEmptyString;
				wxString nameRight = right->GetModSource() ? right->GetModSource()->GetName() : wxEmptyString;

				return nameLeft < nameRight;
			}
			case Date:
			{
				return left->GetDate() < right->GetDate();
			}
		};
		return false;
	}

	void DisplayModel::OnSelectItem(KxDataViewEvent& event)
	{
		//const IDownloadEntry* entry = GetDataEntry(GetRow(event.GetItem()));
	}
	void DisplayModel::OnActivateItem(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		KxDataViewColumn* column = event.GetColumn();
		IDownloadEntry* entry = GetDataEntry(GetRow(item));
		if (entry && column)
		{
			Install(*entry);
		}
	}
	void DisplayModel::OnContextMenu(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		IDownloadEntry* entry = GetDataEntry(GetRow(item));
		bool isRunning = entry && entry->IsRunning();
		bool isEmpty = IsEmpty();

		KxMenu contextMenu;
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::Install, KTr("DownloadManager.Menu.Install")));
			item->SetBitmap(KGetBitmap(KIMG_BOX));
			item->Enable(entry && entry->IsCompleted());
		}
		contextMenu.AddSeparator();

		if constexpr (false)
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
				for (auto& modSource: INetworkManager::GetInstance()->GetModSources())
				{
					if (modSource->QueryInterface<IModRepository>())
					{
						KxMenuItem* item = providerMenu->Add(new KxMenuItem(modSource->GetName(), wxEmptyString, wxITEM_CHECK));
						item->Check(modSource.get() == entry->GetModSource());
						item->Bind(KxEVT_MENU_SELECT, [entry, &modSource](KxMenuEvent& event)
						{
							entry->SetModSource(modSource.get());
							entry->Save();
						});
					}
				}
			}

			KxMenuItem* item = contextMenu.Add(providerMenu, KTr("DownloadManager.Menu.SetModSource"));
			item->Enable(providerMenu->GetMenuItemCount() != 0);
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::QueryInfo, KTr("DownloadManager.Menu.QueryInfo")));
			item->Enable(entry && !entry->IsRunning() && entry->HasModSource());
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::ShowChangeLog, KTr("DownloadManager.Menu.ShowChangeLog")));
			item->Enable(entry && !entry->GetFileInfo().ChangeLog.IsEmpty());
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
			item->Check(m_DownloadManager->ShouldShowHiddenDownloads());
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

		IDownloadManagerNXM* nxm = nullptr;
		if (m_DownloadManager->QueryInterface(nxm))
		{
			contextMenu.AddSeparator();
			{
				bool assocOK = nxm->IsAssociatedWithNXM();
				wxString label = assocOK ? KTr("DownloadManager.Menu.AssocianedWithNXM") : KTr("DownloadManager.Menu.AssociateWithNXM");

				KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::AssociateWithNXM, label, wxEmptyString, wxITEM_CHECK));
				item->Enable(!assocOK);
				item->Check(assocOK);
				item->SetBitmap(KGetBitmap(KIMG_SITE_NEXUS));
			}
			if (entry && entry->IsModSourceOfType<NetworkManager::NexusSource>())
			{
				KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::CopyNXM, KTr("DownloadManager.Menu.CopyNXM")));
			}
		}

		contextMenu.Bind(KxEVT_MENU_SELECT, [this, entry](KxMenuEvent& event)
		{
			OnContextMenuSelected(event, entry);
			event.Skip();
		});
		contextMenu.Show(GetView());
	}
	void DisplayModel::OnContextMenuSelected(KxMenuEvent& event, IDownloadEntry* entry)
	{
		IDownloadManager* downloadManager = IDownloadManager::GetInstance();

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
						message = KTrf("DownloadManager.Notification.QueryDownloadInfoFailed", entry->GetFileInfo().Name);
					}
					else
					{
						message = KTrf("DownloadManager.Notification.RestoreDownloadFailed", entry->GetFileInfo().Name);
					}
					INotificationCenter::GetInstance()->NotifyUsing<IDownloadManager>(message, KxICON_WARNING);
				}
				break;
			}
			case MenuID::ShowChangeLog:
			{
				KxTaskDialog dialog(GetViewTLW(), KxID_NONE, entry->GetFileInfo().DisplayName, wxEmptyString, KxBTN_OK, KxICON_NONE);
				dialog.SetMessage(KxString::Format("%1 %2", KTr("Generic.Version"), entry->GetFileInfo().Version));
				dialog.SetExMessage(entry->GetFileInfo().ChangeLog);
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
				entry->Save();
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
				downloadManager->QueryInterface<IDownloadManagerNXM>()->AssociateWithNXM();
				break;
			}
			case MenuID::CopyNXM:
			{
				const NetworkManager::NexusSource* nexus = NetworkManager::NexusSource::GetInstance();
				if (wxTheClipboard->Open())
				{
					wxTheClipboard->SetData(new wxTextDataObject(nexus->ConstructNXM(entry->GetFileInfo(), entry->GetTargetGameID())));
					wxTheClipboard->Close();
				}
				break;
			}
		};
	}

	wxBitmap DisplayModel::GetStateBitmap(const IDownloadEntry& entry) const
	{
		if (!entry.GetFileInfo().IsOK())
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
	void DisplayModel::RemoveAll(bool installedOnly)
	{
		KxTaskDialog dialog(GetViewTLW(), KxID_NONE, KTr("DownloadManager.RemoveDownloadsCaption"), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
		dialog.SetMessage(installedOnly ? KTr("DownloadManager.RemoveInstalledDownloadsMessage") : KTr("DownloadManager.RemoveDownloadsMessage"));

		if (dialog.ShowModal() == KxID_YES)
		{
			IDownloadEntry::RefVector items = IDownloadManager::GetInstance()->GetNotRunningDownloads(installedOnly);
			if (!items.empty())
			{
				for (IDownloadEntry* entry: items)
				{
					IDownloadManager::GetInstance()->RemoveDownload(*entry);
				}
				RefreshItems();
			}
		}
	}
	void DisplayModel::SetAllHidden(bool isHidden, bool installedOnly)
	{
		IDownloadEntry::RefVector items = IDownloadManager::GetInstance()->GetNotRunningDownloads(installedOnly);
		if (!items.empty())
		{
			for (IDownloadEntry* entry: items)
			{
				entry->SetHidden(isHidden);
				entry->Save();
			}

			IDownloadManager::GetInstance()->LoadDownloads();
			RefreshItems();
		}
	}
	void DisplayModel::Install(IDownloadEntry& entry)
	{
		entry.Save();
		new KInstallWizardDialog(GetViewTLW(), entry.GetFullPath());
	}

	DisplayModel::DisplayModel()
		:m_DownloadManager(IDownloadManager::GetInstance())
	{
		SetDataViewFlags(KxDV_VERT_RULES);
		SetDataVector(&IDownloadManager::GetInstance()->GetDownloads());
	}

	KxDataViewItem DisplayModel::FindItem(const IDownloadEntry& entry) const
	{
		IDownloadEntry::Vector& items = m_DownloadManager->GetDownloads();
		auto it = m_DownloadManager->GetDownloadIterator(items, entry);
		if (it != items.end())
		{
			return GetItem(std::distance(items.begin(), it));
		}
		return KxDataViewItem();
	}
}
