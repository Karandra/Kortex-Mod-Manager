#include "stdafx.h"
#include "KInstallWizardComponnetsModel.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "PackageManager/KPackageManager.h"
#include "UI/KMainWindow.h"
#include "KApp.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxString.h>

enum ColumnID
{
	Expander,
	Name,
};

void KInstallWizardComponnetsModel::OnInitControl()
{
	/* View */
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KInstallWizardComponnetsModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_HOVERED, &KInstallWizardComponnetsModel::OnHotTrackItem, this);

	/* Columns */
	GetView()->AppendColumn<KxDataViewTextRenderer>(wxEmptyString, ColumnID::Expander, KxDATAVIEW_CELL_INERT, 0, KxDV_COL_HIDDEN);
	GetView()->AppendColumn<KxDataViewBitmapTextToggleRenderer>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_ACTIVATABLE);
}

bool KInstallWizardComponnetsModel::IsContainer(const KxDataViewItem& item) const
{
	if (const KIWCModelNode* node = GetNode(item))
	{
		return node->IsGroup() && node->HasChildren();
	}
	return false;
}
KxDataViewItem KInstallWizardComponnetsModel::GetParent(const KxDataViewItem& item) const
{
	if (const KIWCModelNode* node = GetNode(item))
	{
		if (node->IsEntry() && node->HasParentNode())
		{
			return MakeItem(node->GetParentNode());
		}
	}
	return KxDataViewItem();
}
void KInstallWizardComponnetsModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
{
	// Root item, read groups
	if (item.IsTreeRootItem())
	{
		for (const KIWCModelNode& node: m_DataVector)
		{
			if (node.IsGroup())
			{
				children.push_back(MakeItem(node));
			}
		}
		return;
	}
	
	// Group item, read entries
	const KIWCModelNode* node = GetNode(item);
	if (const KPPCGroup* group = node->GetGroup())
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

bool KInstallWizardComponnetsModel::GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attibutes, KxDataViewCellState cellState) const
{
	const KIWCModelNode* node = GetNode(item);
	if (node)
	{
		if (const KPPCGroup* group = node->GetGroup())
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					attibutes.SetForegroundColor(KxUtility::GetThemeColor_Caption(GetView()));
					attibutes.SetBold(true);
					return true;
				}
			};
		}
		else if (const KPPCEntry* entry = node->GetEntry())
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					switch (entry->GetTDCurrentValue())
					{
						case KPPC_DESCRIPTOR_RECOMMENDED:
						{
							attibutes.SetItalic(true);
							break;
						}
					}
				}
			};
		}
	}
	return false;
}
void KInstallWizardComponnetsModel::GetValue(wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	if (const KIWCModelNode* node = GetNode(item))
	{
		if (const KPPCGroup* group = node->GetGroup())
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					data = wxString::Format("%s (%s)", group->GetName(), GetSelectionModeString(*group));
					break;
				}
			};
		}
		else if (const KPPCEntry* entry = node->GetEntry())
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
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
bool KInstallWizardComponnetsModel::SetValue(const wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column)
{
	KIWCModelNode* entryNode = GetNode(item);
	if (entryNode)
	{
		if (const KPPCEntry* entry = entryNode->GetEntry())
		{
			const KPPCGroup* group = entryNode->GetParentNode()->GetGroup();
			KPPCSelectionMode selMode = group->GetSelectionMode();
			KPPCTypeDescriptor typeDescriptor = entry->GetTDCurrentValue();

			if (column->GetID() == ColumnID::Name)
			{
				auto CountChecked = [this, entryNode]()
				{
					size_t checkedCount = 0;
					for (const KIWCModelNode* node: GetGroupNodes(entryNode->GetParentNode()))
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
					for (KIWCModelNode* node: GetGroupNodes(entryNode->GetParentNode()))
					{
						node->SetChecked(false);
						NodeChanged(node);
					}
				};

				switch (selMode)
				{
					case KPPC_SELECT_ANY:
					{
						entryNode->ToggleCheck();
						NodeChanged(entryNode);
						return true;
					}
					case KPPC_SELECT_EXACTLY_ONE:
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
					case KPPC_SELECT_AT_LEAST_ONE:
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
					case KPPC_SELECT_AT_MOST_ONE:
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
					case KPPC_SELECT_ALL:
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
bool KInstallWizardComponnetsModel::IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	const KIWCModelNode* node = GetNode(item);
	switch (column->GetID())
	{
		case ColumnID::Name:
		{
			return node && node->IsEntry() && !node->IsRequiredEntry();
		}
	};
	return true;
}

wxBitmap KInstallWizardComponnetsModel::GetImageByTypeDescriptor(KPPCTypeDescriptor type) const
{
	switch (type)
	{
		case KPPC_DESCRIPTOR_NOT_USABLE:
		{
			return KGetBitmap(KIMG_CROSS_CIRCLE_FRAME);
		}
		case KPPC_DESCRIPTOR_COULD_BE_USABLE:
		{
			return KGetBitmap(KIMG_EXCLAMATION);
		}
	};
	return wxNullBitmap;
}
wxString KInstallWizardComponnetsModel::GetMessageTypeDescriptor(KPPCTypeDescriptor type) const
{
	return KTr("PackageCreator.TypeDescriptor." + KPackageProjectComponents::TypeDescriptorToString(type));
}
KxDataViewBitmapTextToggleValue::ToggleType KInstallWizardComponnetsModel::GetToggleType(KPPCSelectionMode mode) const
{
	switch (mode)
	{
		case KPPC_SELECT_EXACTLY_ONE:
		case KPPC_SELECT_AT_MOST_ONE:
		{
			return KxDataViewBitmapTextToggleValue::RadioBox;
		}
	};
	return KxDataViewBitmapTextToggleValue::CheckBox;
}
const wxString& KInstallWizardComponnetsModel::GetSelectionModeString(const KPPCGroup& group) const
{
	static const wxString ms_Select = KTr("Generic.Select");
	auto MakeString = [](KPPCSelectionMode mode) -> wxString
	{
		return ms_Select + ' ' + KxString::MakeLower(KPackageProjectComponents::SelectionModeToTranslation(mode));
	};

	static const wxString ms_Any = MakeString(KPPC_SELECT_ANY);
	static const wxString ms_ExactlyOne = MakeString(KPPC_SELECT_EXACTLY_ONE);
	static const wxString ms_AtLeastOne = MakeString(KPPC_SELECT_AT_LEAST_ONE);
	static const wxString ms_AtMostOne = MakeString(KPPC_SELECT_AT_MOST_ONE);
	static const wxString ms_All = MakeString(KPPC_SELECT_ALL);

	switch (group.GetSelectionMode())
	{
		case KPPC_SELECT_EXACTLY_ONE:
		{
			return ms_ExactlyOne;
		}
		case KPPC_SELECT_AT_LEAST_ONE:
		{
			return ms_AtLeastOne;
		}
		case KPPC_SELECT_AT_MOST_ONE:
		{
			return ms_AtMostOne;
		}
		case KPPC_SELECT_ALL:
		{
			return ms_All;
		}
	};
	return ms_Any;
}
KIWCNodesRefVector KInstallWizardComponnetsModel::GetGroupNodes(const KIWCModelNode* groupNode)
{
	KIWCNodesRefVector items;
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
bool KInstallWizardComponnetsModel::IsEntryShouldBeChecked(const KPPCEntry* entry) const
{
	KPPCTypeDescriptor typeDescriptor = entry->GetTDCurrentValue();
	if (typeDescriptor == KPPC_DESCRIPTOR_REQUIRED || typeDescriptor == KPPC_DESCRIPTOR_RECOMMENDED)
	{
		return true;
	}
	else
	{
		return std::any_of(m_CheckedEntries.begin(), m_CheckedEntries.end(), [entry](const KPPCEntry* v)
		{
			return v == entry;
		});
	}
}

void KInstallWizardComponnetsModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	if (const KIWCModelNode* node = GetNode(item))
	{
		if (const KPPCGroup* group = node->GetGroup())
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
		else if (const KPPCEntry* entry = node->GetEntry())
		{
			KxTaskDialog dialog(GetViewTLW(), KxID_NONE, KTr(KxID_INFO), GetMessageTypeDescriptor(entry->GetTDCurrentValue()), KxBTN_OK, KxICON_INFORMATION);
			dialog.ShowModal();
		}
	}
}
void KInstallWizardComponnetsModel::OnHotTrackItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	if (const KIWCModelNode* node = GetNode(item))
	{
		if (const KPPCEntry* entry = node->GetEntry())
		{
			m_HotItem = entry;
			return;
		}
	}
	m_HotItem = NULL;
}

KInstallWizardComponnetsModel::KInstallWizardComponnetsModel()
{
	SetDataViewFlags(KxDV_NO_HEADER);
}

size_t KInstallWizardComponnetsModel::GetItemsCount() const
{
	size_t count = 0;
	if (m_Step)
	{
		for (const auto& group: m_Step->GetGroups())
		{
			count += group->GetEntries().size() + 1;
		}
	}
	return count;
}
void KInstallWizardComponnetsModel::SetDataVector()
{
	m_DataVector.clear();
	m_CheckedEntries.clear();
	m_ComponentsInfo = NULL;
	m_Step = NULL;

	ItemsCleared();
	GetView()->Disable();
}
void KInstallWizardComponnetsModel::SetDataVector(const KPackageProjectComponents* compInfo, const KPPCStep* step, const KPPCEntry::RefVector& checkedEntries)
{
	SetDataVector();

	m_ComponentsInfo = compInfo;
	m_Step = step;
	m_CheckedEntries = checkedEntries;

	RefreshItems();
	GetView()->Enable();
}
void KInstallWizardComponnetsModel::RefreshItems()
{
	m_DataVector.clear();
	m_DataVector.reserve(GetItemsCount());
	ItemsCleared();

	KxDataViewItem selection;
	KxDataViewItem::Vector groupItems;
	groupItems.reserve(m_Step->GetGroups().size());
	for (const auto& group: m_Step->GetGroups())
	{
		KIWCModelNode& groupNode = m_DataVector.emplace_back(*group);

		size_t groupSize = group->GetEntries().size();
		groupNode.SetBounds(m_DataVector.size(), (m_DataVector.size() - 1) + groupSize + 1);

		KxDataViewItem groupItem = MakeItem(groupNode);
		groupItems.push_back(groupItem);

		KxDataViewItem::Vector entryItems;
		entryItems.reserve(group->GetEntries().size());
		size_t checkedCount = 0;
		for (const auto& entry: group->GetEntries())
		{
			KIWCModelNode& entryNode = m_DataVector.emplace_back(*entry);
			entryNode.SetParentNode(groupNode);

			KxDataViewItem entryItem = MakeItem(entryNode);
			entryItems.push_back(entryItem);
			if (!selection.IsOK())
			{
				selection = entryItem;
			}

			// Check entries of this group if all of them needs to be checked.
			// Or if this is required or recommended item.
			if (group->GetSelectionMode() == KPPC_SELECT_ALL || IsEntryShouldBeChecked(entry.get()))
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
				case KPPC_SELECT_AT_LEAST_ONE:
				case KPPC_SELECT_EXACTLY_ONE:
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
	for (const KIWCModelNode& node: m_DataVector)
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
bool KInstallWizardComponnetsModel::OnLeaveStep(KPPCEntry::RefVector& checkedEntries)
{
	checkedEntries = GetCheckedEntries();
	return true;
}

const KPPCEntry* KInstallWizardComponnetsModel::GetSelectedEntry() const
{
	KIWCModelNode* node = GetNode(GetView()->GetSelection());
	if (node)
	{
		return node->GetEntry();
	}
	return NULL;
}

KxDataViewItem KInstallWizardComponnetsModel::MakeItem(const KIWCModelNode* node) const
{
	return KxDataViewItem(node);
}
KIWCModelNode* KInstallWizardComponnetsModel::GetNode(const KxDataViewItem& item) const
{
	return item.GetValuePtr<KIWCModelNode>();
}
KPPCEntry::RefVector KInstallWizardComponnetsModel::GetCheckedEntries() const
{
	KPPCEntry::RefVector entries;
	for (const KIWCModelNode& node: m_DataVector)
	{
		if (node.IsEntry() && node.IsChecked())
		{
			entries.push_back(const_cast<KPPCEntry*>(node.GetEntry()));
		}
	}
	return entries;
}
