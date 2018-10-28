#include "stdafx.h"
#include "KPCComponentsModel.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "PackageCreator/KPackageCreatorController.h"
#include "PackageCreator/KPackageCreatorPageComponents.h"
#include "PackageCreator/PageInterface/KPCIImagesListModel.h"
#include "KPCCFileDataSelectorModel.h"
#include "KPCCRequirementsSelectorModel.h"
#include "KPCCFlagsSelectorModel.h"
#include "UI/KMainWindow.h"
#include "UI/KTextEditorDialog.h"
#include "UI/KImageViewerDialog.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxTextBoxDialog.h>
#include <KxFramework/KxComboBoxDialog.h>
#include <KxFramework/DataView/KxDataViewMainWindow.h>

namespace
{
	template<class ArrayT, class ValueT> auto FindElement(ArrayT& array, const ValueT& value)
	{
		return std::find_if(array.begin(), array.end(), [value](const auto& v)
		{
			return v.get() == value;
		});
	};
	
	template<class EntryT> void SwapNodesInSameBranch(KPCComponentsModelNode* thisNode, KPCComponentsModelNode* draggedNode, std::vector<std::unique_ptr<EntryT>>* entries = NULL)
	{
		EntryT* thisEntry = NULL;
		EntryT* draggedEntry = NULL;

		if constexpr(std::is_same<EntryT, KPPCStep>::value)
		{
			thisEntry = thisNode->GetStep();
			draggedEntry = draggedNode->GetStep();
		}
		else if constexpr(std::is_same<EntryT, KPPCGroup>::value)
		{
			entries = &thisNode->GetParent()->GetStep()->GetGroups();

			thisEntry = thisNode->GetGroup();
			draggedEntry = draggedNode->GetGroup();
		}
		else if constexpr(std::is_same<EntryT, KPPCEntry>::value)
		{
			entries = &thisNode->GetParent()->GetGroup()->GetEntries();

			thisEntry = thisNode->GetEntry();
			draggedEntry = draggedNode->GetEntry();
		}

		if (thisEntry && entries)
		{
			auto thisNodeIter = FindElement(*entries, thisEntry);
			auto draggedNodeIter = FindElement(*entries, draggedEntry);
			std::iter_swap(thisNodeIter, draggedNodeIter);
		}
	}
	template<class EntryT> void MoveNodeToDifferentBranch(KPCComponentsModelNode* thisNode, KPCComponentsModelNode* draggedNode, KPCComponentsModel* model)
	{
		std::vector<std::unique_ptr<EntryT>>* thisItems = NULL;
		std::vector<std::unique_ptr<EntryT>>* draggedItems = NULL;
		
		EntryT* thisEntry = NULL;
		EntryT* draggedEntry = NULL;

		if constexpr(std::is_same<EntryT, KPPCGroup>::value)
		{
			thisItems = &thisNode->GetParent()->GetStep()->GetGroups();
			draggedItems = &draggedNode->GetParent()->GetStep()->GetGroups();

			thisEntry = thisNode->GetGroup();
			draggedEntry = draggedNode->GetGroup();
		}
		else if constexpr(std::is_same<EntryT, KPPCEntry>::value)
		{
			thisItems = &thisNode->GetParent()->GetGroup()->GetEntries();
			draggedItems = &draggedNode->GetParent()->GetGroup()->GetEntries();

			thisEntry = thisNode->GetEntry();
			draggedEntry = draggedNode->GetEntry();
		}

		if (thisItems && draggedItems)
		{
			// Move project items
			auto draggedGroupIt = FindElement(*draggedItems, draggedEntry);
			draggedGroupIt->release();
			draggedItems->erase(draggedGroupIt);

			auto thisGroupIt = FindElement(*thisItems, thisEntry);
			thisItems->emplace(thisGroupIt, draggedEntry);

			// Now move view tree nodes
			{
				KPCComponentsModelNode::Vector& thisNodes = thisNode->GetParent()->GetChildren();
				KPCComponentsModelNode::Vector& draggedNodes = draggedNode->GetParent()->GetChildren();

				auto draggedNodeIt = FindElement(draggedNodes, draggedNode);
				draggedNodeIt->release();
				draggedNodes.erase(draggedNodeIt);
				model->ItemDeleted(model->GetItem(draggedNode->GetParent()), model->GetItem(draggedNode));

				auto thisNodeIt = FindElement(thisNodes, thisNode);
				auto& draggedNodeNew = *thisNodes.emplace(thisNodeIt, draggedNode);
				draggedNodeNew->SetParent(thisNode->GetParent());

				KxDataViewItem newItem = model->GetItem(draggedNodeNew.get());
				model->ItemAdded(model->GetItem(thisNode->GetParent()), newItem);
			}
		}
	}

	KPCComponentsModelNode* GetParentStep(KPCComponentsModelNode* node)
	{
		KPCComponentsModelNode* parent = node;
		while (parent && !parent->GetStep())
		{
			parent = parent->GetParent();
		}
		return parent;
	}
	KPCComponentsModelNode* GetParentGroup(KPCComponentsModelNode* node)
	{
		KPCComponentsModelNode* parent = node;
		while (parent && !parent->GetGroup())
		{
			parent = parent->GetParent();
		}
		return parent;
	}
	KPCComponentsModelNode* GetParentEntry(KPCComponentsModelNode* node)
	{
		KPCComponentsModelNode* parent = node;
		while (parent && !parent->GetEntry())
		{
			parent = parent->GetParent();
		}
		return parent;
	}
}

enum ColumnID
{
	Name,
	Value,
};
enum MenuID
{
	AddStep,
	AddGroup,
	AddEntry,

	AddEntriesFromFiles,

	AllSteps_Name,
	AllSteps_Conditions,

	AllGroups_Name,
	AllGroups_SelectionMode,

	AllEntries,
};

