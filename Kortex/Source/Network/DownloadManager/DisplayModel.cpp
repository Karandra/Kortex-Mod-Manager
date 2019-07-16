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
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("NetworkManager.ModNetwork"), ColumnID::ModSource, KxDATAVIEW_CELL_INERT, 100, flags);
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
		const DownloadItem* entry = GetDataEntry(row);
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
		const DownloadItem* item = GetDataEntry(row);
		if (item)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = KxDataViewBitmapTextValue(item->GetFileInfo().Name, GetStateBitmap(*item));
					break;
				}
				case ColumnID::Version:
				{
					value = item->GetFileInfo().Version.ToString();
					break;
				}
				case ColumnID::Size:
				{
					value = KxFile::FormatFileSize(item->GetFileInfo().Size, 2);
					break;
				}
				case ColumnID::Game:
				{
					const GameID gameID = item->GetTargetGame();
					if (gameID)
					{
						value = gameID.ToGameInstance()->GetGameShortName();
					}
					break;
				}
				case ColumnID::ModSource:
				{
					const IModNetwork* modNetwork = item->GetModNetwork();
					if (modNetwork)
					{
						value = modNetwork->GetName();
					}
					break;
				}
				case ColumnID::Date:
				{
					value = KAux::FormatDateTime(item->GetDownloadDate());
					break;
				}
				case ColumnID::Status:
				{
					// Percent
					const int64_t downloadedSize = item->GetDownloadedSize();
					const int64_t totalSize = item->GetTotalSize();
					int percent = 0;
					if (totalSize > 0)
					{
						percent = ((float)downloadedSize / (float)totalSize) * 100;
					}

					// Bar color
					KxDataViewProgressState state = KxDVR_PROGRESS_STATE_NORMAL;
					if (item->IsPaused())
					{
						state = KxDVR_PROGRESS_STATE_PAUSED;
					}
					else if (item->IsFailed())
					{
						state = KxDVR_PROGRESS_STATE_ERROR;
					}

					// Label
					wxString label;
					if (item->IsRunning())
					{
						const IDownloadExecutor* executor = item->GetExecutor();

						static wxString sec = KTr(wxS("Generic.Sec"));
						label = KxString::Format(wxS("%1%, %2/%3"), percent, KxFile::FormatFileSize(executor->GetSpeed(), 0), sec);
					}
					else
					{
						label = KxString::Format(wxS("%1%"), percent);
					}

					if (!item->IsCompleted())
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
		DownloadItem* entry = GetDataEntry(row);
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
		const DownloadItem* entry = GetDataEntry(row);
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
		const DownloadItem* entry = GetDataEntry(row);
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
		const DownloadItem* left = GetDataEntry(row1);
		const DownloadItem* right = GetDataEntry(row2);

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
				wxString nameLeft = left->GetModNetwork() ? left->GetModNetwork()->GetName() : wxEmptyString;
				wxString nameRight = right->GetModNetwork() ? right->GetModNetwork()->GetName() : wxEmptyString;

				return nameLeft < nameRight;
			}
			case Date:
			{
				return left->GetDownloadDate() < right->GetDownloadDate();
			}
		};
		return false;
	}

	void DisplayModel::OnSelectItem(KxDataViewEvent& event)
	{
		//const DownloadItem* entry = GetDataEntry(GetRow(event.GetItem()));
	}
	void DisplayModel::OnActivateItem(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		KxDataViewColumn* column = event.GetColumn();
		DownloadItem* entry = GetDataEntry(GetRow(item));
		if (entry && column)
		{
			Install(*entry);
		}
	}
	void DisplayModel::OnContextMenu(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		DownloadItem* download = GetDataEntry(GetRow(item));
		const IModNetwork* modNetwork = download ? download->GetModNetwork() : nullptr;

		const bool isRunning = download && download->IsRunning();
		const bool isPaused = download && download->IsPaused();
		const bool isFailed = download && download->IsFailed();
		const bool isCompleted = download && download->IsCompleted();
		const bool isEmpty = IsEmpty();

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
			item->Enable(isRunning);
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
					wxString message = KTrf("DownloadManager.Notification.QueryDownloadInfoFailed", download->GetFileInfo().Name);
					INotificationCenter::NotifyUsing<IDownloadManager>(message, KxICON_WARNING);
				}
			});
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr("DownloadManager.Menu.ShowChangeLog")));
			item->Enable(download && !download->GetFileInfo().ChangeLog.IsEmpty());
			item->Bind(KxEVT_MENU_SELECT, [this, download](KxMenuEvent& event)
			{
				KxTaskDialog dialog(GetView(), KxID_NONE, download->GetFileInfo().DisplayName, wxEmptyString, KxBTN_OK, KxICON_NONE);
				dialog.SetMessage(KxString::Format("%1 %2", KTr("Generic.Version"), download->GetFileInfo().Version));
				dialog.SetExMessage(download->GetFileInfo().ChangeLog);
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
				if (m_DownloadManager->RemoveDownload(*download))
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
			item->Check(m_DownloadManager->ShouldShowHiddenDownloads());
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				m_DownloadManager->ShowHiddenDownloads(!m_DownloadManager->ShouldShowHiddenDownloads());
				m_DownloadManager->LoadDownloads();
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
					KxShell::Execute(GetView(), m_DownloadManager->GetDownloadsLocation(), "open");
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
				KxShell::Execute(GetView(), download->GetModNetwork()->GetModPageURI(ModRepositoryRequest(download->GetFileInfo())).BuildUnescapedURI(), "open");
			});
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr(KxID_REFRESH)));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::ArrowCircleDouble));
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				m_DownloadManager->LoadDownloads();
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

	wxBitmap DisplayModel::GetStateBitmap(const DownloadItem& entry) const
	{
		if (!entry.GetFileInfo().IsOK())
		{
			return ImageProvider::GetBitmap(ImageResourceID::ExclamationCircleFrame);
		}
		if (entry.IsCompleted())
		{
			return ImageProvider::GetBitmap(ImageResourceID::TickCircleFrame);
		}
		if (entry.IsFailed())
		{
			return ImageProvider::GetBitmap(ImageResourceID::CrossCircleFrame);
		}
		if (entry.IsPaused())
		{
			return ImageProvider::GetBitmap(ImageResourceID::ExclamationCircleFrameEmpty);
		}
		return ImageProvider::GetBitmap(ImageResourceID::TickCircleFrameEmpty);
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
			for (DownloadItem* entry: items)
			{
				entry->SetHidden(isHidden);
				entry->Save();
			}

			IDownloadManager::GetInstance()->LoadDownloads();
			RefreshItems();
		}
	}
	void DisplayModel::Install(DownloadItem& entry)
	{
		entry.Save();
		new InstallWizard::WizardDialog(GetViewTLW(), entry.GetFullPath());
	}

	DisplayModel::DisplayModel()
		:m_DownloadManager(IDownloadManager::GetInstance())
	{
		SetDataViewFlags(KxDV_VERT_RULES);
		SetDataVector(&IDownloadManager::GetInstance()->GetDownloads());
	}

	KxDataViewItem DisplayModel::FindItem(const DownloadItem& entry) const
	{
		return KxDataViewItem();
	}
}
