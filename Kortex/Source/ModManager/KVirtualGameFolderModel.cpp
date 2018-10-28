#include "stdafx.h"
#include "KVirtualGameFolderModel.h"
#include "KModManager.h"
#include "KDispatcher.h"
#include "KModWorkspace.h"
#include "KFileTreeNode.h"
#include "KModEntry.h"
#include "KAux.h"
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxLibrary.h>
#include <wx/mimetype.h>

enum ColumnID
{
	Name,
	PartOf,
	Size,
	ModificationDate,
	SourceLocation,
};

void KVirtualGameFolderModel::OnInitControl()
{
	/* View */
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KVirtualGameFolderModel::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KVirtualGameFolderModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KVirtualGameFolderModel::OnContextMenu, this);
	GetView()->Bind(KxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, [this](KxDataViewEvent& event)
	{
		KxMenu menu;
		if (GetView()->CreateColumnSelectionMenu(menu))
		{
			GetView()->OnColumnSelectionMenu(menu);
		}
	});
	GetView()->SetIndent(20);

	/* Columns */
	KxDataViewColumnFlags flags = KxDV_COL_DEFAULT_FLAGS|KxDV_COL_SORTABLE;
	{
		auto info = GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_INERT, 300, flags);
		info.GetRenderer()->SetOptionEnabled(KxDataViewRendererOptions::KxDVR_ALLOW_BITMAP_SCALEDOWN);
	}
	{
		auto info = GetView()->AppendColumn<KxDataViewBitmapTextRenderer, KxDataViewComboBoxEditor>(KTr("Generic.PartOf"), ColumnID::PartOf, KxDATAVIEW_CELL_EDITABLE, 300, flags);
		m_PartOfEditor = info.GetEditor();
		info.GetColumn()->SortAscending();
	}
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(KTr("Generic.Size"), ColumnID::Size, KxDATAVIEW_CELL_INERT, 100, flags);
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(KTr("Generic.ModificationDate"), ColumnID::ModificationDate, KxDATAVIEW_CELL_INERT, 125, flags);
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(KTr("Generic.SourceLocation"), ColumnID::SourceLocation, KxDATAVIEW_CELL_INERT, 100, flags);
}

bool KVirtualGameFolderModel::IsContainer(const KxDataViewItem& item) const
{
	if (const KFileTreeNode* node = GetNode(item))
	{
		return node->HasChildren();
	}
	return false;
}
KxDataViewItem KVirtualGameFolderModel::GetParent(const KxDataViewItem& item) const
{
	if (const KFileTreeNode* node = GetNode(item))
	{
		if (node->HasParent())
		{
			return MakeItem(node->GetParent());
		}
	}
	return KxDataViewItem();
}
void KVirtualGameFolderModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
{
	// Root item
	if (item.IsTreeRootItem())
	{
		children.reserve(m_TreeItems->size());
		for (const KFileTreeNode& childNode: *m_TreeItems)
		{
			if (childNode.GetMod().IsEnabled())
			{
				children.push_back(MakeItem(childNode));
			}
		}
		return;
	}

	// Child item
	if (const KFileTreeNode* node = GetNode(item))
	{
		children.reserve(node->GetChildrenCount());
		for (const KFileTreeNode& childNode: node->GetChildren())
		{
			if (childNode.GetMod().IsEnabled())
			{
				children.push_back(MakeItem(childNode));
			}
		}
	}
}

void KVirtualGameFolderModel::GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	if (const KFileTreeNode* node = GetNode(item))
	{
		const KModEntry& mod = node->GetMod();
		switch (column->GetID())
		{
			case ColumnID::PartOf:
			{
				KxStringVector items(1, mod.GetName());
				for (const KFileTreeNode& currentNode: node->GetAlternatives())
				{
					const KModEntry& currentNodeMod = currentNode.GetMod();
					if (currentNodeMod.IsEnabled())
					{
						items.push_back(KxFormat(wxS("%1. %2")).arg(items.size()).arg(currentNodeMod.GetName()));
					}
				}
				m_PartOfEditor->SetItems(items);

				// Item index
				value = 0;
				break;
			}
		};
	}
}
void KVirtualGameFolderModel::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	if (const KFileTreeNode* node = GetNode(item))
	{
		const KModEntry& mod = node->GetMod();
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				wxBitmap bitmap;
				if (node->IsDirectory())
				{
					bitmap = KGetBitmap(KIMG_FOLDER);
				}
				else
				{
					bitmap = KxShell::GetFileIcon(node->GetFullPath(), true);
					if (!bitmap.IsOk())
					{
						bitmap = KGetBitmap(KIMG_DOCUMENT);
					}
				}
				value = KxDataViewBitmapTextValue(node->GetName(), bitmap);
				break;
			}
			case ColumnID::PartOf:
			{
				KxDataViewBitmapTextValue data(mod.GetName());
				if (node->HasAlternativesFromActiveMods())
				{
					data.SetBitmap(KGetBitmap(node->IsDirectory() ? KIMG_EXCLAMATION_CIRCLE_FRAME_EMPTY : KIMG_EXCLAMATION));
				}
				value = data;
				break;
			}
			case ColumnID::Size:
			{
				if (node->IsFile())
				{
					value = KxFile::FormatFileSize(node->GetFileSize(), 2);
				}
				break;
			}
			case ColumnID::ModificationDate:
			{
				value = KAux::FormatDateTime(node->GetItem().GetModificationTime());
				break;
			}
			case ColumnID::SourceLocation:
			{
				value = node->GetRelativePath();
				break;
			}
		};
	}
}
bool KVirtualGameFolderModel::SetValue(const wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column)
{
	return false;
}
bool KVirtualGameFolderModel::IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	return true;
}