void KPCComponentsModel::OnInitControl()
{
	/* View */
	EnableDragAndDrop();
	GetView()->SetIndent(KGetImageList()->GetSize().GetWidth());
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KPCComponentsModel::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KPCComponentsModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KPCComponentsModel::OnContextMenu, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_EDIT_ATTACH, &KPCComponentsModel::OnAttachEditor, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_EDIT_DETACH, &KPCComponentsModel::OnDetachEditor, this);

	/* Columns */
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 300);
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Value"), ColumnID::Value, KxDATAVIEW_CELL_EDITABLE, 500);
		info.GetColumn()->UseDynamicEditor(true);
	}
}

bool KPCComponentsModel::IsContainer(const KxDataViewItem& item) const
{
	if (item.IsTreeRootItem())
	{
		return true;
	}
	else if (KPCComponentsModelNode* node = GetNode(item))
	{
		return node->HasChildren();
	}
	return false;
}
void KPCComponentsModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
{
	if (item.IsTreeRootItem())
	{
		for (const auto& childNode: m_Steps)
		{
			children.push_back(GetItem(childNode.get()));
		}
	}
	else if (KPCComponentsModelNode* node = GetNode(item))
	{
		for (const auto& childNode: node->GetChildren())
		{
			children.push_back(GetItem(childNode.get()));
		}
	}
}
KxDataViewItem KPCComponentsModel::GetParent(const KxDataViewItem& item) const
{
	if (KPCComponentsModelNode* node = GetNode(item))
	{
		return GetItem(node->GetParent());
	}
	return KxDataViewItem();
}
bool KPCComponentsModel::HasContainerColumns(const KxDataViewItem& item) const
{
	return true;
}

bool KPCComponentsModel::IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	return true;
}
bool KPCComponentsModel::IsEditorEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	if (KPCComponentsModelNode* node = GetNode(item))
	{
		if (const KPPCStep* step = node->GetStep())
		{
			return column->GetID() == ColumnID::Name;
		}
		else if (const KPPCGroup* group = node->GetGroup())
		{
			return true;
		}
		else if (const KPPCEntry* entry = node->GetEntry())
		{
			return column->GetID() == ColumnID::Name;
		}
		else if (node->IsEntryItem())
		{
			return column->GetID() == ColumnID::Value;
		}
	}
	return false;
}
void KPCComponentsModel::GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	if (KPCComponentsModelNode* node = GetNode(item))
	{
		if (const KPPCStep* step = node->GetStep())
		{
			GetStepValue(value, column, step, true);
		}
		else if (const KPPCGroup* group = node->GetGroup())
		{
			GetGroupValue(value, column, group, true);
		}
		else if (const KPPCEntry* entry = node->GetEntry())
		{
			GetEntryValue(value, column, entry, true);
		}
		else if (node->IsEntryItem())
		{
			GetEntryItemValue(value, column, node->GetParent()->GetEntry(), node->GetEntryItemID(), true);
		}
	}
}
void KPCComponentsModel::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	if (KPCComponentsModelNode* node = GetNode(item))
	{
		if (const KPPCStep* step = node->GetStep())
		{
			GetStepValue(value, column, step);
		}
		else if (const KPPCGroup* group = node->GetGroup())
		{
			GetGroupValue(value, column, group);
		}
		else if (const KPPCEntry* entry = node->GetEntry())
		{
			GetEntryValue(value, column, entry);
		}
		else if (node->IsEntryItem())
		{
			GetEntryItemValue(value, column, node->GetParent()->GetEntry(), node->GetEntryItemID());
		}
	}
}
bool KPCComponentsModel::SetValue(const wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column)
{
	if (KPCComponentsModelNode* node = GetNode(item))
	{
		bool itemChanged = false;
		if (KPPCStep* step = node->GetStep())
		{
			itemChanged = SetStepValue(value, column, step);
		}
		else if (KPPCGroup* group = node->GetGroup())
		{
			itemChanged = SetGroupValue(value, column, group);
		}
		else if (KPPCEntry* entry = node->GetEntry())
		{
			itemChanged = SetEntryValue(value, column, entry);
		}
		else if (node->IsEntryItem())
		{
			itemChanged = SetEntryItemValue(value, column, node->GetParent()->GetEntry(), node->GetEntryItemID());
		}

		if (itemChanged)
		{
			ChangeNotify();
		}
		return itemChanged;
	}
	return false;
}
bool KPCComponentsModel::GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
{
	if (column->GetID() == ColumnID::Name)
	{
		KPCComponentsModelNode* node = GetNode(item);
		if (node && node->IsEntryItem())
		{
			attributes.SetHeaderButtonBackgound(true);
			return true;
		}
	}
	return false;
}

