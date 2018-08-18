#include "stdafx.h"
#include "KModFilesExplorerModel.h"
#include "KModCollisionViewerModel.h"
#include "UI/KMainWindow.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxShell.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxString.h>

enum ColumnID
{
	Name,
	Collisions,
	ModificationDate,
	Type,
	Size,
};

void KModFilesExplorerModel::OnInitControl()
{
	/* View */
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KModFilesExplorerModel::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KModFilesExplorerModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KModFilesExplorerModel::OnContextMenu, this);
	GetView()->SetIndent(20);

	/* Columns */
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_INERT, 300);
	if (HasModEntry())
	{
		GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(T("ModExplorer.Collisions"), ColumnID::Collisions, KxDATAVIEW_CELL_INERT, 200);
	}
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(T("Generic.ModificationDate"), ColumnID::ModificationDate, KxDATAVIEW_CELL_INERT, 125);
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(T("Generic.Type"), ColumnID::Type, KxDATAVIEW_CELL_INERT, 175);
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(T("Generic.Size"), ColumnID::Size, KxDATAVIEW_CELL_INERT, 125);
}

bool KModFilesExplorerModel::IsContainer(const KxDataViewItem& item) const
{
	if (const KMFEModelNode* node = GetNode(item))
	{
		return node->HasChildren();
	}
	return false;
}
KxDataViewItem KModFilesExplorerModel::GetParent(const KxDataViewItem& item) const
{
	if (const KMFEModelNode* node = GetNode(item))
	{
		if (node->HasParentNode())
		{
			return MakeItem(node->GetParentNode());
		}
	}
	return KxDataViewItem();
}
void KModFilesExplorerModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
{
	// Root item, read groups
	if (item.IsTreeRootItem())
	{
		children.reserve(m_Entries.size());
		for (const KMFEModelNode& node: m_Entries)
		{
			children.push_back(MakeItem(node));
		}
		return;
	}

	// Group item, read entries
	if (const KMFEModelNode* node = GetNode(item))
	{
		children.reserve(node->GetChildrenCount());
		for (const KMFEModelNode& node: node->GetChildren())
		{
			children.push_back(MakeItem(node));
		}
	}
}

void KModFilesExplorerModel::GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	KMFEModelNode* node = GetNode(item);
	if (KxFileFinderItem* item = node->GetFSItem())
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = item->GetName();
				return;
			}
		};
	}
	GetValue(value, item, column);
}
void KModFilesExplorerModel::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	KMFEModelNode* node = GetNode(item);
	if (KxFileFinderItem* item = node->GetFSItem())
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = KxDataViewBitmapTextValue(item->GetName(), KGetBitmap(item->IsDirectory() ? KIMG_FOLDER : KIMG_DOCUMENT));
				break;
			}
			case ColumnID::Collisions:
			{
				if (HasModEntry())
				{
					value = KModCollisionViewerModel::FormatCollisionsCount(node->GetCollisions(m_ModEntry));
				}
				break;
			}
			case ColumnID::ModificationDate:
			{
				value = KAux::FormatDateTime(item->GetModificationTime());
				break;
			}
			case ColumnID::Type:
			{
				value = item->IsDirectory() ? T(KxID_FOLDER) : KxShell::GetTypeName(item->GetName());
				break;
			}
			case ColumnID::Size:
			{
				value = item->IsFile() ? KxFile::FormatFileSize(item->GetFileSize()) : wxEmptyString;
				break;
			}
		};
	}
}
bool KModFilesExplorerModel::SetValue(const wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column)
{
	return false;
}
bool KModFilesExplorerModel::IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	if (column->GetID() == ColumnID::Name)
	{
		KMFEModelNode* node = GetNode(item);
		return node && node->IsOK();
	}
	return false;
}

void KModFilesExplorerModel::OnSelectItem(KxDataViewEvent& event)
{
}
void KModFilesExplorerModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	KMFEModelNode* node = GetNode(item);
	KxFileFinderItem* itemFS = node->GetFSItem();

	if (node->IsDirectory())
	{
		GetView()->ToggleItemExpanded(item);
	}
	else if (itemFS && column)
	{
		switch (column->GetID())
		{
			case ColumnID::Collisions:
			{
				if (HasModEntry() && node->HasCollisions())
				{
					KxTaskDialog dialog(GetViewTLW(), KxID_NONE, column->GetTitle(), KModCollisionViewerModel::FormatCollisionsView(node->GetCollisions(m_ModEntry)));
					dialog.ShowModal();
				}
				break;
			}
		};
	}
}
void KModFilesExplorerModel::OnContextMenu(KxDataViewEvent& event)
{
	const KMFEModelNode* node = GetNode(event.GetItem());
	if (node)
	{
	}
}

KModFilesExplorerModel::KModFilesExplorerModel(const wxString& sExplorerRoot, const KModEntry* modEntry)
	:m_ExplorerRoot(sExplorerRoot), m_ModEntry(modEntry)
{
}

void KModFilesExplorerModel::RefreshItems()
{
	ItemsCleared();
	m_Entries.clear();

	// Call with NULL vector and node to precalc items count
	std::function<size_t(const wxString&, KMFEModelNode::NodeVector*, const KMFEModelNode* p)> Recurse;
	Recurse = [this, &Recurse](const wxString& path, KMFEModelNode::NodeVector* entries, const KMFEModelNode* parentNode = NULL) -> size_t
	{
		if (entries)
		{
			entries->reserve(Recurse(path, NULL, NULL));
		}

		size_t count = 0;

		KxFileFinder filder(path, "*");
		KxFileFinderItem item = filder.FindNext();

		while (item.IsOK())
		{
			if (item.IsNormalItem())
			{
				count++;

				KMFEModelNode* node = NULL;
				if (entries)
				{
					node = &entries->emplace_back(item);
				}

				if (node && item.IsDirectory())
				{
					Recurse(item.GetFullPath(), &node->GetChildren(), node);
				}
			}

			item = filder.FindNext();
		}

		if (entries)
		{
			for (auto& node: *entries)
			{
				node.SetParentNode(parentNode);
				ItemAdded(MakeItem(parentNode), MakeItem(node));
			}
		}

		return count;
	};
	Recurse(m_ExplorerRoot, &m_Entries, NULL);
}
