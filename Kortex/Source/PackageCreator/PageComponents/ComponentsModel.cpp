#include "stdafx.h"
#include "ComponentsModel.h"
#include "PackageCreator/PageBase.h"
#include "PackageCreator/WorkspaceDocument.h"
#include "PackageCreator/PageComponents.h"
#include "PackageCreator/PageInterface/ImageListModel.h"
#include "FileDataSelectorModel.h"
#include "RequirementsSelectorModel.h"
#include "AssignedConditionalsModel.h"
#include "ConditionGroupModel.h"
#include "UI/TextEditDialog.h"
#include "UI/ImageViewerDialog.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"
#include <KxFramework/KxTextBoxDialog.h>
#include <KxFramework/KxComboBoxDialog.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/DataView/KxDataViewMainWindow.h>

namespace Kortex::PackageDesigner::PageComponentsNS
{
	template<class ArrayT, class ValueT> auto FindElement(ArrayT& array, const ValueT& value)
	{
		return std::find_if(array.begin(), array.end(), [value](const auto& v)
		{
			return v.get() == value;
		});
	};
	
	template<class EntryT> void SwapNodesInSameBranch(ComponentsModelNode* thisNode, ComponentsModelNode* draggedNode, std::vector<std::unique_ptr<EntryT>>* entries = nullptr)
	{
		EntryT* thisEntry = nullptr;
		EntryT* draggedEntry = nullptr;

		if constexpr(std::is_same<EntryT, PackageProject::ComponentStep>::value)
		{
			thisEntry = thisNode->GetStep();
			draggedEntry = draggedNode->GetStep();
		}
		else if constexpr(std::is_same<EntryT, PackageProject::ComponentGroup>::value)
		{
			entries = &thisNode->GetParent()->GetStep()->GetGroups();

			thisEntry = thisNode->GetGroup();
			draggedEntry = draggedNode->GetGroup();
		}
		else if constexpr(std::is_same<EntryT, PackageProject::ComponentItem>::value)
		{
			entries = &thisNode->GetParent()->GetGroup()->GetItems();

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
	template<class EntryT> void MoveNodeToDifferentBranch(ComponentsModelNode* thisNode, ComponentsModelNode* draggedNode, ComponentsModel* model)
	{
		std::vector<std::unique_ptr<EntryT>>* thisItems = nullptr;
		std::vector<std::unique_ptr<EntryT>>* draggedItems = nullptr;
		
		EntryT* thisEntry = nullptr;
		EntryT* draggedEntry = nullptr;

		if constexpr(std::is_same<EntryT, PackageProject::ComponentGroup>::value)
		{
			thisItems = &thisNode->GetParent()->GetStep()->GetGroups();
			draggedItems = &draggedNode->GetParent()->GetStep()->GetGroups();

			thisEntry = thisNode->GetGroup();
			draggedEntry = draggedNode->GetGroup();
		}
		else if constexpr(std::is_same<EntryT, PackageProject::ComponentItem>::value)
		{
			thisItems = &thisNode->GetParent()->GetGroup()->GetItems();
			draggedItems = &draggedNode->GetParent()->GetGroup()->GetItems();

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
				ComponentsModelNode::Vector& thisNodes = thisNode->GetParent()->GetChildren();
				ComponentsModelNode::Vector& draggedNodes = draggedNode->GetParent()->GetChildren();

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

	ComponentsModelNode* GetSelectedAndExpanded(ComponentsModel* model, const ComponentsModelNode::Vector& nodes, KxDataViewCtrl* view, std::vector<ComponentsModelNode*>& expandedItems, ComponentsModelNode* except)
	{
		auto AddIfExpanded = [model, &expandedItems, view, except](ComponentsModelNode& node)
		{
			if (&node != except)
			{
				KxDataViewItem item = model->GetItem(&node);
				if (view->IsExpanded(item))
				{
					expandedItems.push_back(&node);
				}
			}
		};

		for (const auto& step: nodes)
		{
			AddIfExpanded(*step);
			for (const auto& group: step->GetChildren())
			{
				AddIfExpanded(*group);
				for (const auto& entry: group->GetChildren())
				{
					AddIfExpanded(*entry);
				}
			}
		}

		auto selectedNode = model->GetNode(view->GetSelection());
		return selectedNode != except ? selectedNode : nullptr;
	}
	void ExapndAndSelect(ComponentsModel* model, KxDataViewCtrl* view, const std::vector<ComponentsModelNode*>& expandedItems, const ComponentsModelNode* selectedItem)
	{
		for (const ComponentsModelNode* node: expandedItems)
		{
			view->Expand(model->GetItem(node));
		}
		view->Select(selectedItem ? model->GetItem(selectedItem) : KxDataViewItem());
	}

	ComponentsModelNode* GetParentStep(ComponentsModelNode* node)
	{
		ComponentsModelNode* parent = node;
		while (parent && !parent->GetStep())
		{
			parent = parent->GetParent();
		}
		return parent;
	}
	ComponentsModelNode* GetParentGroup(ComponentsModelNode* node)
	{
		ComponentsModelNode* parent = node;
		while (parent && !parent->GetGroup())
		{
			parent = parent->GetParent();
		}
		return parent;
	}
	ComponentsModelNode* GetParentEntry(ComponentsModelNode* node)
	{
		ComponentsModelNode* parent = node;
		while (parent && !parent->GetEntry())
		{
			parent = parent->GetParent();
		}
		return parent;
	}
}

namespace
{
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
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	void ComponentsModel::OnInitControl()
	{
		/* View */
		EnableDragAndDrop();
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &ComponentsModel::OnSelectItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &ComponentsModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &ComponentsModel::OnContextMenu, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_EDIT_ATTACH, &ComponentsModel::OnAttachEditor, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_EDIT_DETACH, &ComponentsModel::OnDetachEditor, this);
	
		/* Columns */
		GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 300);
		{
			auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Value"), ColumnID::Value, KxDATAVIEW_CELL_EDITABLE, 500);
			info.GetColumn()->UseDynamicEditor(true);
		}
	}
	
	bool ComponentsModel::IsContainer(const KxDataViewItem& item) const
	{
		if (item.IsTreeRootItem())
		{
			return true;
		}
		else if (ComponentsModelNode* node = GetNode(item))
		{
			return node->HasChildren();
		}
		return false;
	}
	void ComponentsModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
	{
		if (item.IsTreeRootItem())
		{
			for (const auto& childNode: m_Steps)
			{
				children.push_back(GetItem(childNode.get()));
			}
		}
		else if (ComponentsModelNode* node = GetNode(item))
		{
			for (const auto& childNode: node->GetChildren())
			{
				children.push_back(GetItem(childNode.get()));
			}
		}
	}
	KxDataViewItem ComponentsModel::GetParent(const KxDataViewItem& item) const
	{
		if (ComponentsModelNode* node = GetNode(item))
		{
			return GetItem(node->GetParent());
		}
		return KxDataViewItem();
	}
	bool ComponentsModel::HasContainerColumns(const KxDataViewItem& item) const
	{
		return true;
	}
	
	bool ComponentsModel::IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		return true;
	}
	bool ComponentsModel::IsEditorEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		if (ComponentsModelNode* node = GetNode(item))
		{
			if (const PackageProject::ComponentStep* step = node->GetStep())
			{
				return column->GetID() == ColumnID::Name;
			}
			else if (const PackageProject::ComponentGroup* group = node->GetGroup())
			{
				return true;
			}
			else if (const PackageProject::ComponentItem* entry = node->GetEntry())
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
	void ComponentsModel::GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		if (ComponentsModelNode* node = GetNode(item))
		{
			if (const PackageProject::ComponentStep* step = node->GetStep())
			{
				GetStepValue(value, column, step, true);
			}
			else if (const PackageProject::ComponentGroup* group = node->GetGroup())
			{
				GetGroupValue(value, column, group, true);
			}
			else if (const PackageProject::ComponentItem* entry = node->GetEntry())
			{
				GetEntryValue(value, column, entry, true);
			}
			else if (node->IsEntryItem())
			{
				GetEntryItemValue(value, column, node->GetParent()->GetEntry(), node->GetEntryItemID(), true);
			}
		}
	}
	void ComponentsModel::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		if (ComponentsModelNode* node = GetNode(item))
		{
			if (const PackageProject::ComponentStep* step = node->GetStep())
			{
				GetStepValue(value, column, step);
			}
			else if (const PackageProject::ComponentGroup* group = node->GetGroup())
			{
				GetGroupValue(value, column, group);
			}
			else if (const PackageProject::ComponentItem* entry = node->GetEntry())
			{
				GetEntryValue(value, column, entry);
			}
			else if (node->IsEntryItem())
			{
				GetEntryItemValue(value, column, node->GetParent()->GetEntry(), node->GetEntryItemID());
			}
		}
	}
	bool ComponentsModel::SetValue(const wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column)
	{
		if (ComponentsModelNode* node = GetNode(item))
		{
			bool itemChanged = false;
			if (PackageProject::ComponentStep* step = node->GetStep())
			{
				itemChanged = SetStepValue(value, column, step);
			}
			else if (PackageProject::ComponentGroup* group = node->GetGroup())
			{
				itemChanged = SetGroupValue(value, column, group);
			}
			else if (PackageProject::ComponentItem* entry = node->GetEntry())
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
	bool ComponentsModel::GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
	{
		if (column->GetID() == ColumnID::Name)
		{
			ComponentsModelNode* node = GetNode(item);
			if (node && node->IsEntryItem())
			{
				attributes.SetHeaderButtonBackgound(true);
				return true;
			}
		}
		return false;
	}
	
	void ComponentsModel::GetStepValue(wxAny& value, const KxDataViewColumn* column, const PackageProject::ComponentStep* step, bool editor) const
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
					value = KxDataViewBitmapTextValue(step->GetName(), ImageProvider::GetBitmap(ImageResourceID::Direction));
				}
				break;
			}
			case ColumnID::Value:
			{
				wxString conditions = PageComponents::ConditionGroupToString(step->GetConditionGroup());
				if (conditions.IsEmpty())
				{
					conditions = KAux::MakeNoneLabel();
				}
				value = KTr(wxS("PackageCreator.PageComponents.Conditions")) + wxS(": ") + conditions;
				break;
			}
		};
	}
	void ComponentsModel::GetGroupValue(wxAny& value, const KxDataViewColumn* column, const PackageProject::ComponentGroup* group, bool editor) const
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
					value = KxDataViewBitmapTextValue(group->GetName(), ImageProvider::GetBitmap(ImageResourceID::Folder));
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
					value = KTr(wxS("PackageCreator.PageComponents.SelectionMode")) + wxS(": ") + m_SelectionModeEditor.GetItems()[ToInt(group->GetSelectionMode())];
				}
				break;
			}
		};
	}
	void ComponentsModel::GetEntryValue(wxAny& value, const KxDataViewColumn* column, const PackageProject::ComponentItem* entry, bool editor) const
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
					value = KxDataViewBitmapTextValue(entry->GetName(), ImageProvider::GetBitmap(ImageResourceID::Block));
				}
				break;
			}
		};
	}
	void ComponentsModel::GetEntryItemValue(wxAny& value, const KxDataViewColumn* column, const PackageProject::ComponentItem* entry, EntryID id, bool editor) const
	{
		switch (id)
		{
			case EntryID::TypeDescriptor:
			{
				switch (column->GetID())
				{
					case ColumnID::Name:
					{
						value = KTr(wxS("PackageCreator.PageComponents.TypeDescriptor"));
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
							value = m_TypeDescriptorEditor.GetItems()[ToInt(entry->GetTDDefaultValue())];
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
						value = KTr(wxS("PackageCreator.PageComponents.FileData"));
						break;
					}
					case ColumnID::Value:
					{
						value = PageComponents::FormatArrayToText(entry->GetFileData());
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
						value = KTr(wxS("PackageCreator.PageComponents.Requirements"));
						break;
					}
					case ColumnID::Value:
					{
						value = PageComponents::FormatArrayToText(entry->GetRequirements());
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
						value = KTr(wxS("PackageCreator.PageComponents.Image"));
						break;
					}
					case ColumnID::Value:
					{
						if (editor)
						{
							const PackageProject::ImageItem* imageEntry = GetInterface().FindImageByPath(entry->GetImage());
							if (imageEntry)
							{
								const PackageProject::ImageItem::Vector& images = GetInterface().GetImages();
								auto it = std::find_if(images.begin(), images.end(), [imageEntry](const PackageProject::ImageItem& value)
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
						value = KTr(wxS("PackageCreator.PageComponents.Description"));
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
						value = KTr(wxS("PackageCreator.PageComponents.Conditions"));
						break;
					}
					case ColumnID::Value:
					{
						wxString conditions = PageComponents::ConditionGroupToString(entry->GetTDConditionGroup());
						if (conditions.IsEmpty())
						{
							conditions = KAux::MakeNoneLabel();
						}
	
						if (entry->GetTDConditionalValue() != PackageProject::TypeDescriptor::Invalid)
						{
							if (entry->GetTDConditionGroup().GetConditions().size() > 1)
							{
								conditions.Prepend(wxS("("));
								conditions.Append(wxS(")"));
							}
							value = conditions + wxS(' ') + KAux::GetUnicodeChar(KAUX_CHAR_ARROW_RIGHT) + wxS(' ') + PackageProject::ComponentsSection::TypeDescriptorToTranslation(entry->GetTDConditionalValue());
						}
						else
						{
							value = conditions;
						}
						break;
					}
				};
				break;
			}
			case EntryID::ConditionFlags:
			{
				switch (column->GetID())
				{
					case ColumnID::Name:
					{
						value = KTr(wxS("PackageCreator.PageComponents.ConditionFlags"));
						break;
					}
					case ColumnID::Value:
					{
						wxString flags = PageComponents::ConditionToString(entry->GetConditionalFlags(), false);
						if (flags.IsEmpty())
						{
							flags = KAux::MakeNoneLabel();
						}
						value = flags;
						break;
					}
				};
				break;
			}
		};
	}
	
	bool ComponentsModel::SetStepValue(const wxAny& value, const KxDataViewColumn* column, PackageProject::ComponentStep* step)
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
	bool ComponentsModel::SetGroupValue(const wxAny& value, const KxDataViewColumn* column, PackageProject::ComponentGroup* group)
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
				group->SetSelectionMode(value.As<PackageProject::SelectionMode>());
				return true;
			}
		};
		return false;
	}
	bool ComponentsModel::SetEntryValue(const wxAny& value, const KxDataViewColumn* column, PackageProject::ComponentItem* entry)
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
	bool ComponentsModel::SetEntryItemValue(const wxAny& value, const KxDataViewColumn* column, PackageProject::ComponentItem* entry, EntryID id)
	{
		if (column->GetID() == ColumnID::Value)
		{
			switch (id)
			{
				case EntryID::TypeDescriptor:
				{
					entry->SetTDDefaultValue(value.As<PackageProject::TypeDescriptor>());
					return true;
				}
				case EntryID::Image:
				{
					// Account for <None> image
					int index = value.As<int>() - 1;
	
					const PackageProject::ImageItem::Vector& images = GetInterface().GetImages();
					if (index >= 0 && (size_t)index < images.size())
					{
						const PackageProject::ImageItem& imageEntry = images[index];
	
						entry->SetImage(imageEntry.GetPath());
						m_EntryImageView->SetClientData(const_cast<PackageProject::ImageItem*>(&imageEntry));
						m_EntryImageView->SetBitmap(imageEntry.GetBitmap());
					}
					else
					{
						entry->SetImage(wxEmptyString);
						m_EntryImageView->SetClientData(nullptr);
						m_EntryImageView->SetBitmap(wxNullBitmap);
					}
	
					ChangeNotify();
					return true;
				}
			};
		}
		return false;
	}
	
	void ComponentsModel::OnSelectItem(KxDataViewEvent& event)
	{
		ComponentsModelNode* node = GetNode(event.GetItem());
		if (node && (node->GetEntry() || node->IsEntryItem()))
		{
			node = GetParentEntry(node);
			
			PackageProject::ImageItem* imageEntry = m_Controller->GetProject()->GetInterface().FindImageByPath(node->GetEntry()->GetImage());
			if (imageEntry)
			{
				if (!imageEntry->HasBitmap())
				{
					PageInterfaceNS::ImageListModel::LoadBitmap(imageEntry, GetView());
				}
				m_EntryImageView->SetClientData(imageEntry);
				m_EntryImageView->SetBitmap(imageEntry->GetBitmap());
				return;
			}
		}
		m_EntryImageView->SetClientData(nullptr);
		m_EntryImageView->SetBitmap(wxNullBitmap);
	}
	void ComponentsModel::OnActivateItem(KxDataViewEvent& event)
	{
		KxDataViewColumn* column = event.GetColumn();
		KxDataViewItem item = event.GetItem();
	
		ComponentsModelNode* node = GetNode(event.GetItem());
		if (node && column->GetID() == ColumnID::Value)
		{
			if (PackageProject::ComponentStep* step = node->GetStep())
			{
				ConditionGroupDialog dialog(GetView(), column->GetTitle(), m_Controller, step->GetConditionGroup());
				dialog.ShowModal();
				NotifyChangedItem(item);
				return;
			}
			else if (node->IsEntryItem())
			{
				auto GetItemLabel = [this, &item]()
				{
					wxAny value;
					GetValue(value, item, GetView()->GetColumnByID(ColumnID::Name));
					return value.As<wxString>();
				};
	
				PackageProject::ComponentItem* entry = node->GetParent()->GetEntry();
				switch (node->GetEntryItemID())
				{
					case EntryID::ConditionFlags:
					{
						if (entry)
						{
							AssignedConditionalsDialog dialog(GetView(), GetItemLabel(), m_Controller);
							dialog.SetDataVector(entry->GetConditionalFlags());
							dialog.ShowModal();
							NotifyChangedItem(item);
						}
						return;
					}
					case EntryID::Conditions:
					{
						if (entry)
						{
							ConditionGroupDialogWithTypeDescriptor dialog(GetView(), GetItemLabel(), m_Controller, entry->GetTDConditionGroup(), *entry);
							dialog.ShowModal();
							NotifyChangedItem(item);
						}
						return;
					}
					case EntryID::FileData:
					{
						if (entry)
						{
							FileDataSelectorDialog dialog(GetView(), GetItemLabel(), m_Controller);
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
							RequirementsSelectorDialog dialog(GetView(), GetItemLabel(), m_Controller);
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
							UI::TextEditDialog dialog(GetView());
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
	void ComponentsModel::OnContextMenu(KxDataViewEvent& event)
	{
		KxDataViewColumn* column = event.GetColumn();
		KxDataViewItem item = event.GetItem();
		ComponentsModelNode* node = GetNode(item);
	
		if (column)
		{
			KxMenu contextMenu;
	
			// All items
			{
				KxMenu* allItemsMenu = CreateAllItemsMenu();
				CreateAllItemsMenuEntry(allItemsMenu, node, KTr("Generic.Name"), &ComponentsModel::AllSteps_Name);
				CreateAllItemsMenuEntry(allItemsMenu, node, KTr("PackageCreator.PageComponents.Conditions"), &ComponentsModel::AllSteps_Conditions);
	
				KxMenuItem* item = contextMenu.Add(allItemsMenu, KTr("PackageCreator.PageComponents.AllSteps"));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Direction));
			}
			if (node)
			{
				ComponentsModelNode* parent = GetParentStep(node);
				if (parent && parent->GetStep())
				{
					KxMenu* allItemsMenu = CreateAllItemsMenu();
					CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("Generic.Name"), &ComponentsModel::AllGroups_Name);
					CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.SelectionMode"), &ComponentsModel::AllGroups_SelectionMode);
	
					KxMenuItem* item = contextMenu.Add(allItemsMenu, KTrf("PackageCreator.PageComponents.AllGroupsOf", parent->GetStep()->GetName()));
					item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Folder));
				}
			}
			if (node && (node->GetGroup() || node->GetEntry() || node->IsEntryItem()))
			{
				ComponentsModelNode* parent = GetParentGroup(node);
				if (parent && parent->GetGroup())
				{
					KxMenu* allItemsMenu = CreateAllItemsMenu();
					CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("Generic.Name"), &ComponentsModel::AllEntries_Name);
					CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.TypeDescriptor"), &ComponentsModel::AllEntries_DefaultTypeDescriptor);
					CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.FileData"), &ComponentsModel::AllEntries_FileData);
					CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.Requirements"), &ComponentsModel::AllEntries_Requirements);
					CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.Image"), &ComponentsModel::AllEntries_Image);
					CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.Description"), &ComponentsModel::AllEntries_Description);
					CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.Conditions"), &ComponentsModel::AllEntries_Conditions);
					CreateAllItemsMenuEntry(allItemsMenu, parent, KTr("PackageCreator.PageComponents.AssignedFlags"), &ComponentsModel::AllEntries_AssignedFlags);
	
					KxMenuItem* item = contextMenu.Add(allItemsMenu, KTrf("PackageCreator.PageComponents.AllEntriesOf", parent->GetGroup()->GetName()));
					item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Block));
					item->SetClientData(node);
				}
			}
			contextMenu.AddSeparator();
	
			// Add item
			{
				KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::AddStep, KTr("PackageCreator.PageComponents.AddStep")));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::DirectionPlus));
				item->Enable(true);
			}
			{
				KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::AddGroup, KTr("PackageCreator.PageComponents.AddGroup")));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderPlus));
				item->Enable(node);
			}
			{
				KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::AddEntry, KTr("PackageCreator.PageComponents.AddEntry")));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::BlockPlus));
				item->Enable(node && (node->GetGroup() || node->GetEntry() || node->IsEntryItem()));
			}
			{
				KxMenuItem* item = contextMenu.Add(new KxMenuItem(MenuID::AddEntriesFromFiles, KTr("PackageCreator.PageComponents.AddEntriesFromFiles")));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FoldersPlus));
				item->Enable(node && (node->GetGroup() || node->GetEntry() || node->IsEntryItem()));
			}
	
			// Remove item
			KxMenuItem* menuItemRemove = nullptr;
			if (node)
			{
				contextMenu.AddSeparator();
				menuItemRemove = contextMenu.Add(new KxMenuItem(KxID_REMOVE));
				menuItemRemove->Enable(false);
	
				if (const PackageProject::ComponentStep* step = node->GetStep())
				{
					menuItemRemove->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::DirectionMinus));
					menuItemRemove->SetItemLabel(wxString::Format("%s \"%s\"", KTr("PackageCreator.PageComponents.RemoveStep"), step->GetName()));
					menuItemRemove->SetClientData(node);
					menuItemRemove->Enable(true);
				}
				else if (const PackageProject::ComponentGroup* group = node->GetGroup())
				{
					menuItemRemove->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderMinus));
					menuItemRemove->SetItemLabel(wxString::Format("%s \"%s\"", KTr("PackageCreator.PageComponents.RemoveGroup"), group->GetName()));
					menuItemRemove->SetClientData(node);
					menuItemRemove->Enable(true);
				}
				else if (node->GetEntry() || node->IsEntryItem())
				{
					const PackageProject::ComponentItem* entry = node->GetEntry() ? node->GetEntry() : node->GetParent()->GetEntry();
	
					menuItemRemove->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::BlockMinus));
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
					ComponentsModelNode* node = static_cast<ComponentsModelNode*>(menuItemRemove->GetClientData());
					if (const PackageProject::ComponentStep* step = node->GetStep())
					{
						RemoveStep(node, step);
					}
					else if (const PackageProject::ComponentGroup* group = node->GetGroup())
					{
						RemoveGroup(node, group);
					}
					else if (const PackageProject::ComponentItem* entry = node->GetEntry())
					{
						RemoveEntry(node, entry);
					}
					break;
				}
			};
		}
	}
	
	void ComponentsModel::OnAttachEditor(KxDataViewEvent& event)
	{
		KxDataViewColumn* column = event.GetColumn();
		if (ComponentsModelNode* node = GetNode(event.GetItem()))
		{
			if (const PackageProject::ComponentStep* step = node->GetStep())
			{
				if (column->GetID() == ColumnID::Name)
				{
					column->SetEditor(&m_TextEditor);
				}
			}
			else if (const PackageProject::ComponentGroup* group = node->GetGroup())
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
			else if (const PackageProject::ComponentItem* entry = node->GetEntry())
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
	void ComponentsModel::OnDetachEditor(KxDataViewEvent& event)
	{
		event.GetColumn()->SetEditor(nullptr);
	}
	void ComponentsModel::UpdateImageEditorList()
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
	
	bool ComponentsModel::OnDragItems(KxDataViewEventDND& event)
	{
		ComponentsModelNode* node = GetNode(event.GetItem());
		if (node && !node->IsEntryItem())
		{
			SetDragDropDataObject(new ListModelDataObject(event.GetItem()));
			return true;
		}
		return false;
	}
	bool ComponentsModel::OnDropItems(KxDataViewEventDND& event)
	{
		ComponentsModelNode* draggedNode = GetNode(GetDragDropDataObject()->GetItem());
		ComponentsModelNode* thisNode = GetNode(event.GetItem());
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
					if (const PackageProject::ComponentStep* step = thisNode->GetStep())
					{
						SwapNodesInSameBranch<PackageProject::ComponentStep>(thisNode, draggedNode, &GetComponents().GetSteps());
					}
					else if (const PackageProject::ComponentGroup* group = thisNode->GetGroup())
					{
						SwapNodesInSameBranch<PackageProject::ComponentGroup>(thisNode, draggedNode);
					}
					else if (const PackageProject::ComponentItem* entry = thisNode->GetEntry())
					{
						SwapNodesInSameBranch<PackageProject::ComponentItem>(thisNode, draggedNode);
					}
	
					GetView()->Select(draggedItem);
					GetView()->EnsureVisible(draggedItem);
					return true;
				}
			}
			else
			{
				// Step nodes will be handled in 'SameBranch'
				if (const PackageProject::ComponentGroup* group = thisNode->GetGroup())
				{
					MoveNodeToDifferentBranch<PackageProject::ComponentGroup>(thisNode, draggedNode, this);
				}
				else if (const PackageProject::ComponentItem* group = thisNode->GetEntry())
				{
					MoveNodeToDifferentBranch<PackageProject::ComponentItem>(thisNode, draggedNode, this);
				}
	
				GetView()->Select(draggedItem);
				GetView()->EnsureVisible(draggedItem);
				return true;
			}
		}
		return false;
	}
	bool ComponentsModel::OnDropItemsPossible(KxDataViewEventDND& event)
	{
		ComponentsModelNode* draggedNode = GetNode(GetDragDropDataObject()->GetItem());
		ComponentsModelNode* thisNode = GetNode(event.GetItem());
		
		return (draggedNode && thisNode) && (draggedNode != thisNode) && thisNode->IsSameType(draggedNode);
	}
	
	void ComponentsModel::AddStep(ComponentsModelNode* node, const KxDataViewItem& item)
	{
		GetView()->Expand(item);
		auto& step = GetComponents().GetSteps().emplace_back(std::make_unique<PackageProject::ComponentStep>());
		auto& newNode = m_Steps.emplace_back(std::make_unique<ComponentsModelNode>(step.get()));
	
		KxDataViewItem newItem = GetItem(newNode.get());
		ItemAdded(newItem);
		GetView()->Select(newItem);
		GetView()->EditItem(newItem, GetView()->GetColumnByID(ColumnID::Name));
		ChangeNotify();
	}
	void ComponentsModel::AddGroup(ComponentsModelNode* node, const KxDataViewItem& item)
	{
		ComponentsModelNode* parent = GetParentStep(node);
		if (parent && parent->GetStep())
		{
			GetView()->Expand(item);
			auto& group = parent->GetStep()->GetGroups().emplace_back(std::make_unique<PackageProject::ComponentGroup>());
			auto& newNode = parent->GetChildren().emplace_back(std::make_unique<ComponentsModelNode>(group.get(), parent));
	
			KxDataViewItem newItem = GetItem(newNode.get());
			ItemAdded(GetItem(parent), newItem);
			GetView()->Select(newItem);
			GetView()->EditItem(newItem, GetView()->GetColumnByID(ColumnID::Name));
			ChangeNotify();
		}
	}
	void ComponentsModel::AddEntry(ComponentsModelNode* node, KxDataViewItem& item)
	{
		ComponentsModelNode* parent = GetParentGroup(node);
		if (parent && parent->GetGroup())
		{
			GetView()->Expand(item);
			auto& entry = parent->GetGroup()->GetItems().emplace_back(std::make_unique<PackageProject::ComponentItem>());
			auto& newNode = parent->GetChildren().emplace_back(std::make_unique<ComponentsModelNode>(entry.get(), parent));
			newNode->CreateFullEntryNode();
	
			KxDataViewItem newItem = GetItem(newNode.get());
			ItemAdded(GetItem(parent), newItem);
			GetView()->Select(newItem);
			GetView()->Expand(newItem);
			GetView()->EditItem(newItem, GetView()->GetColumnByID(ColumnID::Name));
			ChangeNotify();
		}
	}
	void ComponentsModel::AddEntriesFromFiles(ComponentsModelNode* node, KxDataViewItem& item)
	{
		ComponentsModelNode* parent = GetParentGroup(node);
		if (parent && parent->GetGroup())
		{
			for (const auto& fileEntry: m_Controller->GetProject()->GetFileData().GetData())
			{
				auto& entry = parent->GetGroup()->GetItems().emplace_back(std::make_unique<PackageProject::ComponentItem>());
				auto& newNode = parent->GetChildren().emplace_back(std::make_unique<ComponentsModelNode>(entry.get(), parent));
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
	void ComponentsModel::RemoveStep(ComponentsModelNode* node, const PackageProject::ComponentStep* step)
	{
		KxTaskDialog dialog(GetViewTLW(), KxID_NONE, KTrf("PackageCreator.RemoveStepDialog", step->GetName()), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
		if (dialog.ShowModal() == KxID_YES)
		{
			PackageProject::ComponentStep::Vector& steps = GetComponents().GetSteps();
	
			steps.erase(FindElement(steps, step));
			m_Steps.erase(FindElement(m_Steps, node));
	
			std::vector<ComponentsModelNode*> expandedItems;
			ComponentsModelNode* selectedItem = GetSelectedAndExpanded(this, m_Steps, GetView(), expandedItems, node);
	
			ItemsCleared();
			ChangeNotify();
			ExapndAndSelect(this, GetView(), expandedItems, selectedItem);
		}
	}
	void ComponentsModel::RemoveGroup(ComponentsModelNode* node, const PackageProject::ComponentGroup* group)
	{
		KxTaskDialog dialog(GetViewTLW(), KxID_NONE, KTrf("PackageCreator.RemoveGroupDialog", group->GetName()), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
		if (dialog.ShowModal() == KxID_YES)
		{
			PackageProject::ComponentGroup::Vector& groups = node->GetParent()->GetStep()->GetGroups();
			ComponentsModelNode::Vector& nodes = node->GetParent()->GetChildren();
	
			groups.erase(FindElement(groups, group));
			nodes.erase(FindElement(nodes, node));
	
			std::vector<ComponentsModelNode*> expandedItems;
			ComponentsModelNode* selectedItem = GetSelectedAndExpanded(this, m_Steps, GetView(), expandedItems, node);
	
			ItemsCleared();
			ChangeNotify();
			ExapndAndSelect(this, GetView(), expandedItems, selectedItem);
		}
	}
	void ComponentsModel::RemoveEntry(ComponentsModelNode* node, const PackageProject::ComponentItem* entry)
	{
		PackageProject::ComponentItem::Vector& entries = node->GetParent()->GetGroup()->GetItems();
		ComponentsModelNode::Vector& nodes = node->GetParent()->GetChildren();
	
		entries.erase(FindElement(entries, entry));
		nodes.erase(FindElement(nodes, node));
	
		std::vector<ComponentsModelNode*> expandedItems;
		ComponentsModelNode* selectedItem = GetSelectedAndExpanded(this, m_Steps, GetView(), expandedItems, node);
	
		ItemsCleared();
		ChangeNotify();
		ExapndAndSelect(this, GetView(), expandedItems, selectedItem);
	}
	
	KxMenu* ComponentsModel::CreateAllItemsMenu()
	{
		return new KxMenu();
	}
	void ComponentsModel::CreateAllItemsMenuEntry(KxMenu* menu, ComponentsModelNode* node, const wxString& name, AllItemsFunc func)
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
	
	void ComponentsModel::AllSteps_Name(ComponentsModelNode* node, const wxString& name)
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
	void ComponentsModel::AllSteps_Conditions(ComponentsModelNode* node, const wxString& name)
	{
		PackageProject::ComponentItem tempEntry;
		ConditionGroupDialog dialog(GetViewTLW(), name, m_Controller, tempEntry.GetTDConditionGroup());
		if (dialog.ShowModal() == KxID_OK)
		{
			for (auto& entry: GetComponents().GetSteps())
			{
				entry->GetConditionGroup() = tempEntry.GetTDConditionGroup();
			}
			GetView()->Refresh();
		}
	}
	
	void ComponentsModel::AllGroups_Name(ComponentsModelNode* node, const wxString& name)
	{
		PackageProject::ComponentGroup::Vector& groups = node->GetStep()->GetGroups();
	
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
	void ComponentsModel::AllGroups_SelectionMode(ComponentsModelNode* node, const wxString& name)
	{
		PackageProject::ComponentGroup::Vector& groups = node->GetStep()->GetGroups();
	
		KxComboBoxDialog dialog(GetView(), KxID_NONE, name);
		dialog.SetItems(m_SelectionModeEditor.GetItems());
		if (dialog.ShowModal() == KxID_OK)
		{
			for (auto& entry: groups)
			{
				entry->SetSelectionMode((PackageProject::SelectionMode)dialog.GetSelection());
			}
			GetView()->Refresh();
		}
	}
	
	void ComponentsModel::AllEntries_Name(ComponentsModelNode* node, const wxString& name)
	{
		PackageProject::ComponentItem::Vector& entries = node->GetGroup()->GetItems();
	
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
	void ComponentsModel::AllEntries_DefaultTypeDescriptor(ComponentsModelNode* node, const wxString& name)
	{
		PackageProject::ComponentItem::Vector& entries = node->GetGroup()->GetItems();
	
		KxComboBoxDialog dialog(GetView(), KxID_NONE, name);
		dialog.SetItems(m_TypeDescriptorEditor.GetItems());
		if (dialog.ShowModal() == KxID_OK)
		{
			for (auto& entry: entries)
			{
				entry->SetTDDefaultValue((PackageProject::TypeDescriptor)dialog.GetSelection());
			}
			GetView()->Refresh();
		}
	}
	void ComponentsModel::AllEntries_FileData(ComponentsModelNode* node, const wxString& name)
	{
		PackageProject::ComponentItem::Vector& entries = node->GetGroup()->GetItems();
	
		FileDataSelectorDialog dialog(GetView(), name, m_Controller);
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
	void ComponentsModel::AllEntries_Requirements(ComponentsModelNode* node, const wxString& name)
	{
		PackageProject::ComponentItem::Vector& entries = node->GetGroup()->GetItems();
	
		RequirementsSelectorDialog dialog(GetView(), name, m_Controller);
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
	void ComponentsModel::AllEntries_Image(ComponentsModelNode* node, const wxString& name)
	{
		PackageProject::ComponentItem::Vector& entries = node->GetGroup()->GetItems();
		const PackageProject::ImageItem::Vector& images = GetInterface().GetImages();
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
	void ComponentsModel::AllEntries_Conditions(ComponentsModelNode* node, const wxString& name)
	{
		PackageProject::ComponentItem::Vector& entries = node->GetGroup()->GetItems();
		PackageProject::ComponentItem tempEntry;
	
		ConditionGroupDialogWithTypeDescriptor dialog(GetView(), name, m_Controller, tempEntry.GetTDConditionGroup(), tempEntry);
		if (dialog.ShowModal() == KxID_OK)
		{
			for (auto& entry: entries)
			{
				entry->GetTDConditionGroup() = tempEntry.GetTDConditionGroup();
				entry->SetTDConditionalValue(tempEntry.GetTDConditionalValue());
			}
			GetView()->Refresh();
		}
	}
	void ComponentsModel::AllEntries_AssignedFlags(ComponentsModelNode* node, const wxString& name)
	{
		PackageProject::ComponentItem::Vector& entries = node->GetGroup()->GetItems();
		PackageProject::ComponentItem tempEntry;
	
		AssignedConditionalsDialog dialog(GetView(), name, m_Controller);
		dialog.SetDataVector(tempEntry.GetConditionalFlags());
		if (dialog.ShowModal() == KxID_OK)
		{
			for (auto& entry: entries)
			{
				entry->GetConditionalFlags() = tempEntry.GetConditionalFlags();
			}
			GetView()->Refresh();
		}
	}
	void ComponentsModel::AllEntries_Description(ComponentsModelNode* node, const wxString& name)
	{
		PackageProject::ComponentItem::Vector& entries = node->GetGroup()->GetItems();
	
		UI::TextEditDialog dialog(GetView());
		if (dialog.ShowModal() == KxID_OK)
		{
			for (auto& entry: entries)
			{
				entry->SetDescription(dialog.GetText());
			}
			GetView()->Refresh();
		}
	}
	
	PackageProject::InterfaceSection& ComponentsModel::GetInterface() const
	{
		return m_Controller->GetProject()->GetInterface();
	}
	PackageProject::ComponentsSection& ComponentsModel::GetComponents() const
	{
		return m_Controller->GetProject()->GetComponents();
	}
	
	ComponentsModel::ComponentsModel(WorkspaceDocument* controller)
		:m_Controller(controller)
	{
		SetDataViewFlags(KxDV_DOUBLE_CLICK_EXPAND|KxDV_NO_TIMEOUT_EDIT|KxDV_VERT_RULES);
	
		// Type Descriptor
		{
			KxStringVector tChoices;
			auto AddMode = [&tChoices](PackageProject::TypeDescriptor type)
			{
				tChoices.push_back(PackageProject::ComponentsSection::TypeDescriptorToTranslation(type));
			};
			AddMode(PackageProject::TypeDescriptor::Optional);
			AddMode(PackageProject::TypeDescriptor::Required);
			AddMode(PackageProject::TypeDescriptor::Recommended);
			AddMode(PackageProject::TypeDescriptor::CouldBeUsable);
			AddMode(PackageProject::TypeDescriptor::NotUsable);
			m_TypeDescriptorEditor.SetItems(tChoices);
			m_TypeDescriptorEditor.EndEditOnCloseup(true);
		}
	
		// Selection Mode
		{
			KxStringVector tChoices;
			auto AddMode = [&tChoices](PackageProject::SelectionMode mode)
			{
				tChoices.push_back(PackageProject::ComponentsSection::SelectionModeToTranslation(mode));
			};
			AddMode(PackageProject::SelectionMode::Any);
			AddMode(PackageProject::SelectionMode::ExactlyOne);
			AddMode(PackageProject::SelectionMode::AtLeastOne);
			AddMode(PackageProject::SelectionMode::AtMostOne);
			AddMode(PackageProject::SelectionMode::All);
			m_SelectionModeEditor.SetItems(tChoices);
			m_SelectionModeEditor.EndEditOnCloseup(true);
		}
		
		// Image
		{
			m_ImagesEditor.EndEditOnCloseup(true);
			m_ImagesEditor.SetMaxVisibleItems(16);
		}
	}
	
	void ComponentsModel::ChangeNotify()
	{
		m_Controller->ChangeNotify();
	}
	void ComponentsModel::NotifyChangedItem(const KxDataViewItem& item)
	{
		ItemChanged(item);
		ChangeNotify();
	}
	
	void ComponentsModel::RefreshItems()
	{
		m_Steps.clear();
		for (const auto& step: GetComponents().GetSteps())
		{
			auto& stepNode = m_Steps.emplace_back(std::make_unique<ComponentsModelNode>(step.get()));
			for (const auto& group: step->GetGroups())
			{
				auto& groupNode = stepNode->GetChildren().emplace_back(std::make_unique<ComponentsModelNode>(group.get(), stepNode.get()));
				for (const auto& entry: group->GetItems())
				{
					auto& entryNode = groupNode->GetChildren().emplace_back(std::make_unique<ComponentsModelNode>(entry.get(), groupNode.get()));
					entryNode->CreateFullEntryNode();
				}
			}
		}
		ItemsCleared();
	}
	void ComponentsModel::SetProject(ModPackageProject& project)
	{
		RefreshItems();
	}
}
