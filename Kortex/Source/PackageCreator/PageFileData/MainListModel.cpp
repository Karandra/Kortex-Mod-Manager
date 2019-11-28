#include "stdafx.h"
#include "MainListModel.h"
#include "ContentModel.h"
#include "PackageCreator/PageBase.h"
#include "PackageCreator/WorkspaceDocument.h"
#include <Kortex/Application.hpp>
#include "Utility/KOperationWithProgress.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileOperationEvent.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxTextBoxDialog.h>
#include <KxFramework/KxString.h>

namespace
{
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
}

namespace Kortex::PackageDesigner::PageFileDataNS
{
	void MainListModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &MainListModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &MainListModel::OnSelectItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &MainListModel::OnContextMenu, this);
	
		GetView()->AppendColumn<KxDataViewBitmapTextRenderer, KxDataViewTextEditor>(KTr("PackageCreator.PageFileData.MainList.InPackagePath"), ColumnID::ID, KxDATAVIEW_CELL_EDITABLE, 175);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("PackageCreator.PageFileData.MainList.Source"), ColumnID::Source, KxDATAVIEW_CELL_INERT, 150);
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("PackageCreator.PageFileData.MainList.Destination"), ColumnID::Destination, KxDATAVIEW_CELL_EDITABLE, 150);
		
		// Priority
		{
			auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewSpinEditor>(KTr("Generic.Priority"), ColumnID::Priority, KxDATAVIEW_CELL_EDITABLE, 50);
			m_PriorityRenderer = info.GetEditor();
			m_PriorityRenderer->SetIntergerType();
			m_PriorityRenderer->SetRangeInt(KPackageProjectFileData::ms_MinUserPriority, KPackageProjectFileData::ms_MaxUserPriority);
	
			wxIntegerValidator<int32_t> tValidator;
			tValidator.SetMin(m_PriorityRenderer->GetMinInt());
			tValidator.SetMax(m_PriorityRenderer->GetMaxInt());
			m_PriorityRenderer->SetValidator(tValidator);
		}
	}
	
	void MainListModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
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
	void MainListModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		const KPPFFileEntry* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::ID:
				{
					value = KxDataViewBitmapTextValue(entry->GetID(), ImageProvider::GetBitmap(entry->ToFolderEntry() ? ImageResourceID::Folder : ImageResourceID::Document));
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
	bool MainListModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
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
							PageBase::ShowTooltipWarning(GetView(), KTr("Generic.IDInvalid"), GetItemRect(GetItem(row), column));
						}
						else if (m_FileData->HasEntryWithID(newID, entry))
						{
							PageBase::WarnIDCollision(GetView(), GetItemRect(GetItem(row), column));
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
	
	void MainListModel::AddEverythingFromPath(const wxString& filePath, KPPFFolderEntry* fileEntry, KOperationWithProgressBase* context)
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
	bool MainListModel::DoTrackID(const wxString& trackedID, const wxString& newID, bool remove)
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
	
	void MainListModel::OnActivateItem(KxDataViewEvent& event)
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
						KxFileBrowseDialog dialog(GetView(), KxID_NONE, KxFBD_OPEN);
						if (entry->ToFolderEntry())
						{
							dialog.SetFolder(entry->GetSource());
							dialog.SetOptionEnabled(KxFBD_PICK_FOLDERS);
						}
						else
						{
							dialog.SetFolder(entry->GetSource().BeforeLast('\\'));
							dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
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
	void MainListModel::OnSelectItem(KxDataViewEvent& event)
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
	void MainListModel::OnContextMenu(KxDataViewEvent& event)
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
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddFile, KTr("PackageCreator.AddFile")));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::DocumentPlus));
		}
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddFolder, KTr("PackageCreator.AddFolder")));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderPlus));
		}
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddMultipleFolders, KTr("PackageCreator.AddMultipleFolders")));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FoldersPlus));
		}
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::ReplaceFolderContent, KTr("PackageCreator.PageFileData.ReplaceFolderContent")));
			item->Enable(entry && entry->ToFolderEntry());
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderArrow));
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
	void MainListModel::OnAllItemsMenuSelect(KxDataViewColumn* column)
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
	
	void MainListModel::OnAddFile()
	{
		KxFileBrowseDialog dialog(GetView(), KxID_NONE, KxFBD_OPEN);
		dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
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
	void MainListModel::OnAddFolder()
	{
		KxFileBrowseDialog dialog(GetView(), KxID_NONE, KxFBD_OPEN_FOLDER);
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
	
			auto operation = new KOperationWithProgressDialog<KxFileOperationEvent>(true, GetView());
			operation->OnRun([this, entry, source](KOperationWithProgressBase* self)
			{
				AddEverythingFromPath(source, entry, self);
			});
			operation->OnEnd([this](KOperationWithProgressBase* self)
			{
				KxDataViewItem item = GetItem(GetItemCount() - 1);
				NotifyAddedItem(item);
				SelectItem(item);
			});
			operation->SetDialogCaption(KTr("Generic.FileSearchInProgress"));
			operation->Run();
		}
	}
	void MainListModel::OnAddMultipleFolders()
	{
		KxFileBrowseDialog dialog(GetView(), KxID_NONE, KxFBD_OPEN_FOLDER);
		if (dialog.ShowModal() == KxID_OK)
		{
			wxString source = dialog.GetResult();
			auto operation = new KOperationWithProgressDialog<KxFileOperationEvent>(true, GetView());
			operation->OnRun([this, source](KOperationWithProgressBase* self)
			{
				KxStringVector folders = KxFile(source).Find(KxFile::NullFilter, KxFS_FOLDER, false);
				for (const wxString& folderPath: folders)
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
			operation->OnEnd([this](KOperationWithProgressBase* self)
			{
				RefreshItems();
				ChangeNotify();
			});
			operation->SetDialogCaption(KTr("Generic.FileSearchInProgress"));
			operation->Run();
		}
	}
	void MainListModel::OnReplaceFolderContent(const KxDataViewItem& item, KPPFFolderEntry* folderEntry)
	{
		KxFileBrowseDialog dialog(GetViewTLW(), KxID_NONE, KxFBD_OPEN_FOLDER);
		if (dialog.ShowModal() == KxID_OK)
		{
			wxString source = dialog.GetResult();
	
			auto operation = new KOperationWithProgressDialog<KxFileOperationEvent>(true, GetView());
			operation->OnRun([this, folderEntry, source](KOperationWithProgressBase* self)
			{
				folderEntry->GetFiles().clear();
				folderEntry->SetSource(source);
				AddEverythingFromPath(source, folderEntry, self);
			});
			operation->OnEnd([this, item](KOperationWithProgressBase* self)
			{
				NotifyChangedItem(item);
				SelectItem(item);
			});
			operation->SetDialogCaption(KTr("Generic.FileSearchInProgress"));
			operation->Run();
		}
	}
	void MainListModel::OnRemoveElement(const KxDataViewItem& item)
	{
		if (KPPFFileEntry* entry = GetDataEntry(GetRow(item)))
		{
			TrackRemoveID(entry->GetID());
			RemoveItemAndNotify(*GetDataVector(), item);
		}
	}
	void MainListModel::OnClearList()
	{
		for (size_t i = 0; i < GetItemCount(); i++)
		{
			TrackRemoveID(GetDataEntry(i)->GetID());
		}
		ClearItemsAndNotify(*GetDataVector());
	}
	
	void MainListModel::SetProject(KPackageProject& projectData)
	{
		m_FileData = &projectData.GetFileData();
	}
}
