#include "stdafx.h"
#include "DisplayModel.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/ScreenshotsGallery.hpp>
#include "UI/KMainWindow.h"
#include "Utility/KAux.h"
#include "Utility/KOperationWithProgress.h"
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

namespace Kortex::Application::OName
{
	KortexDefOption(ShowExpandedValues);
}

namespace Kortex::ProgramManager
{
	void DisplayModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &DisplayModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &DisplayModel::OnSelectItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &DisplayModel::OnContextMenu, this);
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

	void DisplayModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		const IProgramEntry* entry = GetDataEntry(row);
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
	void DisplayModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		const IProgramEntry* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::RequiresVFS:
				{
					value = entry->RequiresVFS();
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
	bool DisplayModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
	{
		IProgramEntry* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::ShowInMainMenu:
				{
					entry->ShowInMainMenu(value.As<bool>());
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
					IProgramManager::GetInstance()->LoadProgramIcons(*entry);
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
	bool DisplayModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
	{
		return true;
	}
	bool DisplayModel::GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
	{
		const IProgramEntry* entry = GetDataEntry(row);
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

	void DisplayModel::OnSelectItem(KxDataViewEvent& event)
	{
		const IProgramEntry* entry = GetDataEntry(GetRow(event.GetItem()));
		if (entry)
		{
			KMainWindow::GetInstance()->SetStatus(entry->GetName() + wxS(": ") + entry->GetExecutable());
			return;
		}
		KMainWindow::GetInstance()->ClearStatus();
	}
	void DisplayModel::OnActivateItem(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		KxDataViewColumn* column = event.GetColumn();
		IProgramEntry* entry = GetDataEntry(GetRow(item));
		if (entry && column)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					if (entry->CanRunNow())
					{
						IProgramManager::GetInstance()->RunEntry(*entry);
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
	void DisplayModel::OnContextMenu(KxDataViewEvent& event)
	{
		KxDataViewColumn* column = event.GetColumn();
		if (column)
		{
			IProgramEntry* entry = GetDataEntry(GetRow(event.GetItem()));

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
					IProgramManager::GetInstance()->RunEntry(*entry);
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
						IProgramManager::GetInstance()->LoadProgramIcons(*entry);
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
						IProgramManager::GetInstance()->LoadProgramIcons(*entry);
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
					IProgramManager::GetInstance()->LoadDefaultPrograms();
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

	bool DisplayModel::OnDragItems(KxDataViewEventDND& event)
	{
		if (CanDragDropNow())
		{
			KxDataViewItem item = GetView()->GetSelection();
			if (item.IsOK())
			{
				SetDragDropDataObject(new DisplayModelDND(item));
				event.SetDragFlags(wxDrag_AllowMove);
				event.SetDropEffect(wxDragMove);
				return true;
			}
		}
		event.SetDropEffect(wxDragError);
		return false;
	}
	bool DisplayModel::OnDropItems(KxDataViewEventDND& event)
	{
		if (HasDragDropDataObject())
		{
			IProgramEntry::Vector& items = *GetDataVector();
			size_t thisRow = GetRow(event.GetItem());
			size_t droppedRow = GetRow(GetDragDropDataObject()->GetItem());

			if (thisRow != droppedRow && thisRow < items.size() && droppedRow < items.size())
			{
				auto movedItem = std::move(items[droppedRow]);
				items.erase(items.begin() + droppedRow);
				items.insert(items.begin() + thisRow, std::move(movedItem));

				SelectItem(thisRow);
				GetView()->Refresh();
				return true;
			}
		}
		return false;
	}
	bool DisplayModel::CanDragDropNow() const
	{
		return true;
	}

	wxString DisplayModel::AskSelectExecutable(const IProgramEntry* entry) const
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
	wxString DisplayModel::AskSelectIcon(const IProgramEntry& entry) const
	{
		KxFileBrowseDialog dialog(GetViewTLW(), KxID_NONE, KxFBD_OPEN);
		dialog.SetFolder(entry.GetIconPath());
		dialog.AddFilter(KxString::Join(Kortex::IScreenshotsGallery::GetSupportedExtensions(), ";"), KTr("FileFilter.Images"));
		dialog.AddFilter("*", KTr("FileFilter.AllFiles"));

		if (dialog.ShowModal())
		{
			return dialog.GetResult();
		}
		return wxEmptyString;
	}
	bool DisplayModel::AddProgram()
	{
		wxString path = AskSelectExecutable();
		if (!path.IsEmpty())
		{
			IProgramEntry& entry = IProgramManager::GetInstance()->EmplaceProgram();
			entry.SetName(path.AfterLast('\\').BeforeLast('.'));
			entry.SetExecutable(path);
			IProgramManager::GetInstance()->LoadProgramIcons(entry);
			return true;
		}
		return false;
	}
	void DisplayModel::RemoveProgram(IProgramEntry* entry)
	{
		IProgramManager::GetInstance()->RemoveProgram(*entry);
	}

	bool DisplayModel::SaveLoadExpandedValues(bool save, bool value) const
	{
		using namespace Kortex;
		using namespace Kortex::Application;

		if (save)
		{
			GetAInstanceOptionOf<IProgramManager>(OName::ShowExpandedValues).SetAttribute(OName::Enabled, value);
			return value;
		}
		else
		{
			return GetAInstanceOptionOf<IProgramManager>(OName::ShowExpandedValues).GetAttributeBool(OName::Enabled, value);
		}
	}

	DisplayModel::DisplayModel()
	{
		SetDataViewFlags(KxDV_VERT_RULES);
		SetDataVector(&IProgramManager::GetInstance()->GetProgramList());
	}

	void DisplayModel::RefreshItems()
	{
		for (auto& entry: *GetDataVector())
		{
			if (!IProgramManager::GetInstance()->CheckProgramIcons(*entry))
			{
				IProgramManager::GetInstance()->LoadProgramIcons(*entry);
			}
		}
		KxDataViewVectorListModelEx::RefreshItems();
	}
}
