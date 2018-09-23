#include "stdafx.h"
#include "KSaveManagerListModel.h"
#include "KSaveManager.h"
#include "KSaveManagerWorkspace.h"
#include "Profile/KSaveManagerConfig.h"
#include "UI/KImageViewerDialog.h"
#include "KApp.h"
#include "KAux.h"
#include "KComparator.h"
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/DataView/KxDataViewMainWindow.h>

enum ColumnID
{
	Bitmap,
	Name,
	ModificationDate,
	Size,
};

void KSaveManagerListModel::OnInitControl()
{
	GetView()->SetUniformRowHeight(m_RowHeight);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KSaveManagerListModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KSaveManagerListModel::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KSaveManagerListModel::OnContextMenu, this);
	GetView()->Bind(KxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, &KSaveManagerListModel::OnHeaderContextMenu, this);
	GetView()->Bind(KxEVT_DATAVIEW_CACHE_HINT, &KSaveManagerListModel::OnCacheHint, this);

	// Columns
	KxDataViewColumnFlags flags = KxDV_COL_DEFAULT_FLAGS|KxDV_COL_SORTABLE;

	GetView()->AppendColumn<KxDataViewBitmapRenderer>(wxEmptyString, ColumnID::Bitmap, KxDATAVIEW_CELL_INERT, ms_BitmapWidth, KxDV_COL_REORDERABLE);
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 200, flags);
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.ModificationDate"), ColumnID::ModificationDate, KxDATAVIEW_CELL_INERT, 100, flags);
		info.GetColumn()->SortDescending();
	}
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Size"), ColumnID::Size, KxDATAVIEW_CELL_INERT, 100, flags);
}
void KSaveManagerListModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KSaveFile* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				wxString name = entry->GetFileInfo().GetName().BeforeLast('.');
				value = !name.IsEmpty() ? name : entry->GetFileInfo().GetName();
				return;
			}
		};
	}
}
void KSaveManagerListModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	KSaveFile* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Bitmap:
			{
				const wxBitmap& bitmap = entry->GetThumbBitmap();
				value = bitmap.IsOk() ? bitmap : KGetBitmap(KIMG_CROSS_WHITE);
				break;
			}
			case ColumnID::Name:
			{
				value = entry->GetFileInfo().GetName();
				break;
			}
			case ColumnID::ModificationDate:
			{
				value = KAux::FormatDateTime(entry->GetFileInfo().GetModificationTime());
				break;
			}
			case ColumnID::Size:
			{
				value = KxFile::FormatFileSize(entry->GetFileInfo().GetFileSize(), 2);
				break;
			}
		};
	}
}
bool KSaveManagerListModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	KSaveFile* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				const KSaveManagerConfig* savesConfig = KSaveManagerConfig::GetInstance();
				KxFileFinderItem& infoPrimary = entry->GetFileInfo();

				if (savesConfig->HasMultiFileSaveConfig())
				{
					KxFileFinderItem infoSecondary(infoPrimary);
					KxFileFinderItem newInfoPrimary(infoPrimary);
					KxFileFinderItem newInfoSecondary(infoPrimary);

					// Set new name
					infoSecondary.SetName(infoPrimary.GetName().BeforeLast('.') + '.' + savesConfig->GetSecondarySaveExtension());
					newInfoPrimary.SetName(value.As<wxString>() + '.' + savesConfig->GetPrimarySaveExtension());
					newInfoSecondary.SetName(value.As<wxString>() + '.' + savesConfig->GetSecondarySaveExtension());

					newInfoPrimary.UpdateInfo();
					newInfoSecondary.UpdateInfo();
					if (!newInfoPrimary.IsOK() && !newInfoPrimary.IsOK())
					{
						bool b1 = KxFile(infoPrimary.GetFullPath()).Rename(newInfoPrimary.GetFullPath(), false);
						bool b2 = KxFile(infoSecondary.GetFullPath()).Rename(newInfoSecondary.GetFullPath(), false);
						if (b1 && b2)
						{
							// Secondary info is not referring to currently known KxDataViewItem
							// so don't even try to find it.
							infoPrimary = newInfoPrimary;
							infoPrimary.UpdateInfo();
							m_Workspace->ProcessSelection(entry);
							return true;
						}
					}
				}
				else
				{
					KxFileFinderItem newPrimaryInfo(infoPrimary);

					wxString ext = infoPrimary.GetName().AfterLast('.');
					if (ext != infoPrimary.GetName())
					{
						newPrimaryInfo.SetName(value.As<wxString>() + '.' + ext);
					}
					else
					{
						newPrimaryInfo.SetName(value.As<wxString>());
					}

					newPrimaryInfo.UpdateInfo();
					if (!newPrimaryInfo.IsOK())
					{
						KxFile(infoPrimary.GetFullPath()).Rename(newPrimaryInfo.GetFullPath(), false);
						infoPrimary = newPrimaryInfo;
						m_Workspace->ProcessSelection(entry);
						return true;
					}
				}
				break;
			}
		};
	}
	return false;
}
bool KSaveManagerListModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	const KSaveFile* entry = GetDataEntry(row);
	return entry && entry->IsOK();
}
bool KSaveManagerListModel::CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const
{
	const KSaveFile* entry1 = GetDataEntry(row1);
	const KSaveFile* entry2 = GetDataEntry(row2);

	switch (column ? column->GetID() : ColumnID::ModificationDate)
	{
		case ColumnID::Name:
		{
			return KComparator::KLess(entry1->GetFileInfo().GetName(), entry2->GetFileInfo().GetName());
		}
		case ColumnID::Size:
		{
			return entry1->GetFileInfo().GetFileSize() < entry2->GetFileInfo().GetFileSize();
		}
		case ModificationDate:
		{
			return entry1->GetFileInfo().GetModificationTime() < entry2->GetFileInfo().GetModificationTime();
		}
	};
	return false;
}

