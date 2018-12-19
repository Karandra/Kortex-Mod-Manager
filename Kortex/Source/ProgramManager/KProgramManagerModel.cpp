#include "stdafx.h"
#include "KProgramManagerModel.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/ScreenshotsGallery.hpp>
#include "KAux.h"
#include "UI/KMainWindow.h"
#include "KOperationWithProgress.h"
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>

namespace
{
	enum ColumnID
	{
		RequiresVFS,
		ShowInMainMenu,
		Name,
		Arguments,
		Executable,
		WorkingDirectory,
	};
	enum MenuID
	{
		RunProgram,

		AddProgram,
		RemoveProgram,

		ClearPrograms,
		LoadDefaultPrograms,

		Edit,
		ChooseIcon,
		ChooseExecutable,
		ChooseWorkingDirectory,

		ShowExpandedValues,
	};
}

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
	EnableDragAndDrop();
	m_ShowExpandedValues = SaveLoadExpandedValues(false);

	m_BitmapSize.FromSystemIcon();
	GetView()->SetUniformRowHeight(m_BitmapSize.GetHeight() + 4);

	GetView()->AppendColumn<KxDataViewToggleRenderer>(KTr("ProgramManager.List.RequiresVFS"), ColumnID::RequiresVFS, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE);
	GetView()->AppendColumn<KxDataViewToggleRenderer>(KTr("ProgramManager.List.ShowInMainMenu"), ColumnID::ShowInMainMenu, KxDATAVIEW_CELL_ACTIVATABLE, KxCOL_WIDTH_AUTOSIZE);
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer, KxDataViewTextEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 300);
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("ProgramManager.List.Arguments"), ColumnID::Arguments, KxDATAVIEW_CELL_EDITABLE, 300);
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("ProgramManager.List.Executable"), ColumnID::Executable, KxDATAVIEW_CELL_EDITABLE, 100);
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("ProgramManager.List.WorkingDirectory"), ColumnID::WorkingDirectory, KxDATAVIEW_CELL_EDITABLE);
}

void KProgramManagerModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KProgramEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = entry->RawGetName();
				return;
			}
			case ColumnID::Arguments:
			{
				value = entry->RawGetArguments();
				return;
			}
			case ColumnID::Executable:
			{
				value = entry->RawGetExecutable();
				return;
			}
			case ColumnID::WorkingDirectory:
			{
				value = entry->RawGetWorkingDirectory();
				return;
			}
		};
	}
}
void KProgramManagerModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KProgramEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::RequiresVFS:
			{
				value = entry->IsRequiresVFS();
				break;
			}
			case ColumnID::ShowInMainMenu:
			{
				value = entry->ShouldShowInMainMenu();
				break;
			}
			case ColumnID::Name:
			{
				KxDataViewBitmapTextValue data(entry->GetName(), entry->GetLargeBitmap().GetBitmap());
				data.SetVCenterText(true);
				value = data;
				break;
			}
			case ColumnID::Arguments:
			{
				value = m_ShowExpandedValues ? entry->GetArguments() : entry->RawGetArguments();
				break;
			}
			case ColumnID::Executable:
			{
				value = m_ShowExpandedValues ? entry->GetExecutable() : entry->RawGetExecutable();
				break;
			}
			case ColumnID::WorkingDirectory:
			{
				value = m_ShowExpandedValues ? entry->GetWorkingDirectory() : entry->RawGetWorkingDirectory();
				break;
			}
		};
	}
}
bool KProgramManagerModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	KProgramEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::ShowInMainMenu:
			{
				entry->SetShowInMainMenu(value.As<bool>());
				return true;
			}
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
			case ColumnID::Executable:
			{
				entry->SetExecutable(value.As<wxString>());
				Kortex::KProgramManager::GetInstance()->LoadEntryImages(*entry);
				return true;
			}
			case ColumnID::WorkingDirectory:
			{
				entry->SetWorkingDirectory(value.As<wxString>());
				return true;
			}
		};
	}
	return false;
}
bool KProgramManagerModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	return true;
}
bool KProgramManagerModel::GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
{
	const KProgramEntry* entry = GetDataEntry(row);
	if (entry)
	{
		if (column->GetID() == ColumnID::Name && !entry->CanRunNow())
		{
			attributes.SetEnabled(false);
			return true;
		}
	}
	return false;
}

