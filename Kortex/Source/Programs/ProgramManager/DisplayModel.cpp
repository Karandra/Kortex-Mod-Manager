#include "stdafx.h"
#include "DisplayModel.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/ScreenshotsGallery.hpp>
#include "ProgramEditorDialog.h"
#include "Programs/ProgramEvent.h"
#include "Utility/OperationWithProgress.h"
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

		Edit,
		ChooseIcon,
		ShowExpandedValues,

		AddProgram,
		RemoveProgram,

		ClearPrograms,
		LoadDefaultPrograms,
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
		const IProgramItem* entry = GetDataEntry(row);
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
		const IProgramItem* entry = GetDataEntry(row);
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
		IProgramItem* item = GetDataEntry(row);
		if (item)
		{
			switch (column->GetID())
			{
				case ColumnID::ShowInMainMenu:
				{
					item->ShowInMainMenu(value.As<bool>());
					BroadcastProcessor::Get().QueueEvent(ProgramEvent::EvtRemoved, *item);

					return true;
				}
				case ColumnID::Name:
				{
					item->SetName(value.As<wxString>());
					BroadcastProcessor::Get().QueueEvent(ProgramEvent::EvtRemoved, *item);

					return true;
				}
				case ColumnID::Arguments:
				{
					item->SetArguments(value.As<wxString>());
					BroadcastProcessor::Get().QueueEvent(ProgramEvent::EvtRemoved, *item);

					return true;
				}
				case ColumnID::Executable:
				{
					item->SetExecutable(value.As<wxString>());
					IProgramManager::GetInstance()->LoadProgramIcons(*item);
					BroadcastProcessor::Get().QueueEvent(ProgramEvent::EvtRemoved, *item);

					return true;
				}
				case ColumnID::WorkingDirectory:
				{
					item->SetWorkingDirectory(value.As<wxString>());
					BroadcastProcessor::Get().QueueEvent(ProgramEvent::EvtRemoved, *item);

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
		const IProgramItem* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					if (!entry->CanRunNow())
					{
						attributes.SetEnabled(false);
						return true;
					}
					return false;
				}
				case ColumnID::Arguments:
				{
					attributes.SetFontFace(wxS("Consolas"));
					return true;
				}
			};
		}
		return false;
	}

	void DisplayModel::OnSelectItem(KxDataViewEvent& event)
	{
		const IProgramItem* entry = GetDataEntry(GetRow(event.GetItem()));
		if (entry)
		{
			IMainWindow::GetInstance()->SetStatus(entry->GetName() + wxS(": ") + entry->GetExecutable());
			return;
		}
		IMainWindow::GetInstance()->ClearStatus();
	}
	void DisplayModel::OnActivateItem(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		KxDataViewColumn* column = event.GetColumn();
		IProgramItem* entry = GetDataEntry(GetRow(item));
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
					ProgramEditorDialog dialog(GetViewTLW(), *entry);
					if (dialog.ShowModal() == KxID_OK)
					{
						IProgramManager::GetInstance()->LoadProgramIcons(dialog.Accept());
					}
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
			IProgramItem* entry = GetDataEntry(GetRow(event.GetItem()));

			KxMenu menu;
			{
				KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::RunProgram, KTr("ProgramManager.Menu.RunProgram")));
				item->Enable(entry && entry->CanRunNow());
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::ApplicationRun));
			}

			menu.AddSeparator();
			if (entry && column->IsEditable())
			{
				KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::Edit, KTrf("ProgramManager.Menu.EditProgram", column->GetTitle())));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::PencilSmall));
			}
			{
				KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::ChooseIcon, KTr("ProgramManager.Menu.ChooseIcon")));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Image));
				item->Enable(entry);
			}
			{
				KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::ShowExpandedValues, KTr("ProgramManager.Menu.ShowExpandedValues"), wxEmptyString, wxITEM_CHECK));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::EditCode));
				item->Check(m_ShowExpandedValues);
			}

			menu.AddSeparator();
			{
				KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::AddProgram, KTr("ProgramManager.Menu.AddProgram")));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::PlusSmall));
			}
			{
				KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::RemoveProgram, KTr("ProgramManager.Menu.RemoveProgram")));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::MinusSmall));
				item->Enable(entry);
			}

			menu.AddSeparator();
			{
				KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::ClearPrograms, KTr("ProgramManager.Menu.ClearPrograms")));
				item->Enable(!IsEmpty());
			}
			{
				KxMenuItem* item = menu.Add(new KxMenuItem(MenuID::LoadDefaultPrograms, KTr("ProgramManager.Menu.LoadDefaultPrograms")));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::ApplicationRun));
			}

			switch (menu.Show(GetView()))
			{
				case MenuID::RunProgram:
				{
					IProgramManager::GetInstance()->RunEntry(*entry);
					break;
				}

				case MenuID::Edit:
				{
					ProgramEditorDialog dialog(GetViewTLW(), *entry);
					if (dialog.ShowModal() == KxID_OK)
					{
						IProgramManager::GetInstance()->LoadProgramIcons(dialog.Accept());
					}
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
				case MenuID::ShowExpandedValues:
				{
					m_ShowExpandedValues = !m_ShowExpandedValues;
					SaveLoadExpandedValues(true, m_ShowExpandedValues);
					GetView()->Refresh();
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
					RemoveProgram(*entry);
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

				case MenuID::LoadDefaultPrograms:
				{
					IProgramManager::GetInstance()->LoadDefaultPrograms();
					RefreshItems();
					SelectItem(0);
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
			IProgramItem::Vector& items = *GetDataVector();
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

	bool DisplayModel::AddProgram()
	{
		ProgramEditorDialog dialog(GetViewTLW());
		if (dialog.ShowModal() == KxID_OK)
		{
			IProgramItem& item = dialog.Accept();
			IProgramManager::GetInstance()->LoadProgramIcons(item);

			BroadcastProcessor::Get().QueueEvent(ProgramEvent::EvtAdded, item);
			return true;
		}
		return false;
	}
	void DisplayModel::RemoveProgram(IProgramItem& item)
	{
		BroadcastProcessor::Get().QueueEvent(ProgramEvent::EvtRemoved, item);
		IProgramManager::GetInstance()->RemoveProgram(item);
	}
	wxString DisplayModel::AskSelectIcon(const IProgramItem& entry) const
	{
		KxFileBrowseDialog dialog(GetViewTLW(), KxID_NONE, KxFBD_OPEN);
		dialog.SetFolder(entry.GetIconPath());
		dialog.AddFilter(KxString::Join(IScreenshotsGallery::GetSupportedExtensions(), ";"), KTr("FileFilter.Images"));
		dialog.AddFilter("*", KTr("FileFilter.AllFiles"));

		if (dialog.ShowModal())
		{
			return dialog.GetResult();
		}
		return wxEmptyString;
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
		for (auto& item: *GetDataVector())
		{
			if (!IProgramManager::GetInstance()->CheckProgramIcons(*item))
			{
				IProgramManager::GetInstance()->LoadProgramIcons(*item);
			}
		}

		BroadcastProcessor::Get().QueueEvent(ProgramEvent::EvtRefreshed);
		KxDataViewVectorListModelEx::RefreshItems();
	}
}
