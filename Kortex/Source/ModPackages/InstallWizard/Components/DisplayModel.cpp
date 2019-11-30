#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/PackageManager.hpp>
#include "DisplayModel.h"
#include "PackageCreator/PageBase.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxUxTheme.h>

namespace
{
	enum ColumnRef
	{
		Expander,
		Name,
	};
}

namespace Kortex::InstallWizard::ComponentsPageNS
{
	void DisplayModel::OnInitControl()
	{
		/* View */
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &DisplayModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_HOVERED, &DisplayModel::OnHotTrackItem, this);

		/* Columns */
		GetView()->AppendColumn<KxDataViewTextRenderer>(wxEmptyString, ColumnRef::Expander, KxDATAVIEW_CELL_INERT, 0, KxDV_COL_HIDDEN);
		GetView()->AppendColumn<KxDataViewBitmapTextToggleRenderer>(KTr("Generic.Name"), ColumnRef::Name, KxDATAVIEW_CELL_ACTIVATABLE);
	}

	bool DisplayModel::IsContainer(const KxDataViewItem& item) const
	{
		if (const DisplayModelNode* node = GetNode(item))
		{
			return node->IsGroup() && node->HasChildren();
		}
		return false;
	}
	KxDataViewItem DisplayModel::GetParent(const KxDataViewItem& item) const
	{
		if (const DisplayModelNode* node = GetNode(item))
		{
			if (node->IsEntry() && node->HasParentNode())
			{
				return MakeItem(node->GetParentNode());
			}
		}
		return KxDataViewItem();
	}
	void DisplayModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
	{
		// Root item, read groups
		if (item.IsTreeRootItem())
		{
			for (const DisplayModelNode& node: m_DataVector)
			{
				if (node.IsGroup())
				{
					children.push_back(MakeItem(node));
				}
			}
			return;
		}

		// Group item, read entries
		const DisplayModelNode* node = GetNode(item);
		if (const PackageProject::ComponentGroup* group = node->GetGroup())
		{
			for (size_t i = node->GetBegin(); i < node->GetSize(); i++)
			{
				if (m_DataVector[i].IsEntry())
				{
					children.push_back(MakeItem(m_DataVector[i]));
				}
			}
		}
	}

	bool DisplayModel::GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attibutes, KxDataViewCellState cellState) const
	{
		const DisplayModelNode* node = GetNode(item);
		if (node)
		{
			if (const PackageProject::ComponentGroup* group = node->GetGroup())
			{
				switch (column->GetID())
				{
					case ColumnRef::Name:
					{
						attibutes.SetForegroundColor(KxUxTheme::GetDialogMainInstructionColor(*GetView()));
						attibutes.SetBold(true);
						return true;
					}
				};
			}
			else if (const PackageProject::ComponentItem* entry = node->GetEntry())
			{
				switch (column->GetID())
				{
					case ColumnRef::Name:
					{
						switch (entry->GetTDCurrentValue())
						{
							case PackageProject::TypeDescriptor::Recommended:
							{
								attibutes.SetItalic();
								break;
							}
						}
					}
				};
			}
		}
		return false;
	}
	void DisplayModel::GetValue(wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		if (const DisplayModelNode* node = GetNode(item))
		{
			if (const PackageProject::ComponentGroup* group = node->GetGroup())
			{
				switch (column->GetID())
				{
					case ColumnRef::Name:
					{
						data = wxString::Format("%s (%s)", group->GetName(), GetSelectionModeString(*group));
						break;
					}
				};
			}
			else if (const PackageProject::ComponentItem* entry = node->GetEntry())
			{
				switch (column->GetID())
				{
					case ColumnRef::Name:
					{
						KxDataViewBitmapTextToggleValue value(node->IsChecked(), GetToggleType(node->GetParentNode()->GetGroup()->GetSelectionMode()));
						value.SetBitmap(GetImageByTypeDescriptor(entry->GetTDCurrentValue()));
						value.SetText(entry->GetName());
						data = value;
						break;
					}
				};
			}
		}
	}
	bool DisplayModel::SetValue(const wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column)
	{
		DisplayModelNode* entryNode = GetNode(item);
		if (entryNode)
		{
			if (const PackageProject::ComponentItem* entry = entryNode->GetEntry())
			{
				const PackageProject::ComponentGroup* group = entryNode->GetParentNode()->GetGroup();
				PackageProject::SelectionMode selMode = group->GetSelectionMode();
				PackageProject::TypeDescriptor typeDescriptor = entry->GetTDCurrentValue();

				if (column->GetID() == ColumnRef::Name)
				{
					auto CountChecked = [this, entryNode]()
					{
						size_t checkedCount = 0;
						for (const DisplayModelNode* node: GetGroupNodes(entryNode->GetParentNode()))
						{
							if (node->IsChecked())
							{
								checkedCount++;
							}
						}
						return checkedCount;
					};
					auto SetAllChecked = [this, entryNode](bool bCheck)
					{
						for (DisplayModelNode* node: GetGroupNodes(entryNode->GetParentNode()))
						{
							node->SetChecked(false);
							NodeChanged(node);
						}
					};

					switch (selMode)
					{
						case PackageProject::SelectionMode::Any:
						{
							entryNode->ToggleCheck();
							NodeChanged(entryNode);
							return true;
						}
						case PackageProject::SelectionMode::ExactlyOne:
						{
							// Is this entry is going to be checked, then uncheck all entries in this group
							// and check this one.
							if (!entryNode->IsChecked())
							{
								SetAllChecked(false);

								if (CountChecked() == 0)
								{
									entryNode->SetChecked(true);
									NodeChanged(entryNode);
								}
								return true;
							}
							break;
						}
						case PackageProject::SelectionMode::AtLeastOne:
						{
							if (!entryNode->IsChecked())
							{
								entryNode->SetChecked(true);
								NodeChanged(entryNode);
							}
							else if (CountChecked() > 1)
							{
								entryNode->SetChecked(false);
								NodeChanged(entryNode);
							}
							return true;
						}
						case PackageProject::SelectionMode::AtMostOne:
						{
							if (!entryNode->IsChecked())
							{
								SetAllChecked(false);

								if (CountChecked() == 0)
								{
									entryNode->SetChecked(true);
									NodeChanged(entryNode);
								}
							}
							else
							{
								entryNode->SetChecked(false);
								NodeChanged(entryNode);
							}
							return true;
						}
						case PackageProject::SelectionMode::All:
						{
							entryNode->SetChecked(true);
							NodeChanged(entryNode);
						}
					};
				}
			}
		}
		return false;
	}
	bool DisplayModel::IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		const DisplayModelNode* node = GetNode(item);
		switch (column->GetID())
		{
			case ColumnRef::Name:
			{
				return node && node->IsEntry() && !node->IsRequiredEntry();
			}
		};
		return true;
	}

	wxBitmap DisplayModel::GetImageByTypeDescriptor(PackageProject::TypeDescriptor type) const
	{
		switch (type)
		{
			case PackageProject::TypeDescriptor::NotUsable:
			{
				return ImageProvider::GetBitmap(ImageResourceID::CrossCircleFrame);
			}
			case PackageProject::TypeDescriptor::CouldBeUsable:
			{
				return ImageProvider::GetBitmap(ImageResourceID::Exclamation);
			}
		};
		return wxNullBitmap;
	}
	wxString DisplayModel::GetMessageTypeDescriptor(PackageProject::TypeDescriptor type) const
	{
		return KTr("PackageCreator.TypeDescriptor." + PackageProject::ComponentsSection::TypeDescriptorToString(type));
	}
	KxDataViewBitmapTextToggleValue::ToggleType DisplayModel::GetToggleType(PackageProject::SelectionMode mode) const
	{
		switch (mode)
		{
			case PackageProject::SelectionMode::ExactlyOne:
			case PackageProject::SelectionMode::AtMostOne:
			{
				return KxDataViewBitmapTextToggleValue::RadioBox;
			}
		};
		return KxDataViewBitmapTextToggleValue::CheckBox;
	}
	const wxString& DisplayModel::GetSelectionModeString(const PackageProject::ComponentGroup& group) const
	{
		static const wxString ms_Select = KTr("Generic.Select");
		auto MakeString = [](PackageProject::SelectionMode mode) -> wxString
		{
			return ms_Select + ' ' + KxString::MakeLower(PackageProject::ComponentsSection::SelectionModeToTranslation(mode));
		};

		static const wxString ms_Any = MakeString(PackageProject::SelectionMode::Any);
		static const wxString ms_ExactlyOne = MakeString(PackageProject::SelectionMode::ExactlyOne);
		static const wxString ms_AtLeastOne = MakeString(PackageProject::SelectionMode::AtLeastOne);
		static const wxString ms_AtMostOne = MakeString(PackageProject::SelectionMode::AtMostOne);
		static const wxString ms_All = MakeString(PackageProject::SelectionMode::All);

		switch (group.GetSelectionMode())
		{
			case PackageProject::SelectionMode::ExactlyOne:
			{
				return ms_ExactlyOne;
			}
			case PackageProject::SelectionMode::AtLeastOne:
			{
				return ms_AtLeastOne;
			}
			case PackageProject::SelectionMode::AtMostOne:
			{
				return ms_AtMostOne;
			}
			case PackageProject::SelectionMode::All:
			{
				return ms_All;
			}
		};
		return ms_Any;
	}
	DisplayModelNode::RefVector DisplayModel::GetGroupNodes(const DisplayModelNode* groupNode)
	{
		DisplayModelNode::RefVector items;
		if (groupNode && groupNode->IsGroup())
		{
			for (size_t i = groupNode->GetBegin(); i < groupNode->GetSize(); i++)
			{
				if (m_DataVector[i].IsEntry())
				{
					items.push_back(&m_DataVector[i]);
				}
			}
		}
		return items;
	}
	bool DisplayModel::IsEntryShouldBeChecked(const PackageProject::ComponentItem* entry) const
	{
		PackageProject::TypeDescriptor typeDescriptor = entry->GetTDCurrentValue();
		if (typeDescriptor == PackageProject::TypeDescriptor::Required || typeDescriptor == PackageProject::TypeDescriptor::Recommended)
		{
			return true;
		}
		else
		{
			return std::any_of(m_CheckedEntries.begin(), m_CheckedEntries.end(), [entry](const PackageProject::ComponentItem* v)
			{
				return v == entry;
			});
		}
	}

	void DisplayModel::OnActivateItem(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		if (const DisplayModelNode* node = GetNode(item))
		{
			if (const PackageProject::ComponentGroup* group = node->GetGroup())
			{
				if (GetView()->IsExpanded(item))
				{
					GetView()->Collapse(item);
				}
				else
				{
					GetView()->Expand(item);
				}
			}
			else if (const PackageProject::ComponentItem* entry = node->GetEntry())
			{
				KxTaskDialog dialog(GetViewTLW(), KxID_NONE, KTr(KxID_INFO), GetMessageTypeDescriptor(entry->GetTDCurrentValue()), KxBTN_OK, KxICON_INFORMATION);
				dialog.ShowModal();
			}
		}
	}
	void DisplayModel::OnHotTrackItem(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		if (const DisplayModelNode* node = GetNode(item))
		{
			if (const PackageProject::ComponentItem* entry = node->GetEntry())
			{
				m_HotItem = entry;
				return;
			}
		}
		m_HotItem = nullptr;
	}

	DisplayModel::DisplayModel()
	{
		SetDataViewFlags(KxDV_NO_HEADER);
	}

	size_t DisplayModel::GetItemsCount() const
	{
		size_t count = 0;
		if (m_Step)
		{
			for (const auto& group: m_Step->GetGroups())
			{
				count += group->GetItems().size() + 1;
			}
		}
		return count;
	}
	void DisplayModel::SetDataVector()
	{
		m_DataVector.clear();
		m_CheckedEntries.clear();
		m_ComponentsInfo = nullptr;
		m_Step = nullptr;

		ItemsCleared();
		GetView()->Disable();
	}
	void DisplayModel::SetDataVector(const PackageProject::ComponentsSection* compInfo, const PackageProject::ComponentStep* step, const PackageProject::ComponentItem::RefVector& checkedEntries)
	{
		SetDataVector();

		m_ComponentsInfo = compInfo;
		m_Step = step;
		m_CheckedEntries = checkedEntries;

		RefreshItems();
		GetView()->Enable();
	}
	void DisplayModel::RefreshItems()
	{
		m_DataVector.clear();
		m_DataVector.reserve(GetItemsCount());
		ItemsCleared();

		KxDataViewItem selection;
		KxDataViewItem::Vector groupItems;
		groupItems.reserve(m_Step->GetGroups().size());
		for (const auto& group: m_Step->GetGroups())
		{
			DisplayModelNode& groupNode = m_DataVector.emplace_back(*group);

			size_t groupSize = group->GetItems().size();
			groupNode.SetBounds(m_DataVector.size(), (m_DataVector.size() - 1) + groupSize + 1);

			KxDataViewItem groupItem = MakeItem(groupNode);
			groupItems.push_back(groupItem);

			KxDataViewItem::Vector entryItems;
			entryItems.reserve(group->GetItems().size());
			size_t checkedCount = 0;
			for (const auto& entry: group->GetItems())
			{
				DisplayModelNode& entryNode = m_DataVector.emplace_back(*entry);
				entryNode.SetParentNode(groupNode);

				KxDataViewItem entryItem = MakeItem(entryNode);
				entryItems.push_back(entryItem);
				if (!selection.IsOK())
				{
					selection = entryItem;
				}

				// Check entries of this group if all of them needs to be checked.
				// Or if this is required or recommended item.
				if (group->GetSelectionMode() == PackageProject::SelectionMode::All || IsEntryShouldBeChecked(entry.get()))
				{
					checkedCount++;
					entryNode.SetChecked(true);
				}
			}

			// if no items was checked and at least one of them needs to be checked (by selection mode),
			// check the first one.
			if (checkedCount == 0 && !entryItems.empty())
			{
				switch (group->GetSelectionMode())
				{
					case PackageProject::SelectionMode::AtLeastOne:
					case PackageProject::SelectionMode::ExactlyOne:
					{
						GetNode(entryItems[0])->SetChecked(true);
						break;
					}
				};
			}
			ItemsAdded(groupItem, entryItems);
		}
		ItemsAdded(KxDataViewItem(), groupItems);

		// Expand all groups
		for (const DisplayModelNode& node: m_DataVector)
		{
			if (node.IsGroup())
			{
				GetView()->Expand(MakeItem(node));
			}
		}

		// Select first entry item
		SelectItem(selection);
		GetView()->SetFocus();
	}
	bool DisplayModel::OnLeaveStep(PackageProject::ComponentItem::RefVector& checkedEntries)
	{
		checkedEntries = GetCheckedEntries();
		return true;
	}

	const PackageProject::ComponentItem* DisplayModel::GetSelectedEntry() const
	{
		DisplayModelNode* node = GetNode(GetView()->GetSelection());
		if (node)
		{
			return node->GetEntry();
		}
		return nullptr;
	}

	KxDataViewItem DisplayModel::MakeItem(const DisplayModelNode* node) const
	{
		return KxDataViewItem(node);
	}
	DisplayModelNode* DisplayModel::GetNode(const KxDataViewItem& item) const
	{
		return item.GetValuePtr<DisplayModelNode>();
	}
	PackageProject::ComponentItem::RefVector DisplayModel::GetCheckedEntries() const
	{
		PackageProject::ComponentItem::RefVector entries;
		for (const DisplayModelNode& node: m_DataVector)
		{
			if (node.IsEntry() && node.IsChecked())
			{
				entries.push_back(const_cast<PackageProject::ComponentItem*>(node.GetEntry()));
			}
		}
		return entries;
	}
}