void KSaveManagerListModel::OnSelectItem(KxDataViewEvent& event)
{
	m_Workspace->ProcessSelection(GetDataEntry(GetRow(event.GetItem())));
}
void KSaveManagerListModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewColumn* column = event.GetColumn();
	const KSaveFile* entry = GetDataEntry(GetRow(event.GetItem()));
	if (column && entry && entry->IsOK())
	{
		switch (column->GetID())
		{
			case ColumnID::Bitmap:
			{
				KImageViewerDialog dialog(GetViewTLW(), entry->GetFileInfo().GetName());

				KImageViewerEvent evt;
				evt.SetBitmap(entry->GetBitmap());
				dialog.Navigate(evt);

				dialog.ShowModal();
				break;
			}
			case ColumnID::Name:
			{
				GetView()->EditItem(event.GetItem(), column);
				break;
			}
		};
	}
}
void KSaveManagerListModel::OnContextMenu(KxDataViewEvent& event)
{
	m_Workspace->ProcessContextMenu(GetDataEntry(GetRow(event.GetItem())));
}
void KSaveManagerListModel::OnHeaderContextMenu(KxDataViewEvent& event)
{
	KxMenu menu;
	if (GetView()->CreateColumnSelectionMenu(menu))
	{
		GetView()->OnColumnSelectionMenu(menu);
		UpdateRowHeight();
	}
}
void KSaveManagerListModel::OnCacheHint(KxDataViewEvent& event)
{
	for (size_t row = event.GetCacheHintFrom(); row <= event.GetCacheHintTo(); row++)
	{
		KxDataViewItem item = GetView()->GetMainWindow()->GetItemByRow(row);
		KSaveFile* entry = GetDataEntry(GetRow(item));
		if (entry && (!entry->IsOK() || !entry->HasThumbBitmap()))
		{
			entry->ReadData();
			entry->CreateThumbBitmap(ms_BitmapWidth, m_RowHeight - 4);
		}
	}
}

KSaveManagerListModel::KSaveManagerListModel(KSaveManager* manager, KSaveManagerWorkspace* workspace)
	:m_Manager(manager), m_Workspace(workspace), m_RowHeight(78)
{
	ms_BitmapWidth = (m_RowHeight / 9.0) * 16.0;
	SetDataViewFlags(KxDataViewCtrl::DefaultStyle|KxDV_MULTIPLE_SELECTION);
}

void KSaveManagerListModel::SetDataVector()
{
	m_DataVector.clear();
	KDataViewVectorListModel::SetDataVector();
}
void KSaveManagerListModel::SetDataVector(const wxString& folder, const KxStringVector& filtersList)
{
	m_DataVector.clear();
	for (const wxString& filter: filtersList)
	{
		KxFileFinder tFinder(folder, filter);
		KxFileFinderItem item;
		do
		{
			item = tFinder.FindNext();
			if (item.IsOK())
			{
				m_DataVector.emplace_back(KSaveManagerConfig::GetInstance()->QuerySaveFile(item.GetFullPath()));
			}
		}
		while (item.IsOK());
	}
	KDataViewVectorListModel::SetDataVector(&m_DataVector);
}

void KSaveManagerListModel::UpdateRowHeight()
{
	KxDataViewColumn* column = GetView()->GetColumnByID(ColumnID::Bitmap);
	if (column)
	{
		if (column->IsExposed())
		{
			GetView()->SetUniformRowHeight(m_RowHeight);
		}
		else
		{
			GetView()->SetUniformRowHeight(-1);
		}
	}
}
