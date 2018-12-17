#include "stdafx.h"
#include <Kortex/ScreenshotsGallery.hpp>
#include "KPCIImagesListModel.h"
#include "PackageProject/KPackageProject.h"
#include "UI/KMainWindow.h"
#include "UI/KTextEditorDialog.h"
#include "UI/KImageViewerDialog.h"
#include "KOperationWithProgress.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxFileOperationEvent.h>
#include <KxFramework/KxDualProgressDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/DataView/KxDataViewMainWindow.h>

enum ColumnID
{
	Bitmap,
	Visible,
	Main,
	Header,
	Path,
	Description,
};
enum MenuID
{
	ImportFiles,
	AddMultipleFiles,
};

void KPCIImagesListModel::LoadBitmap(KPPIImageEntry* entry)
{
	wxImage image;
	image.SetOption(wxIMAGE_OPTION_MAX_WIDTH, ms_BitmapWidth);
	image.SetOption(wxIMAGE_OPTION_MAX_HEIGHT, ms_BitmapHeight);
	image.LoadFile(entry->GetPath());
	entry->SetBitmap(wxBitmap(image, 32));
}

void KPCIImagesListModel::OnInitControl()
{
	GetView()->SetUniformRowHeight(ms_BitmapHeight + 4);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KPCIImagesListModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KPCIImagesListModel::OnContextMenu, this);
	GetView()->Bind(KxEVT_DATAVIEW_CACHE_HINT, &KPCIImagesListModel::OnCacheHint, this);

	GetView()->AppendColumn<KxDataViewBitmapRenderer>(wxEmptyString, ColumnID::Bitmap, KxDATAVIEW_CELL_INERT, ms_BitmapWidth, KxDV_COL_NONE);
	GetView()->AppendColumn<KxDataViewToggleRenderer>(KTr("PackageCreator.PageInterface.ImageList.Show"), ColumnID::Visible, KxDATAVIEW_CELL_ACTIVATABLE, KxCOL_WIDTH_AUTOSIZE);

	// Main
	{
		auto info = GetView()->AppendColumn<KxDataViewToggleRenderer>(KTr("PackageCreator.PageInterface.ImageList.Main"), ColumnID::Main, KxDATAVIEW_CELL_ACTIVATABLE, KxCOL_WIDTH_AUTOSIZE);
		info.GetRenderer()->SetDefaultToggleType(KxDataViewToggleRenderer::ToggleType::RadioBox);
	}

	// Header
	{
		auto info = GetView()->AppendColumn<KxDataViewToggleRenderer>(KTr("PackageCreator.PageInterface.ImageList.Header"), ColumnID::Header, KxDATAVIEW_CELL_ACTIVATABLE, KxCOL_WIDTH_AUTOSIZE);
		info.GetRenderer()->SetDefaultToggleType(KxDataViewToggleRenderer::ToggleType::RadioBox);
	}

	GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("PackageCreator.PageInterface.ImageList.Value"), ColumnID::Path, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH);
	GetView()->AppendColumn<KxDataViewHTMLRenderer>(KTr("Generic.Description"), ColumnID::Description, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH);
}

void KPCIImagesListModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KPPIImageEntry* entry = GetDataEntry(row);
	if (entry)
	{
		auto IsChecked = [entry](const wxString& path)
		{
			return !path.IsEmpty() && !entry->GetPath().IsEmpty() && entry->GetPath() == path;
		};

		switch (column->GetID())
		{
			case ColumnID::Bitmap:
			{
				value = entry->GetBitmap();
				break;
			}
			case ColumnID::Main:
			{
				value = IsChecked(m_Interface->GetMainImage());
				break;
			}
			case ColumnID::Header:
			{
				value = IsChecked(m_Interface->GetHeaderImage());
				break;
			}
			case ColumnID::Visible:
			{
				value = entry->IsVisible();
				break;
			}
			case ColumnID::Path:
			{
				value = entry->GetPath();
				break;
			}
			case ColumnID::Description:
			{
				value = entry->GetDescriptionRaw();
				break;
			}
		};
	}
}
bool KPCIImagesListModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	KPPIImageEntry* entry = GetDataEntry(row);
	if (entry)
	{
		auto SetChecked = [&value, entry]() -> wxString
		{
			return value.As<bool>() ? entry->GetPath() : wxEmptyString;
		};

		switch (column->GetID())
		{
			case ColumnID::Main:
			{
				m_Interface->SetMainImage(SetChecked());
				ChangeNotify();
				return true;
			}
			case ColumnID::Header:
			{
				m_Interface->SetHeaderImage(SetChecked());
				ChangeNotify();
				return true;
			}
			case ColumnID::Visible:
			{
				entry->SetVisible(value.As<bool>());
				ChangeNotify();
				return true;
			}
		};
	}
	return false;
}

