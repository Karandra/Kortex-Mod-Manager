#include "stdafx.h"
#include "MainListModel.h"
#include "ContentModel.h"
#include "PackageCreator/PageBase.h"
#include "PackageCreator/WorkspaceDocument.h"
#include <Kortex/Application.hpp>
#include "Utility/OperationWithProgress.h"
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
			m_PriorityRenderer->SetRangeInt(PackageProject::FileDataSection::ms_MinUserPriority, PackageProject::FileDataSection::ms_MaxUserPriority);
	
			wxIntegerValidator<int32_t> tValidator;
			tValidator.SetMin(m_PriorityRenderer->GetMinInt());
			tValidator.SetMax(m_PriorityRenderer->GetMaxInt());
			m_PriorityRenderer->SetValidator(tValidator);
		}
	}
	
	void MainListModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		const PackageProject::FileItem* entry = GetDataEntry(row);
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
		if (const PackageProject::FileItem* item = GetDataEntry(row))
		{
			switch (column->GetID())
			{
				case ColumnID::ID:
				{
					value = KxDataViewBitmapTextValue(item->GetID(), ImageProvider::GetBitmap(item->QueryInterface<PackageProject::FolderItem>() ? ImageResourceID::Folder : ImageResourceID::Document));
					break;
				}
				case ColumnID::Source:
				{
					value = item->GetSource();
					break;
				}
				case ColumnID::Destination:
				{
					value = item->GetDestination();
					break;
				}
				case ColumnID::Priority:
				{
					value = item->GetPriority();
					break;
				}
			};
		}
	}
	bool MainListModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
	{
		PackageProject::FileItem* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::ID:
				{
					wxString newID = value.As<wxString>();
					if (newID != entry->GetID())
					{
						if (!PackageProject::FileDataSection::IsFileIDValid(newID))
						{
							PageBase::ShowTooltipWarning(GetView(), KTr("Generic.IDInvalid"), GetItemRect(GetItem(row), column));
						}
						else if (m_FileData->HasItemWithID(newID, entry))
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
	
	void MainListModel::AddEverythingFromPath(const wxString& filePath, PackageProject::FolderItem& folderItem, Utility::OperationWithProgressBase& context)
	{
		KxEvtFile source(filePath);
		context.LinkHandler(&source, KxEVT_FILEOP_SEARCH);

		for (const wxString& path: source.Find(KxFile::NullFilter, KxFS_FILE, true))
		{
			wxString target = path;
			target.Remove(0, filePath.Length());
			if (!target.IsEmpty() && target[0] == wxS('\\'))
			{
				target.Remove(0, 1);
			}
	
			PackageProject::FileItem& entry = folderItem.AddFile();
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
				for (auto& entry: group->GetItems())
				{
					TrackID_ReplaceOrRemove(trackedID, newID, entry->GetFileData(), remove);
				}
			}
		}
	
		// Conditional install
		for (auto& step: GetProject().GetComponents().GetConditionalSteps())
		{
			TrackID_ReplaceOrRemove(trackedID, newID, step->GetItems(), remove);
		}
	
		return true;
	}
	
	void MainListModel::OnActivateItem(KxDataViewEvent& event)
	{
		if (KxDataViewColumn* column = event.GetColumn())
		{
			switch (column->GetID())
			{
				case ColumnID::Source:
				{
					if (PackageProject::FileItem* item = GetDataEntry(GetRow(event.GetItem())))
					{
						KxFileBrowseDialog dialog(GetView(), KxID_NONE, KxFBD_OPEN);
						if (item->QueryInterface<PackageProject::FolderItem>())
						{
							dialog.SetFolder(item->GetSource());
							dialog.SetOptionEnabled(KxFBD_PICK_FOLDERS);
						}
						else
						{
							dialog.SetFolder(item->GetSource().BeforeLast('\\'));
							dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
						}
	
						if (dialog.ShowModal() == KxID_OK)
						{
							item->SetSource(dialog.GetResult());
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
			PackageProject::FileItem* item = GetDataEntry(GetRow(event.GetItem()));
			if (PackageProject::FolderItem* folderItem; item && item->QueryInterface(folderItem))
			{
				wxWindowUpdateLocker lock(m_ContentModel->GetView());
				m_ContentModel->SetDataVector(folderItem);
				m_ContentModel->SelectItem();

				return;
			}
			m_ContentModel->SetDataVector();
		}
	}
	void MainListModel::OnContextMenu(KxDataViewEvent& event)
	{
		PackageProject::FileItem* fileItem = GetDataEntry(GetRow(event.GetItem()));
	
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
			item->Enable(fileItem && fileItem->QueryInterface<PackageProject::FolderItem>());
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderArrow));
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
				OnReplaceFolderContent(event.GetItem(), *fileItem->QueryInterface<PackageProject::FolderItem>());
				break;
			}
			case KxID_REMOVE:
			{
				OnRemoveElement(event.GetItem());
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
					for (auto& item: *GetDataVector())
					{
						item->SetDestination(dialog.GetValue());
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
			bool freeID = m_FileData->IsUnusedID(target);
	
			PackageProject::FileItem& entry = m_FileData->AddFile(std::make_unique<PackageProject::FileItem>());
			entry.SetSource(source);
			entry.SetDestination(target);
			
			// Select ID
			entry.SetID(target);
			if (!freeID)
			{
				entry.MakeUniqueID();
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
			bool freeID = m_FileData->IsUnusedID(id);
	
			PackageProject::FolderItem& folderItem = m_FileData->AddFolder(std::make_unique<PackageProject::FolderItem>());
			folderItem.SetID(id);
			folderItem.SetSource(source);
	
			// Select ID
			if (!freeID)
			{
				folderItem.MakeUniqueID();
			}
	
			auto operation = new Utility::OperationWithProgressDialog<KxFileOperationEvent>(true, GetView());
			operation->OnRun([this, operation, &folderItem, source]()
			{
				AddEverythingFromPath(source, folderItem, *operation);
			});
			operation->OnEnd([this]()
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
			auto operation = new Utility::OperationWithProgressDialog<KxFileOperationEvent>(true, GetView());
			operation->OnRun([this, operation, source]()
			{
				KxStringVector folders = KxFile(source).Find(KxFile::NullFilter, KxFS_FOLDER, false);
				for (const wxString& folderPath: folders)
				{
					wxString id = folderPath.AfterLast('\\');
					bool bCanUseID = m_FileData->IsUnusedID(id);
	
					PackageProject::FolderItem& folderItem = m_FileData->AddFolder(std::make_unique<PackageProject::FolderItem>());
					folderItem.SetID(id);
					folderItem.SetSource(folderPath);
	
					if (!bCanUseID)
					{
						folderItem.MakeUniqueID();
					}
	
					AddEverythingFromPath(folderPath, folderItem, *operation);
				}
			});
			operation->OnEnd([this]()
			{
				RefreshItems();
				ChangeNotify();
			});
			operation->SetDialogCaption(KTr("Generic.FileSearchInProgress"));
			operation->Run();
		}
	}
	void MainListModel::OnReplaceFolderContent(const KxDataViewItem& item, PackageProject::FolderItem& folderEntry)
	{
		KxFileBrowseDialog dialog(GetViewTLW(), KxID_NONE, KxFBD_OPEN_FOLDER);
		if (dialog.ShowModal() == KxID_OK)
		{
			wxString source = dialog.GetResult();
	
			auto operation = new Utility::OperationWithProgressDialog<KxFileOperationEvent>(true, GetView());
			operation->OnRun([this, operation, &folderEntry, source]()
			{
				folderEntry.SetSource(source);
				folderEntry.GetFiles().clear();
				AddEverythingFromPath(source, folderEntry, *operation);
			});
			operation->OnEnd([this, item]()
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
		if (PackageProject::FileItem* entry = GetDataEntry(GetRow(item)))
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
	
	void MainListModel::SetProject(ModPackageProject& projectData)
	{
		m_FileData = &projectData.GetFileData();
	}
}