void KPCComponentsModel::GetStepValue(wxAny& value, const KxDataViewColumn* column, const KPPCStep* step, bool editor) const
{
	switch (column->GetID())
	{
		case ColumnID::Name:
		{
			if (editor)
			{
				value = step->GetName();
			}
			else
			{
				value = KxDataViewBitmapTextValue(step->GetName(), KGetBitmap(KIMG_DIRECTION));
			}
			break;
		}
		case ColumnID::Value:
		{
			value = KTr("PackageCreator.PageComponents.Conditions") + ": " + KPackageCreatorPageComponents::FormatArrayToText(step->GetConditions(), true);
			break;
		}
	};
}
void KPCComponentsModel::GetGroupValue(wxAny& value, const KxDataViewColumn* column, const KPPCGroup* group, bool editor) const
{
	switch (column->GetID())
	{
		case ColumnID::Name:
		{
			if (editor)
			{
				value = group->GetName();
			}
			else
			{
				value = KxDataViewBitmapTextValue(group->GetName(), KGetBitmap(KIMG_FOLDER));
			}
			break;
		}
		case ColumnID::Value:
		{
			if (editor)
			{
				value = group->GetSelectionMode();
			}
			else
			{
				value = KTr("PackageCreator.PageComponents.SelectionMode") + ": " + m_SelectionModeEditor.GetItems()[group->GetSelectionMode()];
			}
			break;
		}
	};
}
void KPCComponentsModel::GetEntryValue(wxAny& value, const KxDataViewColumn* column, const KPPCEntry* entry, bool editor) const
{
	switch (column->GetID())
	{
		case ColumnID::Name:
		{
			if (editor)
			{
				value = entry->GetName();
			}
			else
			{
				value = KxDataViewBitmapTextValue(entry->GetName(), KGetBitmap(KIMG_BLOCK));
			}
			break;
		}
	};
}
void KPCComponentsModel::GetEntryItemValue(wxAny& value, const KxDataViewColumn* column, const KPPCEntry* entry, EntryID id, bool editor) const
{
	switch (id)
	{
		case EntryID::TypeDescriptor:
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = KTr("PackageCreator.PageComponents.TypeDescriptor");
					break;
				}
				case ColumnID::Value:
				{
					if (editor)
					{
						value = entry->GetTDDefaultValue();
					}
					else
					{
						value = m_TypeDescriptorEditor.GetItems()[entry->GetTDDefaultValue()];
					}
					break;
				}
			};
			break;
		}
		case EntryID::FileData:
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = KTr("PackageCreator.PageComponents.FileData");
					break;
				}
				case ColumnID::Value:
				{
					value = KPackageCreatorPageComponents::FormatArrayToText(entry->GetFileData());
					break;
				}
			};
			break;
		}
		case EntryID::Requirements:
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = KTr("PackageCreator.PageComponents.Requirements");
					break;
				}
				case ColumnID::Value:
				{
					value = KPackageCreatorPageComponents::FormatArrayToText(entry->GetRequirements());
					break;
				}
			};
			break;
		}
		case EntryID::Image:
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = KTr("PackageCreator.PageComponents.Image");
					break;
				}
				case ColumnID::Value:
				{
					if (editor)
					{
						const KPPIImageEntry* imageEntry = GetInterface().FindEntryWithValue(entry->GetImage());
						if (imageEntry)
						{
							const KPPIImageEntryArray& images = GetInterface().GetImages();
							auto it = std::find_if(images.begin(), images.end(), [imageEntry](const KPPIImageEntry& value)
							{
								return &value == imageEntry;
							});
							if (it != images.end())
							{
								// +1 for <None> image
								value = std::distance(images.begin(), it) + 1;
							}
						}
					}
					else
					{
						value = entry->GetImage().IsEmpty() ? KAux::MakeNoneLabel() : entry->GetImage();
					}
					break;
				}
			};
			break;
		}
		case EntryID::Description:
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = KTr("PackageCreator.PageComponents.Description");
					break;
				}
				case ColumnID::Value:
				{
					if (editor)
					{
						value = entry->GetDescription();
					}
					else if (entry->GetDescription().IsEmpty())
					{
						value = KAux::MakeNoneLabel();
					}
					else
					{
						value = entry->GetDescription();
					}
					break;
				}
			};
			break;
		}
		case EntryID::Conditions:
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = KTr("PackageCreator.PageComponents.Conditions");
					break;
				}
				case ColumnID::Value:
				{
					wxString sConditions = KPackageCreatorPageComponents::FormatArrayToText(entry->GetTDConditions(), true);
					if (entry->GetTDConditionalValue() != KPPC_DESCRIPTOR_INVALID)
					{
						value = sConditions + ' ' + KAux::GetUnicodeChar(KAUX_CHAR_ARROW_RIGHT) + ' ' + KPackageProjectComponents::TypeDescriptorToTranslation(entry->GetTDConditionalValue());
					}
					else
					{
						value = sConditions;
					}
					break;
				}
			};
			break;
		}
		case EntryID::AssignedFlags:
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = KTr("PackageCreator.PageComponents.AssignedFlags");
					break;
				}
				case ColumnID::Value:
				{
					value = KPackageCreatorPageComponents::FormatArrayToText(entry->GetAssignedFlags(), false);
					break;
				}
			};
			break;
		}
	};
}

bool KPCComponentsModel::SetStepValue(const wxAny& value, const KxDataViewColumn* column, KPPCStep* step)
{
	switch (column->GetID())
	{
		case ColumnID::Name:
		{
			step->SetName(value.As<wxString>());
			return true;
		}
	};
	return false;
}
bool KPCComponentsModel::SetGroupValue(const wxAny& value, const KxDataViewColumn* column, KPPCGroup* group)
{
	switch (column->GetID())
	{
		case ColumnID::Name:
		{
			group->SetName(value.As<wxString>());
			return true;
		}
		case ColumnID::Value:
		{
			group->SetSelectionMode(value.As<KPPCSelectionMode>());
			return true;
		}
	};
	return false;
}
bool KPCComponentsModel::SetEntryValue(const wxAny& value, const KxDataViewColumn* column, KPPCEntry* entry)
{
	switch (column->GetID())
	{
		case ColumnID::Name:
		{
			entry->SetName(value.As<wxString>());
			return true;
		}
	};
	return false;
}
bool KPCComponentsModel::SetEntryItemValue(const wxAny& value, const KxDataViewColumn* column, KPPCEntry* entry, EntryID id)
{
	if (column->GetID() == ColumnID::Value)
	{
		switch (id)
		{
			case EntryID::TypeDescriptor:
			{
				entry->SetTDDefaultValue(value.As<KPPCTypeDescriptor>());
				return true;
			}
			case EntryID::Image:
			{
				// Account for <None> image
				int index = value.As<int>() - 1;

				const KPPIImageEntryArray& images = GetInterface().GetImages();
				if (index >= 0 && (size_t)index < images.size())
				{
					const KPPIImageEntry& imageEntry = images[index];

					entry->SetImage(imageEntry.GetPath());
					m_EntryImageView->SetClientData(const_cast<KPPIImageEntry*>(&imageEntry));
					m_EntryImageView->SetBitmap(imageEntry.GetBitmap());
				}
				else
				{
					entry->SetImage(wxEmptyString);
					m_EntryImageView->SetClientData(NULL);
					m_EntryImageView->SetBitmap(wxNullBitmap);
				}

				ChangeNotify();
				return true;
			}
		};
	}
	return false;
}

