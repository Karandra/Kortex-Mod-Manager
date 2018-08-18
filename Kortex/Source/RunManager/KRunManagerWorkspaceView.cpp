#include "stdafx.h"
#include "KRunManagerWorkspaceView.h"
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

void KRunManagerWorkspaceView::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KRunManagerWorkspaceView::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KRunManagerWorkspaceView::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KRunManagerWorkspaceView::OnContextMenu, this);
	GetView()->Bind(KxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, [this](KxDataViewEvent& event)
	{
		KxMenu menu;
		if (GetView()->CreateColumnSelectionMenu(menu))
		{
			GetView()->OnColumnSelectionMenu(menu);
		}
	});

	GetView()->AppendColumn<KxDataViewToggleRenderer>(T("RunManager.List.RequiresVFS"), ColumnID::RequiresVFS, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE);
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer, KxDataViewTextEditor>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 300);
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("RunManager.List.Arguments"), ColumnID::Arguments, KxDATAVIEW_CELL_EDITABLE, 300);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("RunManager.List.Executable"), ColumnID::Executable, KxDATAVIEW_CELL_INERT, 100);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("RunManager.List.WorkingDirectory"), ColumnID::WorkingDirectory, KxDATAVIEW_CELL_INERT);
}

void KRunManagerWorkspaceView::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KRunManagerProgram* entry = GetDataEntry(row);
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
void KRunManagerWorkspaceView::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KRunManagerProgram* entry = GetDataEntry(row);
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
bool KRunManagerWorkspaceView::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	KRunManagerProgram* entry = GetDataEntry(row);
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
bool KRunManagerWorkspaceView::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	const KRunManagerProgram* entry = GetDataEntry(row);
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

void KRunManagerWorkspaceView::OnSelectItem(KxDataViewEvent& event)
{
	const KRunManagerProgram* entry = GetDataEntry(GetRow(event.GetItem()));
	if (entry)
	{
		KApp::Get().GetMainWindow()->SetStatus(entry->GetName() + ": " + entry->GetExecutable());
		return;
	}
	KApp::Get().GetMainWindow()->ClearStatus();
}
void KRunManagerWorkspaceView::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	KRunManagerProgram* entry = GetDataEntry(GetRow(item));
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
					KRunManager::Get().OnQueryItemImage(*entry);
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
					KRunManager::Get().RunEntry(*entry);
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
void KRunManagerWorkspaceView::OnContextMenu(KxDataViewEvent& event)
{
	KRunManagerProgram* entry = GetDataEntry(GetRow(event.GetItem()));

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

wxString KRunManagerWorkspaceView::AskSelectExecutablePath(const KRunManagerProgram* entry) const
{
	KxFileBrowseDialog dialog(GetViewTLW(), KxID_NONE, KxFBD_OPEN);
	dialog.SetFolder(V(KVAR(KVAR_VIRTUAL_GAME_ROOT)));
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
bool KRunManagerWorkspaceView::AddProgram()
{
	wxString path = AskSelectExecutablePath();
	if (!path.IsEmpty())
	{
		KRunManagerProgram& entry = GetDataVector()->emplace_back();
		entry.SetName(path.AfterLast('\\').BeforeLast('.'));
		entry.SetExecutable(path);
		entry.SetBitmap(KRunManager::Get().OnQueryItemImage(entry));
		
		// Let 'KRunManagerProgram::IsRequiresVFS' decide if this program needs VFS.
		entry.SetRequiresVFS(false);
		return true;
	}
	return false;
}
void KRunManagerWorkspaceView::RemoveProgram(KRunManagerProgram* entry)
{
	auto it = std::remove_if(GetDataVector()->begin(), GetDataVector()->end(), [entry](const KRunManagerProgram& v)
	{
		return &v == entry;
	});
	if (it != GetDataVector()->end())
	{
		GetDataVector()->erase(it, GetDataVector()->end());
	}
}

void KRunManagerWorkspaceView::RefreshItems()
{
	KRunManager::Get().UpdateProgramListImages();
	KDataViewVectorListModel::RefreshItems();
}
