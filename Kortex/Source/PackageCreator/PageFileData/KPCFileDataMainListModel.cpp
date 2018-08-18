#include "stdafx.h"
#include "KPCFileDataMainListModel.h"
#include "KPCFileDataFolderContentModel.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "PackageCreator/KPackageCreatorController.h"
#include "UI/KMainWindow.h"
#include "KApp.h"
#include "KOperationWithProgress.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileOperationEvent.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxTextBoxDialog.h>
#include <KxFramework/KxString.h>

enum ColumnID
{
	ID,
	Source,
	Destination,
	Priority,
};
enum MenuID
{
	AddFile,
	AddFolder,
	AddMultipleFolders,
	ReplaceFolderContent,
};

void KPCFileDataMainListModel::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KPCFileDataMainListModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KPCFileDataMainListModel::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KPCFileDataMainListModel::OnContextMenu, this);

	GetView()->AppendColumn<KxDataViewBitmapTextRenderer, KxDataViewTextEditor>(T("PackageCreator.PageFileData.MainList.InPackagePath"), ColumnID::ID, KxDATAVIEW_CELL_EDITABLE, 175);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("PackageCreator.PageFileData.MainList.Source"), ColumnID::Source, KxDATAVIEW_CELL_INERT, 150);
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("PackageCreator.PageFileData.MainList.Destination"), ColumnID::Destination, KxDATAVIEW_CELL_EDITABLE, 150);
	
	// Priority
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewSpinEditor>(T("Generic.Priority"), ColumnID::Priority, KxDATAVIEW_CELL_EDITABLE, 50);
		m_PriorityRenderer = info.GetEditor();
		m_PriorityRenderer->SetIntergerType();
		m_PriorityRenderer->SetRangeInt(KPackageProjectFileData::ms_MinUserPriority, KPackageProjectFileData::ms_MaxUserPriority);

		wxIntegerValidator<int32_t> tValidator;
		tValidator.SetMin(m_PriorityRenderer->GetMinInt());
		tValidator.SetMax(m_PriorityRenderer->GetMaxInt());
		m_PriorityRenderer->SetValidator(tValidator);
	}
}

void KPCFileDataMainListModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KPPFFileEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::ID:
			{
				value = entry->GetID();
				break;
			}
			case ColumnID::Source:
			{
				value = entry->GetSource();
				break;
			}
			case ColumnID::Destination:
			{
				value = entry->GetDestination();
				break;
			}
			case ColumnID::Priority:
			{
				value = entry->GetPriority();
				break;
			}
		};
	}
}
void KPCFileDataMainListModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KPPFFileEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::ID:
			{
				value = KxDataViewBitmapTextValue(entry->GetID(), KGetBitmap(entry->ToFolderEntry() ? KIMG_FOLDER : KIMG_DOCUMENT));
				break;
			}
			case ColumnID::Source:
			{
				value = entry->GetSource();
				break;
			}
			case ColumnID::Destination:
			{
				value = entry->GetDestination();
				break;
			}
			case ColumnID::Priority:
			{
				value = entry->GetPriority();
				break;
			}
		};
	}
}
bool KPCFileDataMainListModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	KPPFFileEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::ID:
			{
				wxString newID = value.As<wxString>();
				if (newID != entry->GetID())
				{
					if (!KPackageProjectFileData::IsFileIDValid(newID))
					{
						KPackageCreatorPageBase::ShowTooltipWarning(GetView(), T("Generic.IDInvalid"), GetItemRect(GetItem(row), column));
					}
					else if (m_FileData->HasEntryWithID(newID, entry))
					{
						KPackageCreatorPageBase::WarnIDCollision(GetView(), GetItemRect(GetItem(row), column));
					}
					else
					{
						TrackChangeID(entry->GetID(), newID);
						entry->SetID(newID);
						ChangeNotify();
						return true;
					}
				}
				return false;
			}
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
			case ColumnID::Priority:
			{
				entry->SetPriority(value.As<int32_t>());
				ChangeNotify();
				return true;
			}
		};
	}
	return false;
}

void KPCFileDataMainListModel::AddEverythingFromPath(const wxString& filePath, KPPFFolderEntry* fileEntry, KOperationWithProgressBase* context)
{
	KxEvtFile source(filePath);
	context->LinkHandler(&source, KxEVT_FILEOP_SEARCH);
	KxStringVector files = source.Find(KxFile::NullFilter, KxFS_FILE, true);

	KPPFFolderItemsArray& tEntryFiles = fileEntry->GetFiles();
	for (const wxString& path: files)
	{
		wxString target = path;
		target.Remove(0, filePath.Length());
		if (!target.IsEmpty() && target[0] == '\\')
		{
			target.Remove(0, 1);
		}

		KPPFFolderEntryItem& entry = tEntryFiles.emplace_back(KPPFFolderEntryItem());
		entry.SetSource(path);
		entry.SetDestination(target);
	}
}
bool KPCFileDataMainListModel::DoTrackID(const wxString& trackedID, const wxString& newID, bool remove)
{
	// Required files
	KxStringVector& tRequiredFiles = GetProject().GetComponents().GetRequiredFileData();
	TrackID_ReplaceOrRemove(trackedID, newID, tRequiredFiles, remove);

	// Manual components
	for (auto& step: GetProject().GetComponents().GetSteps())
	{
		for (auto& group: step->GetGroups())
		{
			for (auto& entry: group->GetEntries())
			{
				TrackID_ReplaceOrRemove(trackedID, newID, entry->GetFileData(), remove);
			}
		}
	}

	// Conditional install
	for (auto& step: GetProject().GetComponents().GetConditionalSteps())
	{
		TrackID_ReplaceOrRemove(trackedID, newID, step->GetEntries(), remove);
	}

	return true;
}

void KPCFileDataMainListModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewColumn* column = event.GetColumn();
	if (column)
	{
		switch (column->GetID())
		{
			case ColumnID::Source:
			{
				KPPFFileEntry* entry = GetDataEntry(GetRow(event.GetItem()));
				if (entry)
				{
					KxFileBrowseDialog dialog(KApp::Get().GetMainWindow(), KxID_NONE, KxFBD_OPEN);
					if (entry->ToFolderEntry())
					{
						dialog.SetFolder(entry->GetSource());
						dialog.SetOptionEnabled(KxFBD_PICK_FOLDERS);
					}
					else
					{
						dialog.SetFolder(entry->GetSource().BeforeLast('\\'));
						dialog.AddFilter("*", T("FileFilter.AllFiles"));
					}

					if (dialog.ShowModal() == KxID_OK)
					{
						entry->SetSource(dialog.GetResult());
						NotifyChangedItem(event.GetItem());
					}
				}
				break;
			}
			default:
			{
				GetView()->EditItem(event.GetItem(), column);
				break;
			}
		};
	}
}
void KPCFileDataMainListModel::OnSelectItem(KxDataViewEvent& event)
{
	if (m_ContentModel)
	{
		KxDataViewItem item = event.GetItem();
		wxWindowUpdateLocker lock(m_ContentModel->GetView());

		if (IsItemValid(item))
		{
			KPPFFolderEntry* folderEntry = GetDataEntry(GetRow(item))->ToFolderEntry();
			if (folderEntry)
			{
				m_ContentModel->SetDataVector(folderEntry);
				m_ContentModel->SelectItem();
				return;
			}
		}
		m_ContentModel->SetDataVector();
	}
}
void KPCFileDataMainListModel::OnContextMenu(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KPPFFileEntry* entry = GetDataEntry(GetRow(item));

	KxMenu menu;
	{
		KxMenu* allItems = CreateAllItemsMenu(menu);
		CreateAllItemsMenuEntry(allItems, ColumnID::Destination);
		CreateAllItemsMenuEntry(allItems, ColumnID::Priority);
		menu.AddSeparator();
	}

	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddFile, T("PackageCreator.AddFile")));
		item->SetBitmap(KGetBitmap(KIMG_DOCUMENT_PLUS));
	}
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddFolder, T("PackageCreator.AddFolder")));
		item->SetBitmap(KGetBitmap(KIMG_FOLDER_PLUS));
	}
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddMultipleFolders, T("PackageCreator.AddMultipleFolders")));
		item->SetBitmap(KGetBitmap(KIMG_FOLDERS_PLUS));
	}
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::ReplaceFolderContent, T("PackageCreator.PageFileData.ReplaceFolderContent")));
		item->Enable(entry && entry->ToFolderEntry());
		item->SetBitmap(KGetBitmap(KIMG_FOLDER_ARROW));
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
		case MenuID::AddFile:
		{
			OnAddFile();
			break;
		}
		case MenuID::AddFolder:
		{
			OnAddFolder();
			break;
		}
		case MenuID::AddMultipleFolders:
		{
			OnAddMultipleFolders();
			break;
		}
		case MenuID::ReplaceFolderContent:
		{
			OnReplaceFolderContent(item, entry->ToFolderEntry());
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
void KPCFileDataMainListModel::OnAllItemsMenuSelect(KxDataViewColumn* column)
{
	switch (column->GetID())
	{
		case ColumnID::Destination:
		{
			KxTextBoxDialog dialog(GetViewTLW(), KxID_NONE, column->GetTitle());
			if (dialog.ShowModal() == KxID_OK)
			{
				for (auto& entry: *GetDataVector())
				{
					entry->SetDestination(dialog.GetValue());
				}
				NotifyAllItemsChanged();
			}
			break;
		}
		case ColumnID::Priority:
		{
			KxTextBoxDialog dialog(GetViewTLW(), KxID_NONE, column->GetTitle());
			dialog.GetDialogMainCtrl()->SetValidator(m_PriorityRenderer->GetValidator());
			if (dialog.ShowModal() == KxID_OK)
			{
				long value = -2;
				if (dialog.GetValue().ToCLong(&value) && value > -2)
				{
					for (auto& entry: *GetDataVector())
					{
						entry->SetPriority(value);
					}
					NotifyAllItemsChanged();
				}
			}
			break;
		}
	}
}

void KPCFileDataMainListModel::OnAddFile()
{
	KxFileBrowseDialog dialog(KApp::Get().GetMainWindow(), KxID_NONE, KxFBD_OPEN);
	dialog.AddFilter("*", T("FileFilter.AllFiles"));
	if (dialog.ShowModal() == KxID_OK)
	{
		wxString source = dialog.GetResult();
		wxString target = source.AfterLast('\\');
		bool bCanUseID = m_FileData->CanUseThisIDForNewEntry(target);

		KPPFFileEntry* entry = m_FileData->AddFile(new KPPFFileEntry());
		entry->SetSource(source);
		entry->SetDestination(target);
		
		// Select ID
		entry->SetID(target);
		if (!bCanUseID)
		{
			entry->MakeUniqueID();
		}

		KxDataViewItem item = GetItem(GetItemCount() - 1);
		NotifyAddedItem(item);
		SelectItem(item);
	}
}
void KPCFileDataMainListModel::OnAddFolder()
{
	KxFileBrowseDialog dialog(KApp::Get().GetMainWindow(), KxID_NONE, KxFBD_OPEN_FOLDER);
	if (dialog.ShowModal() == KxID_OK)
	{
		wxString source = dialog.GetResult();
		wxString id = source.AfterLast('\\');
		bool bCanUseID = m_FileData->CanUseThisIDForNewEntry(id);

		KPPFFolderEntry* entry = m_FileData->AddFolder(new KPPFFolderEntry());
		entry->SetID(id);
		entry->SetSource(source);

		// Select ID
		if (!bCanUseID)
		{
			entry->MakeUniqueID();
		}

		auto pOperation = new KOperationWithProgressDialog<KxFileOperationEvent>(true, GetView());
		pOperation->OnRun([this, entry, source](KOperationWithProgressBase* self)
		{
			AddEverythingFromPath(source, entry, self);
		});
		pOperation->OnEnd([this](KOperationWithProgressBase* self)
		{
			KxDataViewItem item = GetItem(GetItemCount() - 1);
			NotifyAddedItem(item);
			SelectItem(item);
		});
		pOperation->SetDialogCaption(T("Generic.FileFindInProgress"));
		pOperation->Run();
	}
}
void KPCFileDataMainListModel::OnAddMultipleFolders()
{
	KxFileBrowseDialog dialog(KApp::Get().GetMainWindow(), KxID_NONE, KxFBD_OPEN_FOLDER);
	if (dialog.ShowModal() == KxID_OK)
	{
		wxString source = dialog.GetResult();
		auto pOperation = new KOperationWithProgressDialog<KxFileOperationEvent>(true, GetView());
		pOperation->OnRun([this, source](KOperationWithProgressBase* self)
		{
			KxStringVector folders = KxFile(source).Find(KxFile::NullFilter, KxFS_FOLDER, false);
			for (const wxString folderPath: folders)
			{
				wxString id = folderPath.AfterLast('\\');
				bool bCanUseID = m_FileData->CanUseThisIDForNewEntry(id);

				KPPFFolderEntry* entry = m_FileData->AddFolder(new KPPFFolderEntry());
				entry->SetID(id);
				entry->SetSource(folderPath);

				if (!bCanUseID)
				{
					entry->MakeUniqueID();
				}

				AddEverythingFromPath(folderPath, entry, self);
			}
		});
		pOperation->OnEnd([this](KOperationWithProgressBase* self)
		{
			RefreshItems();
			ChangeNotify();
		});
		pOperation->SetDialogCaption(T("Generic.FileFindInProgress"));
		pOperation->Run();
	}
}
void KPCFileDataMainListModel::OnReplaceFolderContent(const KxDataViewItem& item, KPPFFolderEntry* folderEntry)
{
	KxFileBrowseDialog dialog(GetViewTLW(), KxID_NONE, KxFBD_OPEN_FOLDER);
	if (dialog.ShowModal() == KxID_OK)
	{
		wxString source = dialog.GetResult();

		auto pOperation = new KOperationWithProgressDialog<KxFileOperationEvent>(true, GetView());
		pOperation->OnRun([this, folderEntry, source](KOperationWithProgressBase* self)
		{
			folderEntry->GetFiles().clear();
			folderEntry->SetSource(source);
			AddEverythingFromPath(source, folderEntry, self);
		});
		pOperation->OnEnd([this, item](KOperationWithProgressBase* self)
		{
			NotifyChangedItem(item);
			SelectItem(item);
		});
		pOperation->SetDialogCaption(T("Generic.FileFindInProgress"));
		pOperation->Run();
	}
}
void KPCFileDataMainListModel::OnRemoveElement(const KxDataViewItem& item)
{
	if (KPPFFileEntry* entry = GetDataEntry(item))
	{
		TrackRemoveID(entry->GetID());
		RemoveItemAndNotify(*GetDataVector(), item);
	}
}
void KPCFileDataMainListModel::OnClearList()
{
	for (size_t i = 0; i < GetItemCount(); i++)
	{
		TrackRemoveID(GetDataEntry(i)->GetID());
	}
	ClearItemsAndNotify(*GetDataVector());
}

void KPCFileDataMainListModel::SetProject(KPackageProject& projectData)
{
	m_FileData = &projectData.GetFileData();
}