bool KPCIImagesListModel::DoTrackImagePath(const wxString& trackedID, const wxString& newID, bool remove) const
{
	KPackageProjectInterface& interfaceConfig = GetProject().GetInterface();

	// Main image
	if (interfaceConfig.GetMainImage() == trackedID)
	{
		interfaceConfig.SetMainImage(remove ? wxEmptyString : newID);
	}

	// Header image
	if (interfaceConfig.GetHeaderImage() == trackedID)
	{
		interfaceConfig.SetHeaderImage(remove ? wxEmptyString : newID);
	}

	// Manual components
	for (auto& step: GetProject().GetComponents().GetSteps())
	{
		for (auto& group: step->GetGroups())
		{
			for (auto& entry: group->GetEntries())
			{
				if (entry->GetImage() == trackedID)
				{
					entry->SetImage(remove ? wxEmptyString : newID);
				}
			}
		}
	}

	return true;
}

void KPCIImagesListModel::OnActivateItem(KxDataViewEvent& event)
{
	if (event.GetColumn())
	{
		KPPIImageEntry* entry = GetDataEntry(GetRow(event.GetItem()));
		switch (event.GetColumn()->GetID())
		{
			case ColumnID::Bitmap:
			{
				if (entry && entry->HasBitmap())
				{
					KImageViewerDialog dialog(KMainWindow::GetInstance(), entry->GetPath());
					
					KImageViewerEvent imageEvent;
					imageEvent.SetFilePath(entry->GetPath());
					imageEvent.SetDescription(entry->GetDescription());
					dialog.Navigate(imageEvent);
					dialog.ShowModal();
				}
				break;
			}
			case ColumnID::Path:
			{
				if (entry)
				{
					KxFileBrowseDialog dialog(KMainWindow::GetInstance(), KxID_NONE, KxFBD_OPEN);
					dialog.SetFolder(entry->GetPath().BeforeLast('\\'));
					dialog.AddFilter(KxString::Join(Kortex::IScreenshotsGallery::GetSupportedExtensions(), ";"), KTr("FileFilter.Images"));
					dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
					if (dialog.ShowModal() == KxID_OK)
					{
						wxString newPath = dialog.GetResult();
						TrackChangeID(entry->GetPath(), newPath);

						entry->ResetBitmap();
						entry->SetPath(newPath);
						NotifyChangedItem(event.GetItem());
					}
				}
				break;
			}
			case ColumnID::Description:
			{
				if (entry)
				{
					KTextEditorDialog dialog(KMainWindow::GetInstance());
					dialog.SetText(entry->GetDescriptionRaw());
					if (dialog.ShowModal() == KxID_OK && dialog.IsModified())
					{
						entry->SetDescription(dialog.GetText());
						NotifyChangedItem(event.GetItem());
					}
				}
				break;
			}
		};
	}
}
void KPCIImagesListModel::OnContextMenu(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	const KPPIImageEntry* entry = GetDataEntry(GetRow(item));

	KxMenu menu;
	{
		KxMenu* allItems = CreateAllItemsMenu(menu);
		CreateAllItemsMenuEntry(allItems, ColumnID::Description);
		menu.AddSeparator();
	}

	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::ImportFiles, KTr("PackageCreator.ImportFiles")));
		item->SetBitmap(KGetBitmap(KIMG_FOLDER_SEARCH_RESULT));
	}
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddMultipleFiles, KTr("PackageCreator.AddMultipleFiles")));
		item->SetBitmap(KGetBitmap(KIMG_DOCUMENTS_PLUS));
	}
	menu.AddSeparator();
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(KxID_REMOVE, KTr(KxID_REMOVE)));
		item->Enable(entry != nullptr);
	}
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(KxID_CLEAR, KTr(KxID_CLEAR)));
		item->Enable(!IsEmpty());
	}

	switch (menu.Show(GetView()))
	{
		case MenuID::ImportFiles:
		{
			OnImportFiles();
			break;
		}
		case MenuID::AddMultipleFiles:
		{
			OnAddMultipleItems();
			break;
		}
		case KxID_REMOVE:
		{
			OnRemoveEntry(item);
			break;
		}
		case KxID_CLEAR:
		{
			OnClearList();
			break;
		}
	};
};
void KPCIImagesListModel::OnCacheHint(KxDataViewEvent& event)
{
	for (size_t row = event.GetCacheHintFrom(); row <= event.GetCacheHintTo(); row++)
	{
		KxDataViewItem item = GetView()->GetMainWindow()->GetItemByRow(row);
		KPPIImageEntry* entry = GetDataEntry(GetRow(item));
		if (entry && !entry->HasBitmap())
		{
			LoadBitmap(entry);
		}
	}
}