void KProgramManagerModel::OnSelectItem(KxDataViewEvent& event)
{
	const KProgramEntry* entry = GetDataEntry(GetRow(event.GetItem()));
	if (entry)
	{
		KMainWindow::GetInstance()->SetStatus(entry->GetName() + wxS(": ") + entry->GetExecutable());
		return;
	}
	KMainWindow::GetInstance()->ClearStatus();
}
void KProgramManagerModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	KProgramEntry* entry = GetDataEntry(GetRow(item));
	if (entry && column)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				if (entry->CanRunNow())
				{
					Kortex::KProgramManager::GetInstance()->RunEntry(*entry);
				}
				else
				{
					wxBell();
				}
				break;
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
	KxDataViewColumn* column = event.GetColumn();
	if (column)
	{
		auto SaveLoadExpandedValues = [](bool save, bool value = false) -> bool
		{
			if (save)
			{
				Kortex::KProgramManager::GetInstance()->GetActiveInstanceOption().SetAttribute("ShowExpandedValues", value);
				return value;
			}
			else
			{
				return Kortex::KProgramManager::GetInstance()->GetActiveInstanceOption().GetAttributeBool("ShowExpandedValues", value);
			}
		};
		KProgramEntry* entry = GetDataEntry(GetRow(event.GetItem()));

		KxMenu menu;
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::RunProgram, KTr("ProgramManager.Menu.RunProgram")));
			item->Enable(entry && entry->CanRunNow());
			item->SetBitmap(KGetBitmap(KIMG_APPLICATION_RUN));
		}
		menu.AddSeparator();
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddProgram, KTr("ProgramManager.Menu.AddProgram")));
			item->SetBitmap(KGetBitmap(KIMG_PLUS_SMALL));
		}
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::RemoveProgram, KTr("ProgramManager.Menu.RemoveProgram")));
			item->SetBitmap(KGetBitmap(KIMG_MINUS_SMALL));
			item->Enable(entry);
		}
		menu.AddSeparator();
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::ClearPrograms, KTr("ProgramManager.Menu.ClearPrograms")));
			item->Enable(!IsEmpty());
		}
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::LoadDefaultPrograms, KTr("ProgramManager.Menu.LoadDefaultPrograms")));
			item->SetBitmap(KGetBitmap(KIMG_APPLICATION_RUN));
		}
		menu.AddSeparator();
		if (entry && column->IsEditable())
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::Edit, KTrf("ProgramManager.Menu.EditProgram", column->GetTitle())));
			item->SetBitmap(KGetBitmap(KIMG_PENCIL_SMALL));
		}
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::ChooseIcon, KTr("ProgramManager.Menu.ChooseIcon")));
			item->SetBitmap(KGetBitmap(KIMG_IMAGE));
			item->Enable(entry);
		}
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::ChooseExecutable, KTr("ProgramManager.Menu.ChooseExecutable")));
			item->SetBitmap(KGetBitmap(KIMG_DOCUMENT_IMPORT));
			item->Enable(entry);
		}
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::ChooseWorkingDirectory, KTr("ProgramManager.Menu.ChooseWorkingDirectory")));
			item->SetBitmap(KGetBitmap(KIMG_FOLDER));
			item->Enable(entry);
		}
		menu.AddSeparator();
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::ShowExpandedValues, KTr("ProgramManager.Menu.ShowExpandedValues"), wxEmptyString, wxITEM_CHECK));
			item->SetBitmap(KGetBitmap(KIMG_EDIT_CODE));
			item->Check(m_ShowExpandedValues);
		}

		switch (menu.Show(GetView()))
		{
			case MenuID::RunProgram:
			{
				Kortex::KProgramManager::GetInstance()->RunEntry(*entry);
				break;
			}

			case MenuID::AddProgram:
			{
				if (AddProgram())
				{
					size_t sel = GetRow(event.GetItem());
					RefreshItems();
					SelectItem(sel);
				}
				break;
			}
			case MenuID::RemoveProgram:
			{
				size_t sel = GetRow(event.GetItem());
				RemoveProgram(entry);
				RefreshItems();
				SelectItem(sel != 0 ? sel - 1 : 0);
				break;
			}
			case MenuID::ClearPrograms:
			{
				KxTaskDialog dialog(GetViewTLW(), KxID_NONE, KTr("ProgramManager.Menu.ClearPrograms.Message"), KTr("ProgramManager.Menu.ClearPrograms"), KxBTN_YES|KxBTN_NO, KxICON_WARNING);
				if (dialog.ShowModal() == KxID_YES)
				{
					GetDataVector()->clear();
					RefreshItems();
				}
				break;
			}

			case MenuID::Edit:
			{
				GetView()->EditItem(event.GetItem(), column);
				break;
			}
			case MenuID::ChooseIcon:
			{
				wxString path = AskSelectIcon(*entry);
				if (!path.IsEmpty() && path != entry->GetExecutable())
				{
					entry->SetIconPath(path);
					Kortex::KProgramManager::GetInstance()->LoadEntryImages(*entry);
					ItemChanged(event.GetItem());
				}
				break;
			}
			case MenuID::ChooseExecutable:
			{
				wxString path = AskSelectExecutable(entry);
				if (!path.IsEmpty() && path != entry->GetExecutable())
				{
					entry->SetExecutable(path);
					Kortex::KProgramManager::GetInstance()->LoadEntryImages(*entry);
					ItemChanged(event.GetItem());
				}
				break;
			}
			case MenuID::ChooseWorkingDirectory:
			{
				KxFileBrowseDialog dialog(GetViewTLW(), KxID_NONE, KxFBD_OPEN_FOLDER);
				dialog.SetFolder(entry->GetWorkingDirectory());
				if (dialog.ShowModal())
				{
					entry->SetWorkingDirectory(dialog.GetResult());
				}
				else
				{
					entry->SetWorkingDirectory(wxEmptyString);
				}
				ItemChanged(event.GetItem());
				break;
			}
			case MenuID::LoadDefaultPrograms:
			{
				Kortex::KProgramManager::GetInstance()->LoadDefaultPrograms();
				RefreshItems();
				SelectItem(0);
				break;
			}

			case MenuID::ShowExpandedValues:
			{
				m_ShowExpandedValues = !m_ShowExpandedValues;
				SaveLoadExpandedValues(true, m_ShowExpandedValues);
				GetView()->Refresh();
				break;
			}
		};
	}
}

