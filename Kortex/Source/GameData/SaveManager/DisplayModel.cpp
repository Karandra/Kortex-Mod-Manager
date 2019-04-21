#include "stdafx.h"
#include <Kortex/SaveManager.hpp>
#include <Kortex/Application.hpp>
#include "UI/KImageViewerDialog.h"
#include "Utility/KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxComparator.h>
#include <KxFramework/DataView/KxDataViewMainWindow.h>

namespace Kortex::SaveManager
{
	enum ColumnID
	{
		Bitmap,
		Name,
		ModificationDate,
		Size,
	};

	void DisplayModel::OnInitControl()
	{
		GetView()->SetUniformRowHeight(m_BitmapSize.GetHeight());
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &DisplayModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &DisplayModel::OnSelectItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &DisplayModel::OnContextMenu, this);
		GetView()->Bind(KxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, &DisplayModel::OnHeaderContextMenu, this);
		GetView()->Bind(KxEVT_DATAVIEW_CACHE_HINT, &DisplayModel::OnCacheHint, this);

		// Columns
		KxDataViewColumnFlags flags = KxDV_COL_DEFAULT_FLAGS|KxDV_COL_SORTABLE;
		{
			auto info = GetView()->AppendColumn<KxDataViewBitmapRenderer>(KTr("Generic.Image"), ColumnID::Bitmap, KxDATAVIEW_CELL_INERT, m_BitmapSize.GetWidth(), KxDV_COL_REORDERABLE);
			m_BitmapColumn = info.GetColumn();
		}
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 200, flags);
		{
			auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.ModificationDate"), ColumnID::ModificationDate, KxDATAVIEW_CELL_INERT, 100, flags);
			info.GetColumn()->SortDescending();
		}
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Size"), ColumnID::Size, KxDATAVIEW_CELL_INERT, 100, flags);
	}
	void DisplayModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		const IGameSave* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					wxString name = entry->GetFileItem().GetName().BeforeLast('.');
					value = !name.IsEmpty() ? name : entry->GetFileItem().GetName();
					return;
				}
			};
		}
	}
	void DisplayModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		IGameSave* entry = GetDataEntry(row);
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
					value = entry->GetDisplayName();
					break;
				}
				case ColumnID::ModificationDate:
				{
					value = KAux::FormatDateTime(entry->GetFileItem().GetModificationTime());
					break;
				}
				case ColumnID::Size:
				{
					value = KxFile::FormatFileSize(entry->GetFileItem().GetFileSize(), 2);
					break;
				}
			};
		}
	}
	bool DisplayModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
	{
		IGameSave* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					const Config& savesConfig = m_Manager->GetConfig();
					KxFileItem& infoPrimary = entry->GetFileItem();

					if (savesConfig.HasMultiFileSaveConfig())
					{
						KxFileItem infoSecondary(infoPrimary);
						KxFileItem newInfoPrimary(infoPrimary);
						KxFileItem newInfoSecondary(infoPrimary);

						// Set new name
						infoSecondary.SetName(infoPrimary.GetName().BeforeLast('.') + '.' + savesConfig.GetSecondarySaveExtension());
						newInfoPrimary.SetName(value.As<wxString>() + '.' + savesConfig.GetPrimarySaveExtension());
						newInfoSecondary.SetName(value.As<wxString>() + '.' + savesConfig.GetSecondarySaveExtension());

						newInfoPrimary.UpdateInfo();
						newInfoSecondary.UpdateInfo();
						if (!newInfoPrimary.IsOK() && !newInfoSecondary.IsOK())
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
						KxFileItem newPrimaryInfo(infoPrimary);

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
	bool DisplayModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
	{
		const IGameSave* entry = GetDataEntry(row);
		return entry && entry->IsOK();
	}
	bool DisplayModel::CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const
	{
		const IGameSave* entry1 = GetDataEntry(row1);
		const IGameSave* entry2 = GetDataEntry(row2);

		if (entry1 && entry2)
		{
			switch (column ? column->GetID() : ColumnID::ModificationDate)
			{
				case ColumnID::Name:
				{
					return KxComparator::IsLess(entry1->GetFileItem().GetName(), entry2->GetFileItem().GetName());
				}
				case ColumnID::Size:
				{
					return entry1->GetFileItem().GetFileSize() < entry2->GetFileItem().GetFileSize();
				}
				case ModificationDate:
				{
					return entry1->GetFileItem().GetModificationTime() < entry2->GetFileItem().GetModificationTime();
				}
			};
		}
		return false;
	}

	void DisplayModel::OnSelectItem(KxDataViewEvent& event)
	{
		m_Workspace->ProcessSelection(GetDataEntry(GetRow(event.GetItem())));
	}
	void DisplayModel::OnActivateItem(KxDataViewEvent& event)
	{
		KxDataViewColumn* column = event.GetColumn();
		const IGameSave* entry = GetDataEntry(GetRow(event.GetItem()));
		if (column && entry && entry->IsOK())
		{
			switch (column->GetID())
			{
				case ColumnID::Bitmap:
				{
					KImageViewerDialog dialog(GetViewTLW(), entry->GetFileItem().GetName());

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
	void DisplayModel::OnContextMenu(KxDataViewEvent& event)
	{
		m_Workspace->ProcessContextMenu(GetDataEntry(GetRow(event.GetItem())));
	}
	void DisplayModel::OnHeaderContextMenu(KxDataViewEvent& event)
	{
		KxMenu menu;
		if (GetView()->CreateColumnSelectionMenu(menu))
		{
			GetView()->OnColumnSelectionMenu(menu);
			UpdateRowHeight();
		}
	}
	void DisplayModel::OnCacheHint(KxDataViewEvent& event)
	{
		if (m_BitmapColumn->IsVisible())
		{
			for (size_t row = event.GetCacheHintFrom(); row <= event.GetCacheHintTo(); row++)
			{
				KxDataViewItem item = GetView()->GetMainWindow()->GetItemByRow(row);
				IGameSave* entry = GetDataEntry(GetRow(item));
				if (entry && (!entry->IsOK() || !entry->HasThumbBitmap()))
				{
					entry->ReadFile();
					entry->SetThumbBitmap(m_BitmapSize.ScaleMaintainRatio(entry->GetBitmap(), 0, 4));
				}
			}
		}
	}

	DisplayModel::DisplayModel(ISaveManager* manager, Workspace* workspace)
		:m_Manager(manager), m_Workspace(workspace)
	{
		m_BitmapSize.FromHeight(78, KBitmapSize::r16_9);
		SetDataViewFlags(KxDataViewCtrl::DefaultStyle|KxDV_MULTIPLE_SELECTION);
	}

	void DisplayModel::SetDataVector()
	{
		m_DataVector.clear();
		KDataViewVectorListModel::SetDataVector();
	}
	void DisplayModel::SetDataVector(const wxString& folder, const KxStringVector& filtersList)
	{
		m_DataVector.clear();
		for (const wxString& filter: filtersList)
		{
			KxFileFinder finder(folder, filter);
			KxFileItem item;
			do
			{
				item = finder.FindNext();
				if (item.IsOK())
				{
					auto& entry = ISaveManager::GetInstance()->NewSave();
					if (entry && entry->Create(item.GetFullPath()))
					{
						m_DataVector.emplace_back(std::move(entry));
					}
				}
			}
			while (item.IsOK());
		}
		KxDataViewVectorListModelEx::SetDataVector(&m_DataVector);
	}

	void DisplayModel::UpdateRowHeight()
	{
		KxDataViewColumn* column = GetView()->GetColumnByID(ColumnID::Bitmap);
		if (column)
		{
			if (column->IsVisible())
			{
				GetView()->SetUniformRowHeight(m_BitmapSize.GetHeight());
			}
			else
			{
				GetView()->SetUniformRowHeight(-1);
			}
		}
	}
}