void KPCIImagesListModel::OnAllItemsMenuSelect(KxDataViewColumn* column)
{
	switch (column->GetID())
	{
		case ColumnID::Description:
		{
			KTextEditorDialog dialog(GetView());
			if (dialog.ShowModal() == KxID_OK)
			{
				for (auto& entry: *GetDataVector())
				{
					entry.SetDescription(dialog.GetText());
				}
				NotifyAllItemsChanged();
			}
			break;
		}
	};
}

void KPCIImagesListModel::OnImportFiles()
{
	KxFileBrowseDialog dialog(KMainWindow::GetInstance(), KxID_NONE, KxFBD_OPEN_FOLDER);
	if (dialog.ShowModal() == KxID_OK)
	{
		GetView()->Freeze();
		wxString source = dialog.GetResult();
		auto operation = new KOperationWithProgressDialog<KxFileOperationEvent>(true, GetView());
		operation->OnRun([this, source](KOperationWithProgressBase* self)
		{
			KxEvtFile source(source);
			self->LinkHandler(&source, KxEVT_FILEOP_SEARCH);
			KxStringVector files = source.Find(Kortex::IScreenshotsGallery::GetSupportedExtensions(), KxFS_FILE, true);

			size_t count = files.size();
			size_t processed = 0;
			for (const wxString& path: files)
			{
				if (self->CanContinue())
				{
					GetDataVector()->emplace_back(KPPIImageEntry(path, wxEmptyString, true));
				}
				else
				{
					break;
				}
			}
		});
		operation->OnEnd([this](KOperationWithProgressBase* self)
		{
			RefreshItems();
			ChangeNotify();
			GetView()->Thaw();
		});
		operation->SetDialogCaption(KTr("Generic.FileSearchInProgress"));
		operation->Run();
	}
}
void KPCIImagesListModel::OnAddMultipleItems()
{
	KxFileBrowseDialog dialog(GetView(), KxID_NONE, KxFBD_OPEN);
	dialog.SetOptionEnabled(KxFBD_ALLOW_MULTISELECT);
	dialog.AddFilter(KxString::Join(Kortex::IScreenshotsGallery::GetSupportedExtensions(), ";"), KTr("FileFilter.Images"));
	dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
	if (dialog.ShowModal() == KxID_OK)
	{
		wxWindowUpdateLocker lock(GetView());
		for (const wxString& path: dialog.GetResults())
		{
			GetDataVector()->emplace_back(KPPIImageEntry(path, wxEmptyString, true));
			NotifyAddedItem(GetItem(GetItemCount() - 1));
		}
	}
}
void KPCIImagesListModel::OnRemoveEntry(const KxDataViewItem& item)
{
	if (KPPIImageEntry* entry = GetDataEntry(GetRow(item)))
	{
		TrackRemoveID(entry->GetPath());
		RemoveItemAndNotify(*GetDataVector(), item);
	}
}
void KPCIImagesListModel::OnClearList()
{
	for (size_t i = 0; i < GetItemCount(); i++)
	{
		TrackRemoveID(GetDataEntry(i)->GetPath());
	}
	ClearItemsAndNotify(*GetDataVector());
}

void KPCIImagesListModel::SetProject(KPackageProject& projectData)
{
	m_Interface = &projectData.GetInterface();
}