bool KProgramManagerModel::OnDragItems(KxDataViewEventDND& event)
{
	if (CanDragDropNow())
	{
		KxDataViewItem item = GetView()->GetSelection();
		if (item.IsOK())
		{
			SetDragDropDataObject(new KProgramManagerModelDND(item));
			event.SetDragFlags(wxDrag_AllowMove);
			event.SetDropEffect(wxDragMove);
			return true;
		}
	}
	event.SetDropEffect(wxDragError);
	return false;
}
bool KProgramManagerModel::OnDropItems(KxDataViewEventDND& event)
{
	if (HasDragDropDataObject())
	{
		KProgramEntry::Vector& items = GetProgramsList();
		size_t thisRow = GetRow(event.GetItem());
		size_t droppedRow = GetRow(GetDragDropDataObject()->GetItem());

		if (thisRow != droppedRow && thisRow < items.size() && droppedRow < items.size())
		{
			KProgramEntry movedItem = std::move(items[droppedRow]);
			items.erase(items.begin() + droppedRow);
			items.insert(items.begin() + thisRow, movedItem);

			SelectItem(thisRow);
			GetView()->Refresh();
			return true;
		}
	}
	return false;
}
bool KProgramManagerModel::CanDragDropNow() const
{
	return true;
}

wxString KProgramManagerModel::AskSelectExecutable(const KProgramEntry* entry) const
{
	KxFileBrowseDialog dialog(GetViewTLW(), KxID_NONE, KxFBD_OPEN);
	dialog.SetFolder(entry->GetExecutable());
	dialog.AddFilter("*.exe", KTr("FileFilter.Programs"));
	dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
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
wxString KProgramManagerModel::AskSelectIcon(const KProgramEntry& entry) const
{
	KxFileBrowseDialog dialog(GetViewTLW(), KxID_NONE, KxFBD_OPEN);
	dialog.SetFolder(entry.HasIconPath() ? entry.GetIconPath() : wxEmptyString);
	dialog.AddFilter(KxString::Join(Kortex::IScreenshotsGallery::GetSupportedExtensions(), ";"), KTr("FileFilter.Images"));
	dialog.AddFilter("*", KTr("FileFilter.AllFiles"));

	if (dialog.ShowModal())
	{
		return dialog.GetResult();
	}
	return wxEmptyString;
}
bool KProgramManagerModel::AddProgram()
{
	wxString path = AskSelectExecutable();
	if (!path.IsEmpty())
	{
		KProgramEntry& entry = GetDataVector()->emplace_back();
		entry.SetName(path.AfterLast('\\').BeforeLast('.'));
		entry.SetExecutable(path);
		Kortex::KProgramManager::GetInstance()->LoadEntryImages(entry);
		return true;
	}
	return false;
}
void KProgramManagerModel::RemoveProgram(KProgramEntry* entry)
{
	auto it = std::remove_if(GetDataVector()->begin(), GetDataVector()->end(), [entry](const KProgramEntry& v)
	{
		return &v == entry;
	});
	if (it != GetDataVector()->end())
	{
		GetDataVector()->erase(it, GetDataVector()->end());
	}
}

bool KProgramManagerModel::SaveLoadExpandedValues(bool save, bool value) const
{
	if (save)
	{
		Kortex::KProgramManager::GetInstance()->GetActiveInstanceOption().SetAttribute("ShowExpandedValues", value);
		return value;
	}
	else
	{
		return Kortex::KProgramManager::GetInstance()->GetActiveInstanceOption().GetAttributeBool("ShowExpandedValues", value);
	}
}

KProgramManagerModel::KProgramManagerModel()
{
	SetDataViewFlags(KxDV_VERT_RULES);
	SetDataVector(&GetProgramsList());
}

void KProgramManagerModel::RefreshItems()
{
	for (KProgramEntry& entry: GetProgramsList())
	{
		if (!Kortex::KProgramManager::GetInstance()->CheckEntryImages(entry))
		{
			Kortex::KProgramManager::GetInstance()->LoadEntryImages(entry);
		}
	}
	KxDataViewVectorListModelEx::RefreshItems();
}