void KPCComponentsModel::OnSelectItem(KxDataViewEvent& event)
{
	KPCComponentsModelNode* node = GetNode(event.GetItem());
	if (node && (node->GetEntry() || node->IsEntryItem()))
	{
		node = GetParentEntry(node);
		
		KPPIImageEntry* imageEntry = m_Controller->GetProject()->GetInterface().FindEntryWithValue(node->GetEntry()->GetImage());
		if (imageEntry)
		{
			if (!imageEntry->HasBitmap())
			{
				KPCIImagesListModel::LoadBitmap(imageEntry);
			}
			m_EntryImageView->SetClientData(imageEntry);
			m_EntryImageView->SetBitmap(imageEntry->GetBitmap());
			return;
		}
	}
	m_EntryImageView->SetClientData(NULL);
	m_EntryImageView->SetBitmap(wxNullBitmap);
}
void KPCComponentsModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewColumn* column = event.GetColumn();
	KxDataViewItem item = event.GetItem();

	KPCComponentsModelNode* node = GetNode(event.GetItem());
	if (node && column->GetID() == ColumnID::Value)
	{
		if (KPPCStep* step = node->GetStep())
		{
			KPCCFlagsSelectorModelDialog dialog(GetView(), column->GetTitle(), m_Controller, false);
			dialog.SetDataVector(step->GetConditions());
			dialog.ShowModal();
			NotifyChangedItem(item);
			return;
		}
		else if (node->IsEntryItem())
		{
			KPPCEntry* entry = node->GetParent()->GetEntry();
			switch (node->GetEntryItemID())
			{
				case EntryID::AssignedFlags:
				{
					if (entry)
					{
						KPCCFlagsSelectorModelDialog dialog(KMainWindow::GetInstance(), column->GetTitle(), m_Controller, true);
						dialog.SetDataVector(entry->GetAssignedFlags());
						dialog.ShowModal();
						NotifyChangedItem(item);
					}
					return;
				}
				case EntryID::Conditions:
				{
					if (entry)
					{
						KPCCFlagsTDSelectorModelDialog dialog(KMainWindow::GetInstance(), column->GetTitle(), m_Controller, entry);
						dialog.SetDataVector(entry->GetTDConditions());
						dialog.ShowModal();
						NotifyChangedItem(item);
					}
					return;
				}
				case EntryID::FileData:
				{
					if (entry)
					{
						KPCCFileDataSelectorModelDialog dialog(KMainWindow::GetInstance(), column->GetTitle(), m_Controller);
						dialog.SetDataVector(entry->GetFileData(), &m_Controller->GetProject()->GetFileData());
						if (dialog.ShowModal() == KxID_OK)
						{
							entry->GetFileData() = dialog.GetSelectedItems();
							NotifyChangedItem(event.GetItem());
						}
					}
					return;
				}
				case EntryID::Requirements:
				{
					if (entry)
					{
						KPCCRequirementsSelectorModelDialog dialog(KMainWindow::GetInstance(), column->GetTitle(), m_Controller);
						dialog.SetDataVector(entry->GetRequirements(), &m_Controller->GetProject()->GetRequirements());
						if (dialog.ShowModal() == KxID_OK)
						{
							entry->GetRequirements() = dialog.GetSelectedItems();
							NotifyChangedItem(item);
						}
					}
					return;
				}
				case EntryID::Description:
				{
					if (entry)
					{
						KTextEditorDialog dialog(KMainWindow::GetInstance());
						dialog.SetText(entry->GetDescription());
						if (dialog.ShowModal() == KxID_OK && dialog.IsModified())
						{
							entry->SetDescription(dialog.GetText());
							NotifyChangedItem(item);
						}
					}
					return;
				}
			};
		}
	}

	if (!GetView()->EditItem(event.GetItem(), event.GetColumn()))
	{
		event.Skip();
	}
}
void KPCComponentsModel::OnContextMenu(KxDataViewEvent& event)
{
	KxDataViewColumn* column = event.GetColumn();
	KxDataViewItem item = event.GetItem();
	KPCComponentsModelNode* node = GetNode(item);

	if (column)
	{
		KxMenu contextMenu;

		// All items
		{
			KxMenu* allItemsMenu = CreateAllItemsMenu();
			CreateAllItemsMenuEntry(allItemsMenu, node, KTr("Generic.Name"), &KPCComponentsModel::AllSteps_Name);
			CreateAllItemsMenuEntry(allItemsMenu, node, KTr("PackageCreator.PageComponents.Conditions"), &KPCComponentsModel::AllSteps_Conditions);

			KxMenuItem* item = contextMenu.Add(allItemsMenu, KTr("PackageCreator.PageComponents.AllSteps"));
			item->SetBitmap(KGetBitmap(KIMG_DIRECTION));
		}
		if (node)
		{
			KPCComponentsModelNode* parent = GetParentStep(node);
			if (parent && parent->GetStep())
			{
				KxMenu* allItemsMenu = CreateAllItemsMenu();
				CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("Generic.Name"), &KPCComponentsModel::AllGroups_Name);
				CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.SelectionMode"), &KPCComponentsModel::AllGroups_SelectionMode);

				KxMenuItem* item = contextMenu.Add(allItemsMenu, KTrf("PackageCreator.PageComponents.AllGroupsOf", parent->GetStep()->GetName()));
				item->SetBitmap(KGetBitmap(KIMG_FOLDER));
			}
		}
		if (node && (node->GetGroup() || node->GetEntry() || node->IsEntryItem()))
		{
			KPCComponentsModelNode* parent = GetParentGroup(node);
			if (parent && parent->GetGroup())
			{
				KxMenu* allItemsMenu = CreateAllItemsMenu();
				CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("Generic.Name"), &KPCComponentsModel::AllEntries_Name);
				CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.TypeDescriptor"), &KPCComponentsModel::AllEntries_DefaultTypeDescriptor);
				CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.FileData"), &KPCComponentsModel::AllEntries_FileData);
				CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.Requirements"), &KPCComponentsModel::AllEntries_Requirements);
				CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.Image"), &KPCComponentsModel::AllEntries_Image);
				CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.Description"), &KPCComponentsModel::AllEntries_Description);
				CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.Conditions"), &KPCComponentsModel::AllEntries_Conditions);
				CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.AssignedFlags"), &KPCComponentsModel::AllEntries_AssignedFlags);

				KxMenuItem* item = contextMenu.Add(allItemsMenu, KTrf("PackageCreator.PageComponents.AllEntriesOf", parent->GetGroup()->GetName()));
				item->SetBitmap(KGetBitmap(KIMG_BLOCK));
				item->SetClientData(node);
			}
		}
		contextMenu.AddSeparator();

		// Add item
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::AddStep, KTr("PackageCreator.PageComponents.AddStep")));
			item->SetBitmap(KGetBitmap(KIMG_DIRECTION_PLUS));
			item->Enable(true);
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::AddGroup, KTr("PackageCreator.PageComponents.AddGroup")));
			item->SetBitmap(KGetBitmap(KIMG_FOLDER_PLUS));
			item->Enable(node);
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::AddEntry, KTr("PackageCreator.PageComponents.AddEntry")));
			item->SetBitmap(KGetBitmap(KIMG_BLOCK_PLUS));
			item->Enable(node && (node->GetGroup() || node->GetEntry() || node->IsEntryItem()));
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::AddEntriesFromFiles, KTr("PackageCreator.PageComponents.AddEntriesFromFiles")));
			item->SetBitmap(KGetBitmap(KIMG_FOLDER_ARROW));
			item->Enable(node && (node->GetGroup() || node->GetEntry() || node->IsEntryItem()));
		}
		
		// Remove item
		KxMenuItem* menuItemRemove = NULL;
		if (node)
		{
			contextMenu.AddSeparator();
			menuItemRemove = contextMenu.Add(new KxMenuItem(KxID_REMOVE));
			menuItemRemove->Enable(false);

			if (const KPPCStep* step = node->GetStep())
			{
				menuItemRemove->SetBitmap(KGetBitmap(KIMG_DIRECTION_MINUS));
				menuItemRemove->SetItemLabel(wxString::Format("%s \"%s\"", KTr("PackageCreator.PageComponents.RemoveStep"), step->GetName()));
				menuItemRemove->SetClientData(node);
				menuItemRemove->Enable(true);
			}
			else if (const KPPCGroup* group = node->GetGroup())
			{
				menuItemRemove->SetBitmap(KGetBitmap(KIMG_FOLDER_MINUS));
				menuItemRemove->SetItemLabel(wxString::Format("%s \"%s\"", KTr("PackageCreator.PageComponents.RemoveGroup"), group->GetName()));
				menuItemRemove->SetClientData(node);
				menuItemRemove->Enable(true);
			}
			else if (node->GetEntry() || node->IsEntryItem())
			{
				const KPPCEntry* entry = node->GetEntry() ? node->GetEntry() : node->GetParent()->GetEntry();

				menuItemRemove->SetBitmap(KGetBitmap(KIMG_BLOCK_MINUS));
				menuItemRemove->SetItemLabel(wxString::Format("%s \"%s\"", KTr("PackageCreator.PageComponents.RemoveEntry"), entry->GetName()));
				menuItemRemove->SetClientData(node->GetEntry() ? node : node->GetParent());
				menuItemRemove->Enable(true);
			}
		}

		switch (int menuID = contextMenu.Show(GetView()))
		{
			case MenuID::AddStep:
			{
				AddStep(node, item);
				break;
			}
			case MenuID::AddGroup:
			{
				AddGroup(node, item);
				break;
			}
			case MenuID::AddEntry:
			{
				AddEntry(node, item);
				break;
			}
			case MenuID::AddEntriesFromFiles:
			{
				AddEntriesFromFiles(node, item);
				break;
			}

			case KxID_REMOVE:
			{
				KPCComponentsModelNode* node = static_cast<KPCComponentsModelNode*>(menuItemRemove->GetClientData());
				if (const KPPCStep* step = node->GetStep())
				{
					RemoveStep(node, step);
				}
				else if (const KPPCGroup* group = node->GetGroup())
				{
					RemoveGroup(node, group);
				}
				else if (const KPPCEntry* entry = node->GetEntry())
				{
					RemoveEntry(node, entry);
				}
				break;
			}
		};
	}
}

