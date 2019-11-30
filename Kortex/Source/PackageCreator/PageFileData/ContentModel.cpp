#include "stdafx.h"
#include "ContentModel.h"
#include "PackageProject/ModPackageProject.h"
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
		if (const PackageProject::FileItem* item = GetDataEntry(row))
		{
			switch (column->GetID())
			{
				case ColumnID::Source:
				{
					value = item->GetSource();
					return;
				}
				case ColumnID::Destination:
				{
					value = item->GetDestination();
					return;
				}
			};
		}
	}
	void ContentModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		if (const PackageProject::FileItem* item = GetDataEntry(row))
		{
			switch (column->GetID())
			{
				case ColumnID::Source:
				{
					value = item->GetSource();
					break;
				}
				case ColumnID::Destination:
				{
					const wxString& sFolderDest = m_Folder->GetDestination();
					const wxString& sFileDest = item->GetDestination();
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
		if (PackageProject::FileItem* item = GetDataEntry(row))
		{
			switch (column->GetID())
			{
				case ColumnID::Source:
				{
					item->SetSource(value.As<wxString>());
					ChangeNotify();
					return true;
				}
				case ColumnID::Destination:
				{
					item->SetDestination(value.As<wxString>());
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
				if (PackageProject::FileItem* item = GetDataEntry(GetRow(event.GetItem())))
				{
					KxFileBrowseDialog dialog(GetView(), KxID_NONE, KxFBD_OPEN);
					dialog.SetFolder(item->GetSource().BeforeLast('\\'));
					dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
					if (dialog.ShowModal() == KxID_OK)
					{
						item->SetSource(dialog.GetResult());
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
		const PackageProject::FileItem* fileItem = GetDataEntry(GetRow(item));
	
		KxMenu menu;
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddMultipleFiles, KTr("PackageCreator.AddMultipleFiles")));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::DocumentsPlus));
		}
		menu.AddSeparator();
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(KxID_REMOVE, KTr(KxID_REMOVE)));
			item->Enable(fileItem != nullptr);
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
				PackageProject::FileItem& item = m_Folder->AddFile();
				item.SetSource(source);
				item.SetDestination(source.AfterLast('\\'));
				
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
	
	void ContentModel::SetProject(ModPackageProject& projectData)
	{
		m_FileData = &projectData.GetFileData();
	}
}