bool KVirtualGameFolderModel::GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
{
	const KFileTreeNode* node = GetNode(item);
	switch (column->GetID())
	{
		case ColumnID::PartOf:
		{
			if (cellState & KxDATAVIEW_CELL_HIGHLIGHTED && column->IsHotTracked())
			{
				attributes.SetUnderlined();
				return true;
			}
			break;
		}
	};
	return false;
}
bool KVirtualGameFolderModel::Compare(const KxDataViewItem& item1, const KxDataViewItem& item2, const KxDataViewColumn* column) const
{
	const KFileTreeNode& node1 = *GetNode(item1);
	const KFileTreeNode& node2 = *GetNode(item2);

	// Folders first
	if (node1.IsFile() != node2.IsFile())
	{
		return node1.IsFile() < node2.IsFile();
	}

	switch (column ? column->GetID() : ColumnID::PartOf)
	{
		case ColumnID::Name:
		{
			return KxComparator::IsLess(node1.GetName(), node2.GetName());
		}
		case ColumnID::PartOf:
		{
			const KModEntry& mod1 = node1.GetMod();
			const KModEntry& mod2 = node2.GetMod();
			return mod1.GetOrderIndex() < mod2.GetOrderIndex();
		}
		case ColumnID::Size:
		{
			if (node1.IsFile() && node2.IsFile())
			{
				return node1.GetFileSize() < node2.GetFileSize();
			}
		}
		case ColumnID::ModificationDate:
		{
			return node1.GetItem().GetModificationTime() < node2.GetItem().GetModificationTime();
		}
	};
	return false;
}

void KVirtualGameFolderModel::OnSelectItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	KFileTreeNode* node = GetNode(item);

	if (node && column && column->GetID() == ColumnID::PartOf)
	{
		KModWorkspace* workspace = KModWorkspace::GetInstance();
		wxWindowUpdateLocker lock(workspace);

		workspace->HighlightMod();
		workspace->HighlightMod(&node->GetMod());
	}
}
void KVirtualGameFolderModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	KFileTreeNode* node = GetNode(item);

	if (node && column)
	{
		if (node->IsDirectory())
		{
			KxShell::Execute(GetViewTLW(), node->GetFullPath(), "open");
		}
		else
		{
			KxShell::OpenFolderAndSelectItem(node->GetFullPath());
		}
	}
}
void KVirtualGameFolderModel::OnContextMenu(KxDataViewEvent& event)
{
}

KVirtualGameFolderModel::KVirtualGameFolderModel()
{
	SetDataViewFlags(KxDataViewCtrl::DefaultStyle|KxDV_DOUBLE_CLICK_EXPAND|KxDV_VERT_RULES);
}

void KVirtualGameFolderModel::RefreshItems()
{
	m_FoundItems.clear();
	ItemsCleared();

	if (!m_SearchMask.IsEmpty())
	{
		m_TreeItems = &m_FoundItems;

		KFileTreeNode::CRefVector files = KDispatcher::GetInstance()->FindFiles(wxEmptyString, m_SearchMask, KxFS_FILE, true);
		m_FoundItems.reserve(files.size());

		for (const KFileTreeNode* node: files)
		{
			KFileTreeNode& foundNode = m_FoundItems.emplace_back(KFileTreeNode(node->GetMod(), node->GetItem()));
			ItemAdded(MakeItem(foundNode));
		}
	}
	else
	{
		m_TreeItems = &KDispatcher::GetInstance()->GetVirtualTree().GetChildren();
		for (const KFileTreeNode& node: *m_TreeItems)
		{
			if (node.GetMod().IsEnabled())
			{
				ItemAdded(MakeItem(node));
			}
		}
	}
	
	// Reset scrolling
	GetView()->Scroll(0, 0);
}
bool KVirtualGameFolderModel::SetSearchMask(const wxString& mask)
{
	return KAux::SetSearchMask(m_SearchMask, mask);
}
