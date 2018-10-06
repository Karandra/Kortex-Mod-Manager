#include "stdafx.h"
#include "KModCollisionViewerModel.h"
#include "UI/KMainWindow.h"
#include "KOperationWithProgress.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxShell.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxString.h>

bool KModCollisionViewerModelNS::ModelEntry::FindCollisions(const KModEntry& modEntry)
{
	if (m_Item.IsNormalItem() && m_Item.IsFile())
	{
		m_RelativePath = m_Item.GetFullPath();
		m_RelativePath.Remove(0, modEntry.GetLocation(KMM_LOCATION_MOD_FILES).Length() + 1); // Plus 1 for slash
		m_Collisions = KModManager::GetDispatcher().FindCollisions(modEntry, m_RelativePath);

		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
enum ColumnID
{
	Name,
	Collisions,
	ModificationDate,
	Type,
	Size,
};

wxString KModCollisionViewerModel::FormatSingleCollision(const KMMDispatcherCollision& collision)
{
	const KModEntry* mod = collision.GetMod();
	const KMMDispatcherCollisionType type = collision.GetType();

	return KxFormat(KMMDispatcherCollision::GetLocalizedCollisionName(type)).arg(mod->GetName());
}
wxString KModCollisionViewerModel::FormatCollisionsCount(const CollisionVector& collisions)
{
	if (collisions.size() == 1)
	{
		return FormatSingleCollision(collisions.front());
	}
	return KxFormat("%1").arg(collisions.size());
}
wxString KModCollisionViewerModel::FormatCollisionsView(const CollisionVector& collisions)
{
	KxStringVector names;
	for (const auto& collision: collisions)
	{
		names.push_back(FormatSingleCollision(collision));
	}
	return KxString::Join(names, "\r\n");
}

void KModCollisionViewerModel::OnInitControl()
{
	/* View */
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KModCollisionViewerModel::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KModCollisionViewerModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KModCollisionViewerModel::OnContextMenu, this);

	/* Columns */
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_INERT, 300);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("ModExplorer.Collisions"), ColumnID::Collisions, KxDATAVIEW_CELL_INERT, 200);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.ModificationDate"), ColumnID::ModificationDate, KxDATAVIEW_CELL_INERT, 125);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Type"), ColumnID::Type, KxDATAVIEW_CELL_INERT, 175);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Size"), ColumnID::Size, KxDATAVIEW_CELL_INERT, 125);
}

void KModCollisionViewerModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const ModelEntry* entry = GetDataEntry(row);
	if (entry)
	{
		const KxFileItem& fileItem = entry->GetFileItem();
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				wxIcon icon;
				if (fileItem.IsFile())
				{
					icon = KxShell::GetFileIcon(fileItem.GetFullPath(), true);
					if (!icon.IsOk())
					{
						icon = KGetIcon(KIMG_DOCUMENT);
					}
				}
				else
				{
					icon = KGetIcon(KIMG_FOLDER);
				}

				value = KxDataViewBitmapTextValue(entry->GetRelativePath(), icon);
				break;
			}
			case ColumnID::Collisions:
			{
				value = FormatCollisionsCount(entry->GetCollisions());
				break;
			}
			case ColumnID::ModificationDate:
			{
				value = KAux::FormatDateTime(fileItem.GetModificationTime());
				break;
			}
			case ColumnID::Type:
			{
				value = KxShell::GetTypeName(fileItem.GetName());
				break;
			}
			case ColumnID::Size:
			{
				value = KxFile::FormatFileSize(fileItem.GetFileSize());
				break;
			}
		};
	}
}
bool KModCollisionViewerModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	return false;
}
bool KModCollisionViewerModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	return true;
}

void KModCollisionViewerModel::OnSelectItem(KxDataViewEvent& event)
{
}
void KModCollisionViewerModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	const ModelEntry* entry = GetDataEntry(GetRow(item));
	if (entry && entry->HasCollisions())
	{
		KxTaskDialog dialog(GetViewTLW(), KxID_NONE, column->GetTitle(), FormatCollisionsView(entry->GetCollisions()));
		dialog.ShowModal();
	}
}
void KModCollisionViewerModel::OnContextMenu(KxDataViewEvent& event)
{
}

void KModCollisionViewerModel::RunCollisionsSearch(KOperationWithProgressBase* context)
{
	for (const KFileTreeNode* fileNode: KModManager::GetDispatcher().FindFiles(*m_ModEntry, KxFile::NullFilter, KxFS_FILE, true))
	{
		if (!context->CanContinue())
		{
			m_DataVector.clear();
			return;
		}

		ModelEntry& modelEntry = m_DataVector.emplace_back(fileNode->GetItem());
		modelEntry.FindCollisions(*m_ModEntry);
		if (!modelEntry.HasCollisions())
		{
			m_DataVector.pop_back();
		}
	}
	SetDataVector(&m_DataVector);
}

KModCollisionViewerModel::KModCollisionViewerModel(const KModEntry* modEntry)
	:m_ModEntry(modEntry)
{
}

//////////////////////////////////////////////////////////////////////////
void KModCollisionViewerModelDialog::RunThread()
{
	auto operation = new KOperationWithProgressDialog<>(true, KxStdDialog::GetParent());
	operation->OnRun([this](KOperationWithProgressBase* self)
	{
		RunCollisionsSearch(self);
	});
	operation->OnEnd([this](KOperationWithProgressBase* self)
	{
		if (GetItemCount() != 0)
		{
			SetLabel(KxFormat("%1: %2").arg(T("ModExplorer.Collisions.Count")).arg(GetItemCount()));
			KProgramOptionSerializer::LoadDataViewLayout(GetView(), m_ViewOptions);
			KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);

			ShowModal();
		}
		else
		{
			Close(true);
			KxTaskDialog dialog(KxStdDialog::GetParent(), KxID_NONE, GetCaption(), T("ModExplorer.Collisions.NoneFound"));
			dialog.ShowModal();
		}
	});
	operation->SetDialogCaption(T("ModExplorer.Collisions.Searching"));
	operation->Run();
}

KModCollisionViewerModelDialog::KModCollisionViewerModelDialog(wxWindow* window, const KModEntry* modEntry)
	:KModCollisionViewerModel(modEntry),
	m_ViewOptions("KModCollisionViewerModelDialog", "CollisionsListView"),
	m_WindowOptions("KModCollisionViewerModelDialog", "Window")
{
	wxString caption = wxString::Format("%s \"%s\"", T("ModExplorer.Collisions"), modEntry->GetName());
	if (KxStdDialog::Create(window, KxID_NONE, caption, wxDefaultPosition, wxDefaultSize, KxBTN_OK))
	{
		SetMainIcon(KxICON_NONE);
		SetWindowResizeSide(wxBOTH);

		// List
		KModCollisionViewerModel::Create(GetContentWindow());
		m_ViewPane = GetView();

		PostCreate();
		SetLabel(" ");
		AdjustWindow(wxDefaultPosition, wxSize(900, 500));
		RunThread();
	}
}
KModCollisionViewerModelDialog::~KModCollisionViewerModelDialog()
{
	IncRef();

	KProgramOptionSerializer::SaveDataViewLayout(GetView(), m_ViewOptions);
	KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
}
