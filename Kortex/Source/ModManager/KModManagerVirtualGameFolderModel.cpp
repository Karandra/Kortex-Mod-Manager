#include "stdafx.h"
#include "KModManagerVirtualGameFolderModel.h"
#include "KModManager.h"
#include "KModManagerDispatcher.h"
#include "KModManagerWorkspace.h"
#include "KFileTreeNode.h"
#include "KModEntry.h"
#include "KComparator.h"
#include "KAux.h"
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

void KModManagerVirtualGameFolderModel::OnInitControl()
{
	/* View */
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KModManagerVirtualGameFolderModel::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KModManagerVirtualGameFolderModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KModManagerVirtualGameFolderModel::OnContextMenu, this);
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
		auto info = GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_INERT, 300, flags);
		info.GetRenderer()->SetOptionEnabled(KxDataViewRendererOptions::KxDVR_ALLOW_BITMAP_SCALEDOWN);
	}
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewComboBoxEditor>(T("Generic.PartOf"), ColumnID::PartOf, KxDATAVIEW_CELL_EDITABLE, 300, flags);
		m_PartOfEditor = info.GetEditor();
		info.GetColumn()->SortAscending();
	}
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(T("Generic.Size"), ColumnID::Size, KxDATAVIEW_CELL_INERT, 100, flags);
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(T("Generic.ModificationDate"), ColumnID::ModificationDate, KxDATAVIEW_CELL_INERT, 125, flags);
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(T("Generic.SourceLocation"), ColumnID::SourceLocation, KxDATAVIEW_CELL_INERT, 100, flags);
}

bool KModManagerVirtualGameFolderModel::IsContainer(const KxDataViewItem& item) const
{
	if (const KFileTreeNode* node = GetNode(item))
	{
		return node->HasChildren();
	}
	return false;
}
KxDataViewItem KModManagerVirtualGameFolderModel::GetParent(const KxDataViewItem& item) const
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
void KModManagerVirtualGameFolderModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
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

void KModManagerVirtualGameFolderModel::GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	if (const KFileTreeNode* node = GetNode(item))
	{
		switch (column->GetID())
		{
			case ColumnID::PartOf:
			{
				KxStringVector items(1, node->GetMod().GetName());
				for (const KFileTreeNode& node: node->GetAlternatives())
				{
					items.push_back(node.GetMod().GetName());
				}
				m_PartOfEditor->SetItems(items);

				// Item index
				value = 0;
				break;
			}
		};
	}
}
void KModManagerVirtualGameFolderModel::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	if (const KFileTreeNode* node = GetNode(item))
	{
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
				value = node->GetMod().GetName();
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
bool KModManagerVirtualGameFolderModel::SetValue(const wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column)
{
	return false;
}
bool KModManagerVirtualGameFolderModel::IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	return true;
}

bool KModManagerVirtualGameFolderModel::GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
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
bool KModManagerVirtualGameFolderModel::Compare(const KxDataViewItem& item1, const KxDataViewItem& item2, const KxDataViewColumn* column) const
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
			return KComparator::KLess(node1.GetName(), node2.GetName());
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

void KModManagerVirtualGameFolderModel::OnSelectItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	KFileTreeNode* node = GetNode(item);

	if (node && column && column->GetID() == ColumnID::PartOf)
	{
		KModManagerWorkspace* workspace = KModManagerWorkspace::GetInstance();
		wxWindowUpdateLocker lock(workspace);

		workspace->HighlightMod();
		workspace->HighlightMod(&node->GetMod());
	}
}
void KModManagerVirtualGameFolderModel::OnActivateItem(KxDataViewEvent& event)
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
void KModManagerVirtualGameFolderModel::OnContextMenu(KxDataViewEvent& event)
{
}

KModManagerVirtualGameFolderModel::KModManagerVirtualGameFolderModel()
{
	SetDataViewFlags(KxDataViewCtrl::DefaultStyle|KxDV_DOUBLE_CLICK_EXPAND|KxDV_VERT_RULES);
}

void KModManagerVirtualGameFolderModel::RefreshItems()
{
	m_FoundItems.clear();
	ItemsCleared();

	if (!m_SearchMask.IsEmpty())
	{
		m_TreeItems = &m_FoundItems;

		KFileTreeNode::CRefVector files = KModManager::GetDispatcher().FindFiles(wxEmptyString, m_SearchMask, KxFS_FILE, true);
		m_FoundItems.reserve(files.size());

		for (const KFileTreeNode* node: files)
		{
			KFileTreeNode& foundNode = m_FoundItems.emplace_back(KFileTreeNode(node->GetMod(), node->GetItem()));
			ItemAdded(MakeItem(foundNode));
		}
	}
	else
	{
		m_TreeItems = &KModManager::GetDispatcher().GetVirtualTree().GetChildren();
		for (const KFileTreeNode& node: KModManager::GetDispatcher().GetVirtualTree().GetChildren())
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
bool KModManagerVirtualGameFolderModel::SetSearchMask(const wxString& mask)
{
	return KAux::SetSearchMask(m_SearchMask, mask);
}
