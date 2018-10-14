#include "stdafx.h"
#include "KProgramManagerModel.h"
#include "ModManager/KModManager.h"
#include <KxFramework/KxMenu.h>
#include "KVariablesDatabase.h"
#include "KApp.h"
#include "KAux.h"
#include "UI/KMainWindow.h"
#include "KOperationWithProgress.h"
#include <KxFramework/KxFileBrowseDialog.h>

enum ColumnID
{
	RequiresVFS,
	Name,
	Arguments,
	Executable,
	WorkingDirectory,
};

void KProgramManagerModel::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KProgramManagerModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KProgramManagerModel::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KProgramManagerModel::OnContextMenu, this);
	GetView()->Bind(KxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, [this](KxDataViewEvent& event)
	{
		KxMenu menu;
		if (GetView()->CreateColumnSelectionMenu(menu))
		{
			GetView()->OnColumnSelectionMenu(menu);
		}
	});

	GetView()->AppendColumn<KxDataViewToggleRenderer>(T("ProgramManager.List.RequiresVFS"), ColumnID::RequiresVFS, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE);
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer, KxDataViewTextEditor>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 300);
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("ProgramManager.List.Arguments"), ColumnID::Arguments, KxDATAVIEW_CELL_EDITABLE, 300);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("ProgramManager.List.Executable"), ColumnID::Executable, KxDATAVIEW_CELL_INERT, 100);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("ProgramManager.List.WorkingDirectory"), ColumnID::WorkingDirectory, KxDATAVIEW_CELL_INERT);
}

void KProgramManagerModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KProgramManagerEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = entry->GetName();
				return;
			}
		};
	}
	GetValueByRow(value, row, column);
}
void KProgramManagerModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KProgramManagerEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::RequiresVFS:
			{
				value = entry->CalcRequiresVFS();
				break;
			}
			case ColumnID::Name:
			{
				value = KxDataViewBitmapTextValue(entry->GetName(), entry->GetBitmap());
				break;
			}
			case ColumnID::Arguments:
			{
				value = entry->GetArguments();
				break;
			}
			case ColumnID::Executable:
			{
				value = entry->GetExecutable();
				break;
			}
			case ColumnID::WorkingDirectory:
			{
				const wxString& workingDirectory = entry->GetWorkingDirectory();
				value = workingDirectory.IsEmpty() ? entry->GetExecutable().BeforeLast('\\') : workingDirectory;
				break;
			}
		};
	}
}
bool KProgramManagerModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	KProgramManagerEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				entry->SetName(value.As<wxString>());
				return true;
			}
			case ColumnID::Arguments:
			{
				entry->SetArguments(value.As<wxString>());
				return true;
			}
		};
	}
	return false;
}
bool KProgramManagerModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	const KProgramManagerEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			case ColumnID::Arguments:
			case ColumnID::Executable:
			{
				return true;
			}
		};
	}
	return false;
}

void KProgramManagerModel::OnSelectItem(KxDataViewEvent& event)
{
	const KProgramManagerEntry* entry = GetDataEntry(GetRow(event.GetItem()));
	if (entry)
	{
		KMainWindow::GetInstance()->SetStatus(entry->GetName() + ": " + entry->GetExecutable());
		return;
	}
	KMainWindow::GetInstance()->ClearStatus();
}
void KProgramManagerModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	KProgramManagerEntry* entry = GetDataEntry(GetRow(item));
	if (entry && column)
	{
		switch (column->GetID())
		{
			case ColumnID::Executable:
			{
				wxString path = AskSelectExecutablePath(entry);
				if (!path.IsEmpty() && path != entry->GetExecutable())
				{
					entry->SetExecutable(path);
					KProgramManager::GetInstance()->OnQueryItemImage(*entry);
					ItemChanged(item);
				}
				break;
			}
			case ColumnID::Name:
			{
				if (entry->CalcRequiresVFS() && !KModManager::Get().IsVFSMounted())
				{
					wxBell();
				}
				else
				{
					KProgramManager::GetInstance()->RunEntry(*entry);
				}
				break;
			}
			case ColumnID::WorkingDirectory:
			{
				KxFileBrowseDialog dialog(GetViewTLW(), KxID_NONE, KxFBD_OPEN_FOLDER);

				wxAny folder;
				GetValueByRow(folder, GetRow(item), column);
				dialog.SetFolder(folder.As<wxString>());

				if (dialog.ShowModal())
				{
					entry->SetWorkingDirectory(dialog.GetResult());
				}
				else
				{
					entry->SetWorkingDirectory(wxEmptyString);
				}
				ItemChanged(item);
			}
			default:
			{
				GetView()->EditItem(item, column);
				break;
			}
		};
	}
}
void KProgramManagerModel::OnContextMenu(KxDataViewEvent& event)
{
	KProgramManagerEntry* entry = GetDataEntry(GetRow(event.GetItem()));

	KxMenu menu;
	menu.Add(new KxMenuItem(KxID_ADD, T(KxID_ADD)));
	menu.Add(new KxMenuItem(KxID_REMOVE, T(KxID_REMOVE)))->Enable(entry);
	menu.AddSeparator();
	menu.Add(new KxMenuItem(KxID_CLEAR, T(KxID_CLEAR)));

	switch (menu.Show(GetView()))
	{
		case KxID_ADD:
		{
			if (AddProgram())
			{
				size_t sel = GetRow(event.GetItem());
				RefreshItems();
				SelectItem(sel);
			}
			break;
		}
		case KxID_REMOVE:
		{
			size_t sel = GetRow(event.GetItem());
			RemoveProgram(entry);
			RefreshItems();
			SelectItem(sel != 0 ? sel - 1 : 0);
			break;
		}
		case KxID_CLEAR:
		{
			GetDataVector()->clear();
			RefreshItems();
			break;
		}
	};
}

wxString KProgramManagerModel::AskSelectExecutablePath(const KProgramManagerEntry* entry) const
{
	KxFileBrowseDialog dialog(GetViewTLW(), KxID_NONE, KxFBD_OPEN);
	dialog.SetFolder(V(KVAR(KVAR_VIRTUAL_GAME_DIR)));
	dialog.AddFilter("*.exe", T("FileFilter.Programs"));
	dialog.AddFilter("*", T("FileFilter.AllFiles"));
	if (entry)
	{
		dialog.SetFolder(entry->GetExecutable().BeforeLast('\\'));
	}

	if (dialog.ShowModal())
	{
		return dialog.GetResult();
	}
	return wxEmptyString;
}
bool KProgramManagerModel::AddProgram()
{
	wxString path = AskSelectExecutablePath();
	if (!path.IsEmpty())
	{
		KProgramManagerEntry& entry = GetDataVector()->emplace_back();
		entry.SetName(path.AfterLast('\\').BeforeLast('.'));
		entry.SetExecutable(path);
		entry.SetBitmap(KProgramManager::GetInstance()->OnQueryItemImage(entry));
		
		// Let 'KProgramManagerEntry::IsRequiresVFS' decide if this program needs VFS.
		entry.SetRequiresVFS(false);
		return true;
	}
	return false;
}
void KProgramManagerModel::RemoveProgram(KProgramManagerEntry* entry)
{
	auto it = std::remove_if(GetDataVector()->begin(), GetDataVector()->end(), [entry](const KProgramManagerEntry& v)
	{
		return &v == entry;
	});
	if (it != GetDataVector()->end())
	{
		GetDataVector()->erase(it, GetDataVector()->end());
	}
}

void KProgramManagerModel::RefreshItems()
{
	KProgramManager::GetInstance()->UpdateProgramListImages();
	KDataViewVectorListModel::RefreshItems();
}
