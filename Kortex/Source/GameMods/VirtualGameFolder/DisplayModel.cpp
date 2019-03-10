#include "stdafx.h"
#include "DisplayModel.h"
#include <Kortex/ModManager.hpp>
#include "Utility/KAux.h"
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxLibrary.h>
#include <wx/mimetype.h>

using namespace Kortex::ModManager;

namespace
{
	enum ColumnID
	{
		Name,
		PartOf,
		Size,
		ModificationDate,
		SourceLocation,
	};
}

namespace Kortex::VirtualGameFolder
{
	void DisplayModel::OnInitControl()
	{
		/* View */
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &DisplayModel::OnSelectItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &DisplayModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &DisplayModel::OnContextMenu, this);
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

	bool DisplayModel::IsContainer(const KxDataViewItem& item) const
	{
		if (const FileTreeNode* node = GetNode(item))
		{
			return node->HasChildren();
		}
		return false;
	}
	KxDataViewItem DisplayModel::GetParent(const KxDataViewItem& item) const
	{
		if (const FileTreeNode* node = GetNode(item))
		{
			if (node->HasParent())
			{
				return MakeItem(node->GetParent());
			}
		}
		return KxDataViewItem();
	}
	void DisplayModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
	{
		// Root item
		if (item.IsTreeRootItem())
		{
			children.reserve(m_TreeItems->size());
			for (const FileTreeNode& childNode: *m_TreeItems)
			{
				if (childNode.GetMod().IsActive())
				{
					children.push_back(MakeItem(childNode));
				}
			}
			return;
		}

		// Child item
		if (const FileTreeNode* node = GetNode(item))
		{
			children.reserve(node->GetChildrenCount());
			for (const FileTreeNode& childNode: node->GetChildren())
			{
				if (childNode.GetMod().IsActive())
				{
					children.push_back(MakeItem(childNode));
				}
			}
		}
	}

	void DisplayModel::GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		if (const FileTreeNode* node = GetNode(item))
		{
			const IGameMod& mod = node->GetMod();
			switch (column->GetID())
			{
				case ColumnID::PartOf:
				{
					KxStringVector items(1, mod.GetName());
					for (const FileTreeNode& currentNode: node->GetAlternatives())
					{
						const IGameMod& currentNodeMod = currentNode.GetMod();
						if (currentNodeMod.IsActive())
						{
							items.push_back(KxString::Format(wxS("%1. %2"), items.size(), currentNodeMod.GetName()));
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
	void DisplayModel::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		if (const FileTreeNode* node = GetNode(item))
		{
			const IGameMod& mod = node->GetMod();
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					const auto it = m_IconCache.find(node);
					if (it != m_IconCache.end())
					{
						value = KxDataViewBitmapTextValue(node->GetName(), it->second);
					}
					else
					{
						wxBitmap bitmap = KxShell::GetFileIcon(node->GetFullPath(), true);
						if (!bitmap.IsOk())
						{
							// Couldn't get bitmap from system, use our own
							bitmap = KGetBitmap(node->IsDirectory() ? KIMG_FOLDER : KIMG_DOCUMENT);
						}
						m_IconCache.insert_or_assign(node, bitmap);

						value = KxDataViewBitmapTextValue(node->GetName(), bitmap);
					}
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
	bool DisplayModel::SetValue(const wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column)
	{
		return false;
	}
	bool DisplayModel::IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		return true;
	}

	bool DisplayModel::GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
	{
		const FileTreeNode* node = GetNode(item);
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
	bool DisplayModel::Compare(const KxDataViewItem& item1, const KxDataViewItem& item2, const KxDataViewColumn* column) const
	{
		const FileTreeNode& node1 = *GetNode(item1);
		const FileTreeNode& node2 = *GetNode(item2);

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
				const IGameMod& mod1 = node1.GetMod();
				const IGameMod& mod2 = node2.GetMod();
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

	void DisplayModel::OnSelectItem(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		KxDataViewColumn* column = event.GetColumn();
		FileTreeNode* node = GetNode(item);

		if (node && column && column->GetID() == ColumnID::PartOf)
		{
			Kortex::ModManager::Workspace* workspace = Kortex::ModManager::Workspace::GetInstance();
			wxWindowUpdateLocker lock(workspace);

			workspace->HighlightMod();
			workspace->HighlightMod(&node->GetMod());
		}
	}
	void DisplayModel::OnActivateItem(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		KxDataViewColumn* column = event.GetColumn();
		FileTreeNode* node = GetNode(item);

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
	void DisplayModel::OnContextMenu(KxDataViewEvent& event)
	{
	}

	DisplayModel::DisplayModel()
	{
		SetDataViewFlags(KxDataViewCtrl::DefaultStyle|KxDV_DOUBLE_CLICK_EXPAND|KxDV_VERT_RULES);
	}

	void DisplayModel::RefreshItems()
	{
		m_IconCache.clear();
		m_FoundItems.clear();
		ItemsCleared();

		if (!m_SearchMask.IsEmpty())
		{
			m_TreeItems = &m_FoundItems;

			FileTreeNode::CRefVector files = IModDispatcher::GetInstance()->Find(wxEmptyString, ModManager::DispatcherSearcher(m_SearchMask, KxFS_FILE), true);
			m_FoundItems.reserve(files.size());

			for (const FileTreeNode* node: files)
			{
				FileTreeNode& foundNode = m_FoundItems.emplace_back(FileTreeNode(node->GetMod(), node->GetItem()));
				ItemAdded(MakeItem(foundNode));
			}
		}
		else
		{
			m_TreeItems = &IModDispatcher::GetInstance()->GetVirtualTree().GetChildren();
			for (const FileTreeNode& node: *m_TreeItems)
			{
				ItemAdded(MakeItem(node));
			}
		}

		// Reset scrolling
		GetView()->Scroll(0, 0);
	}
	bool DisplayModel::SetSearchMask(const wxString& mask)
	{
		return KAux::SetSearchMask(m_SearchMask, mask);
	}
}