void KPCComponentsModel::OnAttachEditor(KxDataViewEvent& event)
{
	KxDataViewColumn* column = event.GetColumn();
	if (KPCComponentsModelNode* node = GetNode(event.GetItem()))
	{
		if (const KPPCStep* step = node->GetStep())
		{
			if (column->GetID() == ColumnID::Name)
			{
				column->SetEditor(&m_TextEditor);
			}
		}
		else if (const KPPCGroup* group = node->GetGroup())
		{
			if (column->GetID() == ColumnID::Name)
			{
				column->SetEditor(&m_TextEditor);
			}
			else
			{
				column->SetEditor(&m_SelectionModeEditor);
			}
		}
		else if (const KPPCEntry* entry = node->GetEntry())
		{
			if (column->GetID() == ColumnID::Name)
			{
				column->SetEditor(&m_TextEditor);
			}
		}
		else if (node->IsEntryItem())
		{
			switch (node->GetEntryItemID())
			{
				case EntryID::TypeDescriptor:
				{
					column->SetEditor(&m_TypeDescriptorEditor);
					break;
				}
				case EntryID::Image:
				{
					UpdateImageEditorList();
					column->SetEditor(&m_ImagesEditor);
					break;
				}
			};
		}
	}
}
void KPCComponentsModel::OnDetachEditor(KxDataViewEvent& event)
{
	event.GetColumn()->SetEditor(NULL);
}
void KPCComponentsModel::UpdateImageEditorList()
{
	KxStringVector imagesList;
	imagesList.reserve(GetInterface().GetImages().size() + 1);
	imagesList.push_back(KAux::MakeNoneLabel());

	for (auto& image: GetInterface().GetImages())
	{
		if (image.HasDescription())
		{
			imagesList.push_back(wxString::Format("%s - \"%s\"", image.GetPath(), image.GetDescription()));
		}
		else
		{
			imagesList.push_back(image.GetDescription());
		}
	}
	m_ImagesEditor.SetItems(imagesList);
}

