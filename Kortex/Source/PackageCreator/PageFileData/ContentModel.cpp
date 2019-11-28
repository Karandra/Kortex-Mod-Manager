#include "stdafx.h"
#include "ContentModel.h"
#include "PackageProject/KPackageProject.h"
#include "Application/Resources/ImageResourceID.h"
#include "Application/Resources/IImageProvider.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxFileBrowseDialog.h>

namespace
{
	enum ColumnID
	{
		Source,
		Destination,
	};
	enum MenuID
	{
		AddMultipleFiles,
	};
}

namespace Kortex::PackageDesigner::PageFileDataNS
{
	void ContentModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &ContentModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &ContentModel::OnContextMenu, this);
	
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("PackageCreator.PageFileData.MainList.Source"), ColumnID::Source, KxDATAVIEW_CELL_INERT, 300);
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("PackageCreator.PageFileData.MainList.Destination"), ColumnID::Destination, KxDATAVIEW_CELL_EDITABLE, 300);
	}
	
	void ContentModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		const PackageProject::KPPFFolderEntryItem* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Source:
				{
					value = entry->GetSource();
					return;
				}
				case ColumnID::Destination:
				{
					value = entry->GetDestination();
					return;
				}
			};
		}
	}
	void ContentModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		const PackageProject::KPPFFolderEntryItem* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Source:
				{
					value = entry->GetSource();
					break;
				}
				case ColumnID::Destination:
				{
					const wxString& sFolderDest = m_Folder->GetDestination();
					const wxString& sFileDest = entry->GetDestination();
					if (sFolderDest.IsEmpty())
					{
						value = sFileDest;
					}
					else if (!sFolderDest.IsEmpty() && sFolderDest.Last() != '\\' && !sFileDest.IsEmpty() && sFileDest[0] != '\\')
					{
						value = sFolderDest + '\\' + sFileDest;
					}
					else
					{
						value = sFolderDest + sFileDest;
					}
					break;
				}
			};
		}
	}
	bool ContentModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
	{
		PackageProject::KPPFFolderEntryItem* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Source:
				{
					entry->SetSource(value.As<wxString>());
					ChangeNotify();
					return true;
				}
				case ColumnID::Destination:
				{
					entry->SetDestination(value.As<wxString>());
					ChangeNotify();
					return true;
				}
			};
		}
		return false;
	}
	
	void ContentModel::OnActivateItem(KxDataViewEvent& event)
	{
		if (event.GetColumn())
		{
			if (event.GetColumn()->GetID() == ColumnID::Source)
			{
				PackageProject::KPPFFolderEntryItem* entry = GetDataEntry(GetRow(event.GetItem()));
				if (entry)
				{
					KxFileBrowseDialog dialog(GetView(), KxID_NONE, KxFBD_OPEN);
					dialog.SetFolder(entry->GetSource().BeforeLast('\\'));
					dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
					if (dialog.ShowModal() == KxID_OK)
					{
						entry->SetSource(dialog.GetResult());
						NotifyChangedItem(event.GetItem());
					}
				}
			}
			else
			{
				GetView()->EditItem(event.GetItem(), event.GetColumn());
			}
		}
	}
	void ContentModel::OnContextMenu(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		const PackageProject::KPPFFolderEntryItem* entry = GetDataEntry(GetRow(item));
	
		KxMenu menu;
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddMultipleFiles, KTr("PackageCreator.AddMultipleFiles")));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::DocumentsPlus));
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
			case MenuID::AddMultipleFiles:
			{
				OnAddMultipleFiles();
				break;
			}
			case KxID_REMOVE:
			{
				OnRemoveElement(item);
				break;
			}
			case KxID_CLEAR:
			{
				OnClearList();
				break;
			}
		};
	}
	
	void ContentModel::OnAddMultipleFiles()
	{
		KxFileBrowseDialog dialog(GetView(), KxID_NONE, KxFBD_OPEN);
		dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
		dialog.SetOptionEnabled(KxFBD_ALLOW_MULTISELECT);
		if (dialog.ShowModal() == KxID_OK)
		{
			wxWindowUpdateLocker lock(GetView());
			for (const wxString& source: dialog.GetResults())
			{
				PackageProject::KPPFFolderEntryItem& entry = GetDataVector()->emplace_back();
				entry.SetSource(source);
				entry.SetDestination(source.AfterLast('\\'));
	
				NotifyAddedItem(GetItem(GetItemCount() - 1));
			}
		}
	}
	void ContentModel::OnRemoveElement(const KxDataViewItem& item)
	{
		RemoveItemAndNotify(*GetDataVector(), item);
	}
	void ContentModel::OnClearList()
	{
		ClearItemsAndNotify(*GetDataVector());
	}
	
	void ContentModel::SetProject(KPackageProject& projectData)
	{
		m_FileData = &projectData.GetFileData();
	}
}
