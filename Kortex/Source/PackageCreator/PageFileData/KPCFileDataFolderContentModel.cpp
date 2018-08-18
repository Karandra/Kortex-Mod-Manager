#include "stdafx.h"
#include "KPCFileDataFolderContentModel.h"
#include "PackageProject/KPackageProject.h"
#include "UI/KMainWindow.h"
#include "KApp.h"
#include <KxFramework/KxFileBrowseDialog.h>

enum ColumnID
{
	Source,
	Destination,
};
enum MenuID
{
	AddMultipleFiles,
};

void KPCFileDataFolderContentModel::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KPCFileDataFolderContentModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KPCFileDataFolderContentModel::OnContextMenu, this);

	GetView()->AppendColumn<KxDataViewTextRenderer>(T("PackageCreator.PageFileData.MainList.Source"), ColumnID::Source, KxDATAVIEW_CELL_INERT, 300);
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("PackageCreator.PageFileData.MainList.Destination"), ColumnID::Destination, KxDATAVIEW_CELL_EDITABLE, 300);
}

void KPCFileDataFolderContentModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KPPFFolderEntryItem* entry = GetDataEntry(row);
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
void KPCFileDataFolderContentModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KPPFFolderEntryItem* entry = GetDataEntry(row);
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
bool KPCFileDataFolderContentModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	KPPFFolderEntryItem* entry = GetDataEntry(row);
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

void KPCFileDataFolderContentModel::OnActivateItem(KxDataViewEvent& event)
{
	if (event.GetColumn())
	{
		if (event.GetColumn()->GetID() == ColumnID::Source)
		{
			KPPFFolderEntryItem* entry = GetDataEntry(GetRow(event.GetItem()));
			if (entry)
			{
				KxFileBrowseDialog dialog(KApp::Get().GetMainWindow(), KxID_NONE, KxFBD_OPEN);
				dialog.SetFolder(entry->GetSource().BeforeLast('\\'));
				dialog.AddFilter("*", T("FileFilter.AllFiles"));
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
void KPCFileDataFolderContentModel::OnContextMenu(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	const KPPFFolderEntryItem* entry = GetDataEntry(GetRow(item));

	KxMenu menu;
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddMultipleFiles, T("PackageCreator.AddMultipleFiles")));
		item->SetBitmap(KGetBitmap(KIMG_DOCUMENTS_PLUS));
	}
	menu.AddSeparator();
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(KxID_REMOVE, T(KxID_REMOVE)));
		item->Enable(entry != NULL);
	}
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(KxID_CLEAR, T(KxID_CLEAR)));
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

void KPCFileDataFolderContentModel::OnAddMultipleFiles()
{
	KxFileBrowseDialog dialog(KApp::Get().GetMainWindow(), KxID_NONE, KxFBD_OPEN);
	dialog.AddFilter("*", T("FileFilter.AllFiles"));
	dialog.SetOptionEnabled(KxFBD_ALLOW_MULTISELECT);
	if (dialog.ShowModal() == KxID_OK)
	{
		wxWindowUpdateLocker lock(GetView());
		for (const wxString& source: dialog.GetResults())
		{
			KPPFFolderEntryItem& entry = GetDataVector()->emplace_back();
			entry.SetSource(source);
			entry.SetDestination(source.AfterLast('\\'));

			NotifyAddedItem(GetItem(GetItemCount() - 1));
		}
	}
}
void KPCFileDataFolderContentModel::OnRemoveElement(const KxDataViewItem& item)
{
	RemoveItemAndNotify(*GetDataVector(), item);
}
void KPCFileDataFolderContentModel::OnClearList()
{
	ClearItemsAndNotify(*GetDataVector());
}

void KPCFileDataFolderContentModel::SetProject(KPackageProject& projectData)
{
	m_FileData = &projectData.GetFileData();
}