bool KPCComponentsModel::OnDragItems(KxDataViewEventDND& event)
{
	KPCComponentsModelNode* node = GetNode(event.GetItem());
	if (node && !node->IsEntryItem())
	{
		SetDragDropDataObject(new KPackageCreatorListModelDataObject(event.GetItem()));
		return true;
	}
	return false;
}
bool KPCComponentsModel::OnDropItems(KxDataViewEventDND& event)
{
	KPCComponentsModelNode* draggedNode = GetNode(GetDragDropDataObject()->GetItem());
	KPCComponentsModelNode* thisNode = GetNode(event.GetItem());
	if (draggedNode && thisNode)
	{
		// All this moving shit below is extremely inefficient. I need to rework it someday.
		KxDataViewItem draggedItem = GetItem(draggedNode);

		// Nodes of same branch can be swapped more easily
		if (thisNode->IsSameBranch(draggedNode))
		{
			if (GetView()->GetMainWindow()->SwapTreeNodes(GetItem(thisNode), draggedItem))
			{
				// Nodes was swapped, it's safe to swap actual items. Node types are equal.
				if (const KPPCStep* step = thisNode->GetStep())
				{
					SwapNodesInSameBranch<KPPCStep>(thisNode, draggedNode, &GetComponents().GetSteps());
				}
				else if (const KPPCGroup* group = thisNode->GetGroup())
				{
					SwapNodesInSameBranch<KPPCGroup>(thisNode, draggedNode);
				}
				else if (const KPPCEntry* entry = thisNode->GetEntry())
				{
					SwapNodesInSameBranch<KPPCEntry>(thisNode, draggedNode);
				}

				GetView()->Select(draggedItem);
				GetView()->EnsureVisible(draggedItem);
				return true;
			}
		}
		else
		{
			// Step nodes will be handled in 'SameBranch'
			if (const KPPCGroup* group = thisNode->GetGroup())
			{
				MoveNodeToDifferentBranch<KPPCGroup>(thisNode, draggedNode, this);
			}
			else if (const KPPCEntry* group = thisNode->GetEntry())
			{
				MoveNodeToDifferentBranch<KPPCEntry>(thisNode, draggedNode, this);
			}

			GetView()->Select(draggedItem);
			GetView()->EnsureVisible(draggedItem);
			return true;
		}
	}
	return false;
}
bool KPCComponentsModel::OnDropItemsPossible(KxDataViewEventDND& event)
{
	KPCComponentsModelNode* draggedNode = GetNode(GetDragDropDataObject()->GetItem());
	KPCComponentsModelNode* thisNode = GetNode(event.GetItem());
	
	return (draggedNode && thisNode) && (draggedNode != thisNode) && thisNode->IsSameType(draggedNode);
}

void KPCComponentsModel::AddStep(KPCComponentsModelNode* node, const KxDataViewItem& item)
{
	GetView()->Expand(item);
	auto& step = GetComponents().GetSteps().emplace_back(new KPPCStep());
	auto& newNode = m_Steps.emplace_back(new KPCComponentsModelNode(step.get()));

	KxDataViewItem newItem = GetItem(newNode.get());
	ItemAdded(newItem);
	GetView()->Select(newItem);
	GetView()->EditItem(newItem, GetView()->GetColumnByID(ColumnID::Name));
	ChangeNotify();
}
void KPCComponentsModel::AddGroup(KPCComponentsModelNode* node, const KxDataViewItem& item)
{
	KPCComponentsModelNode* parent = GetParentStep(node);
	if (parent && parent->GetStep())
	{
		GetView()->Expand(item);
		auto& group = parent->GetStep()->GetGroups().emplace_back(new KPPCGroup());
		auto& newNode = parent->GetChildren().emplace_back(new KPCComponentsModelNode(group.get(), parent));

		KxDataViewItem newItem = GetItem(newNode.get());
		ItemAdded(GetItem(parent), newItem);
		GetView()->Select(newItem);
		GetView()->EditItem(newItem, GetView()->GetColumnByID(ColumnID::Name));
		ChangeNotify();
	}
}
void KPCComponentsModel::AddEntry(KPCComponentsModelNode* node, KxDataViewItem& item)
{
	KPCComponentsModelNode* parent = GetParentGroup(node);
	if (parent && parent->GetGroup())
	{
		GetView()->Expand(item);
		auto& entry = parent->GetGroup()->GetEntries().emplace_back(new KPPCEntry());
		auto& newNode = parent->GetChildren().emplace_back(new KPCComponentsModelNode(entry.get(), parent));
		newNode->CreateFullEntryNode();

		KxDataViewItem newItem = GetItem(newNode.get());
		ItemAdded(GetItem(parent), newItem);
		GetView()->Select(newItem);
		GetView()->Expand(newItem);
		GetView()->EditItem(newItem, GetView()->GetColumnByID(ColumnID::Name));
		ChangeNotify();
	}
}
void KPCComponentsModel::AddEntriesFromFiles(KPCComponentsModelNode* node, KxDataViewItem& item)
{
	KPCComponentsModelNode* parent = GetParentGroup(node);
	if (parent && parent->GetGroup())
	{
		for (const auto& fileEntry: m_Controller->GetProject()->GetFileData().GetData())
		{
			auto& entry = parent->GetGroup()->GetEntries().emplace_back(new KPPCEntry());
			auto& newNode = parent->GetChildren().emplace_back(new KPCComponentsModelNode(entry.get(), parent));
			newNode->CreateFullEntryNode();

			entry->SetName(fileEntry->GetID());
			entry->GetFileData().push_back(fileEntry->GetID());

			KxDataViewItem newItem = GetItem(newNode.get());
			ItemAdded(GetItem(parent), newItem);
		}

		GetView()->Expand(GetItem(parent));
		ChangeNotify();
	}
}
void KPCComponentsModel::RemoveStep(KPCComponentsModelNode* node, const KPPCStep* step)
{
	KxTaskDialog dialog(GetViewTLW(), KxID_NONE, KTrf("PackageCreator.RemoveStepDialog", step->GetName()), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
	if (dialog.ShowModal() == KxID_YES)
	{
		KPPCStepArray& steps = GetComponents().GetSteps();

		steps.erase(FindElement(steps, step));
		m_Steps.erase(FindElement(m_Steps, node));
		ItemDeleted(GetItem(node));
		ChangeNotify();
	}
}
void KPCComponentsModel::RemoveGroup(KPCComponentsModelNode* node, const KPPCGroup* group)
{
	KxTaskDialog dialog(GetViewTLW(), KxID_NONE, KTrf("PackageCreator.RemoveGroupDialog", group->GetName()), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
	if (dialog.ShowModal() == KxID_YES)
	{
		KPPCGroupArray& groups = node->GetParent()->GetStep()->GetGroups();
		KPCComponentsModelNode::Vector& nodes = node->GetParent()->GetChildren();

		groups.erase(FindElement(groups, group));
		nodes.erase(FindElement(nodes, node));
		ItemDeleted(GetItem(node->GetParent()), GetItem(node));
		ChangeNotify();
	}
}
void KPCComponentsModel::RemoveEntry(KPCComponentsModelNode* node, const KPPCEntry* entry)
{
	KPPCEntryArray& entries = node->GetParent()->GetGroup()->GetEntries();
	KPCComponentsModelNode::Vector& nodes = node->GetParent()->GetChildren();

	entries.erase(FindElement(entries, entry));
	nodes.erase(FindElement(nodes, node));
	ItemDeleted(GetItem(node->GetParent()), GetItem(node));
	ChangeNotify();
}

KxMenu* KPCComponentsModel::CreateAllItemsMenu()
{
	return new KxMenu();
}
void KPCComponentsModel::CreateAllItemsMenuEntry(KxMenu* menu, KPCComponentsModelNode* node, const wxString& name, AllItemsFunc func)
{
	KxMenuItem* item = menu->Add(new KxMenuItem(name));
	if (node)
	{
		item->Bind(KxEVT_MENU_SELECT, [this, func, node](KxMenuEvent& event)
		{
			(this->*func)(node, event.GetItem()->GetItemLabelText());
		});
	}
}

void KPCComponentsModel::AllSteps_Name(KPCComponentsModelNode* node, const wxString& name)
{
	KxTextBoxDialog dialog(GetViewTLW(), KxID_NONE, name);
	if (dialog.ShowModal() == KxID_OK)
	{
		for (auto& step: GetComponents().GetSteps())
		{
			step->SetName(dialog.GetValue());
		}
		GetView()->Refresh();
	}
}
void KPCComponentsModel::AllSteps_Conditions(KPCComponentsModelNode* node, const wxString& name)
{
	KPPCEntry tempEntry;
	KPCCFlagsSelectorModelDialog dialog(GetViewTLW(), name, m_Controller, false);
	dialog.SetDataVector(tempEntry.GetTDConditions());

	if (dialog.ShowModal() == KxID_OK)
	{
		for (auto& entry: GetComponents().GetSteps())
		{
			entry->GetConditions() = tempEntry.GetTDConditions();
		}
		GetView()->Refresh();
	}
}

void KPCComponentsModel::AllGroups_Name(KPCComponentsModelNode* node, const wxString& name)
{
	KPPCGroupArray& groups = node->GetStep()->GetGroups();

	KxTextBoxDialog dialog(GetViewTLW(), KxID_NONE, name);
	if (dialog.ShowModal() == KxID_OK)
	{
		for (auto& group: groups)
		{
			group->SetName(dialog.GetValue());
		}
		GetView()->Refresh();
	}
}
void KPCComponentsModel::AllGroups_SelectionMode(KPCComponentsModelNode* node, const wxString& name)
{
	KPPCGroupArray& groups = node->GetStep()->GetGroups();

	KxComboBoxDialog dialog(GetView(), KxID_NONE, name);
	dialog.SetItems(m_SelectionModeEditor.GetItems());
	if (dialog.ShowModal() == KxID_OK)
	{
		for (auto& entry: groups)
		{
			entry->SetSelectionMode((KPPCSelectionMode)dialog.GetSelection());
		}
		GetView()->Refresh();
	}
}

void KPCComponentsModel::AllEntries_Name(KPCComponentsModelNode* node, const wxString& name)
{
	KPPCEntryArray& entries = node->GetGroup()->GetEntries();

	KxTextBoxDialog dialog(GetView(), KxID_NONE, name);
	if (dialog.ShowModal() == KxID_OK)
	{
		for (auto& entry: entries)
		{
			entry->SetName(dialog.GetValue());
		}
		GetView()->Refresh();
	}
}
void KPCComponentsModel::AllEntries_DefaultTypeDescriptor(KPCComponentsModelNode* node, const wxString& name)
{
	KPPCEntryArray& entries = node->GetGroup()->GetEntries();

	KxComboBoxDialog dialog(GetView(), KxID_NONE, name);
	dialog.SetItems(m_TypeDescriptorEditor.GetItems());
	if (dialog.ShowModal() == KxID_OK)
	{
		for (auto& entry: entries)
		{
			entry->SetTDDefaultValue((KPPCTypeDescriptor)dialog.GetSelection());
		}
		GetView()->Refresh();
	}
}
void KPCComponentsModel::AllEntries_FileData(KPCComponentsModelNode* node, const wxString& name)
{
	KPPCEntryArray& entries = node->GetGroup()->GetEntries();

	KPCCFileDataSelectorModelDialog dialog(GetView(), name, m_Controller);
	dialog.SetDataVector({}, &m_Controller->GetProject()->GetFileData());
	if (dialog.ShowModal() == KxID_OK)
	{
		for (auto& entry: entries)
		{
			entry->GetFileData() = dialog.GetSelectedItems();
		}
		GetView()->Refresh();
	}
}
void KPCComponentsModel::AllEntries_Requirements(KPCComponentsModelNode* node, const wxString& name)
{
	KPPCEntryArray& entries = node->GetGroup()->GetEntries();

	KPCCRequirementsSelectorModelDialog dialog(GetView(), name, m_Controller);
	dialog.SetDataVector({}, &m_Controller->GetProject()->GetRequirements());
	if (dialog.ShowModal() == KxID_OK)
	{
		for (auto& entry: entries)
		{
			entry->GetRequirements() = dialog.GetSelectedItems();
		}
		GetView()->Refresh();
	}
}
void KPCComponentsModel::AllEntries_Image(KPCComponentsModelNode* node, const wxString& name)
{
	KPPCEntryArray& entries = node->GetGroup()->GetEntries();
	const KPPIImageEntryArray& images = GetInterface().GetImages();
	KxComboBoxDialog dialog(GetView(), KxID_NONE, name, wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL, KxComboBoxDialog::DefaultStyle);

	UpdateImageEditorList();
	dialog.SetItems(m_ImagesEditor.GetItems());
	if (dialog.ShowModal() == KxID_OK)
	{
		int index = dialog.GetSelection();
		if (index >= 0)
		{
			for (auto& entry: entries)
			{
				entry->SetImage(index != 0 ? images[index - 1].GetPath() : wxEmptyString);
			}
			GetView()->Refresh();
		}
		SelectItem(GetView()->GetSelection());
	}
}
void KPCComponentsModel::AllEntries_Conditions(KPCComponentsModelNode* node, const wxString& name)
{
	KPPCEntryArray& entries = node->GetGroup()->GetEntries();
	KPPCEntry tempEntry;

	KPCCFlagsTDSelectorModelDialog dialog(GetView(), name, m_Controller, &tempEntry);
	dialog.SetDataVector(tempEntry.GetTDConditions());
	if (dialog.ShowModal() == KxID_OK)
	{
		for (auto& entry: entries)
		{
			entry->GetTDConditions() = tempEntry.GetTDConditions();
			entry->SetTDConditionalValue(tempEntry.GetTDConditionalValue());
		}
		GetView()->Refresh();
	}
}
void KPCComponentsModel::AllEntries_AssignedFlags(KPCComponentsModelNode* node, const wxString& name)
{
	KPPCEntryArray& entries = node->GetGroup()->GetEntries();
	KPPCEntry tempEntry;

	KPCCFlagsSelectorModelDialog dialog(GetView(), name, m_Controller, true);
	dialog.SetDataVector(tempEntry.GetAssignedFlags());
	if (dialog.ShowModal() == KxID_OK)
	{
		for (auto& entry: entries)
		{
			entry->GetAssignedFlags() = tempEntry.GetAssignedFlags();
		}
		GetView()->Refresh();
	}
}
void KPCComponentsModel::AllEntries_Description(KPCComponentsModelNode* node, const wxString& name)
{
	KPPCEntryArray& entries = node->GetGroup()->GetEntries();

	KTextEditorDialog dialog(GetView());
	if (dialog.ShowModal() == KxID_OK)
	{
		for (auto& entry: entries)
		{
			entry->SetDescription(dialog.GetText());
		}
		GetView()->Refresh();
	}
}

KPackageProjectInterface& KPCComponentsModel::GetInterface() const
{
	return m_Controller->GetProject()->GetInterface();
}
KPackageProjectComponents& KPCComponentsModel::GetComponents() const
{
	return m_Controller->GetProject()->GetComponents();
}

KPCComponentsModel::KPCComponentsModel(KPackageCreatorController* controller)
	:m_Controller(controller)
{
	SetDataViewFlags(KxDV_DOUBLE_CLICK_EXPAND|KxDV_NO_TIMEOUT_EDIT|KxDV_VERT_RULES);

	// Type Descriptor
	{
		KxStringVector tChoices;
		auto AddMode = [&tChoices](KPPCTypeDescriptor type)
		{
			tChoices.push_back(KPackageProjectComponents::TypeDescriptorToTranslation(type));
		};
		AddMode(KPPC_DESCRIPTOR_OPTIONAL);
		AddMode(KPPC_DESCRIPTOR_REQUIRED);
		AddMode(KPPC_DESCRIPTOR_RECOMMENDED);
		AddMode(KPPC_DESCRIPTOR_COULD_BE_USABLE);
		AddMode(KPPC_DESCRIPTOR_NOT_USABLE);
		m_TypeDescriptorEditor.SetItems(tChoices);
		m_TypeDescriptorEditor.EndEditOnCloseup(true);
	}

	// Selection Mode
	{
		KxStringVector tChoices;
		auto AddMode = [&tChoices](KPPCSelectionMode mode)
		{
			tChoices.push_back(KPackageProjectComponents::SelectionModeToTranslation(mode));
		};
		AddMode(KPPC_SELECT_ANY);
		AddMode(KPPC_SELECT_EXACTLY_ONE);
		AddMode(KPPC_SELECT_AT_LEAST_ONE);
		AddMode(KPPC_SELECT_AT_MOST_ONE);
		AddMode(KPPC_SELECT_ALL);
		m_SelectionModeEditor.SetItems(tChoices);
		m_SelectionModeEditor.EndEditOnCloseup(true);
	}
	
	// Image
	{
		m_ImagesEditor.EndEditOnCloseup(true);
		m_ImagesEditor.SetMaxVisibleItems(16);
	}
}

void KPCComponentsModel::ChangeNotify()
{
	m_Controller->ChangeNotify();
}
void KPCComponentsModel::NotifyChangedItem(const KxDataViewItem& item)
{
	ItemChanged(item);
	ChangeNotify();
}

void KPCComponentsModel::RefreshItems()
{
	m_Steps.clear();
	for (const auto& step: GetComponents().GetSteps())
	{
		auto& stepNode = m_Steps.emplace_back(new KPCComponentsModelNode(step.get()));
		for (const auto& group: step->GetGroups())
		{
			auto& groupNode = stepNode->GetChildren().emplace_back(new KPCComponentsModelNode(group.get(), stepNode.get()));
			for (const auto& entry: group->GetEntries())
			{
				auto& entryNode = groupNode->GetChildren().emplace_back(new KPCComponentsModelNode(entry.get(), groupNode.get()));
				entryNode->CreateFullEntryNode();
			}
		}
	}
	ItemsCleared();
}
void KPCComponentsModel::SetProject(KPackageProject& project)
{
	RefreshItems();
}
