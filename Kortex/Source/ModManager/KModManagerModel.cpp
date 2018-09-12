#include "stdafx.h"
#include "KModManagerModel.h"
#include "KModManagerWorkspace.h"
#include "KModManager.h"
#include "KPriorityGroupEntry.h"
#include "Network/KNetwork.h"
#include "PluginManager/KPluginManager.h"
#include "PluginManager/KPluginManagerWorkspace.h"
#include "PluginManager/KPluginViewBaseModel.h"
#include "UI/KMainWindow.h"
#include "KComparator.h"
#include "KApp.h"
#include "KAux.h"

enum ColumnID
{
	Name,
	Priority,
	Version,
	Author,
	Tags,

	Sites,
	Sites_TESALLID,
	Sites_NexusID,
	Sites_LoversLabID,

	DateInstall,
	DateUninstall,
	ModFolder,
	PackagePath,
	Signature,

	MAX
};
using SitesRenderer = KxDataViewImageListRenderer<KNETWORK_PROVIDER_ID_MAX + 1>;
using SitesValue = typename SitesRenderer::ValueT;

bool KModManagerModel::IsSpecialSiteColumn(int column) const
{
	switch (column)
	{
		case ColumnID::Sites_TESALLID:
		case ColumnID::Sites_NexusID:
		case ColumnID::Sites_LoversLabID:
		{
			return true;
		}
	};
	return false;
}
KNetworkProviderID KModManagerModel::ColumnToSpecialSite(int column) const
{
	return (KNetworkProviderID)(column - ColumnID::Sites - 1);
}
wxString KModManagerModel::FormatTagList(const KModEntry* entry) const
{
	const KModManagerTags& tagManager = KModManager::GetTagManager();
	KxStringVector tags;
	tags.reserve(entry->GetTags().size());

	for (const wxString& tagID: entry->GetTags())
	{
		tags.push_back(tagManager.GetTagName(tagID));
	}
	return KxString::Join(tags, "; ");
}

void KModManagerModel::OnInitControl()
{
	/* View */
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KModManagerModel::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KModManagerModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KModManagerModel::OnContextMenu, this);
	GetView()->Bind(KxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, &KModManagerModel::OnHeaderContextMenu, this);
	GetView()->Bind(KxEVT_DATAVIEW_COLUMN_SORTED, &KModManagerModel::OnColumnSorted, this);
	
	EnableDragAndDrop();
	m_PriortyGroupColor = KxUtility::GetThemeColor_Caption(GetView());

	/* Columns */
	KxDataViewColumnFlags defaultFlags = KxDV_COL_RESIZEABLE|KxDV_COL_REORDERABLE|KxDV_COL_SORTABLE;
	KxDataViewColumnFlags defaultFlagsNoSort = (KxDataViewColumnFlags)(defaultFlags & ~KxDV_COL_SORTABLE);
	KxDataViewColumnFlags defaultFlagsNoOrder = (KxDataViewColumnFlags)(defaultFlags & ~KxDV_COL_REORDERABLE);
	KxDataViewColumnFlags defaultFlagsNoSortNoOrder = KxDV_COL_RESIZEABLE;

	GetView()->AppendColumn<KxDataViewBitmapTextToggleRenderer, KxDataViewTextEditor>(T("ModManager.ModList.Name"), ColumnID::Name, KxDATAVIEW_CELL_ACTIVATABLE|KxDATAVIEW_CELL_EDITABLE, 400, defaultFlags);

	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Priority"), ColumnID::Priority, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE, defaultFlags);
		m_PriorityColumn = info.GetColumn();
		m_PriorityColumn->SortAscending();
	}

	GetView()->AppendColumn<KxDataViewBitmapTextRenderer, KxDataViewTextEditor>(T("ModManager.ModList.Version"), ColumnID::Version, KxDATAVIEW_CELL_EDITABLE, 100, defaultFlags);
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("ModManager.ModList.Author"), ColumnID::Author, KxDATAVIEW_CELL_EDITABLE, 100, defaultFlags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("ModManager.Tags"), ColumnID::Tags, KxDATAVIEW_CELL_INERT, 100, defaultFlags);
	
	{
		int spacing = 1;
		int width = (spacing +  16) * (KNETWORK_PROVIDER_ID_MAX + 1);
		auto info = GetView()->AppendColumn<SitesRenderer>(T("ModManager.ModList.Sites"), ColumnID::Sites, KxDATAVIEW_CELL_INERT, width, defaultFlagsNoSort);

		info.GetRenderer()->SetImageList(KApp::Get().GetImageList());
		info.GetRenderer()->SetSpacing(1);
		info.GetRenderer()->SetAlignment(wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
	}

	auto AddSiteColumn = [this, defaultFlags](KNetworkProviderID index)
	{
		int id = ColumnID::Sites + index + 1;
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(wxEmptyString, id, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE, defaultFlags);
		
		KNetworkProvider* site = KNetwork::GetInstance()->GetProvider(index);
		info.GetColumn()->SetTitle(site->GetName());
		info.GetColumn()->SetBitmap(KGetBitmap(site->GetIcon()));
	};
	for (int i = 0; i < KNETWORK_PROVIDER_ID_MAX; i++)
	{
		AddSiteColumn((KNetworkProviderID)i);
	}

	GetView()->AppendColumn<KxDataViewTextRenderer>(T("ModManager.ModList.DateInstall"), ColumnID::DateInstall, KxDATAVIEW_CELL_INERT, 125, defaultFlags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("ModManager.ModList.DateUninstall"), ColumnID::DateUninstall, KxDATAVIEW_CELL_INERT,  125, defaultFlags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("ModManager.ModList.ModFolder"), ColumnID::ModFolder, KxDATAVIEW_CELL_INERT, 125, defaultFlags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("ModManager.ModList.PackagePath"), ColumnID::PackagePath, KxDATAVIEW_CELL_INERT, 125, defaultFlags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("ModManager.ModList.Signature"), ColumnID::Signature, KxDATAVIEW_CELL_INERT, 125, defaultFlags);
}

bool KModManagerModel::IsListModel() const
{
	return false;
}
bool KModManagerModel::IsContainer(const KxDataViewItem& item) const
{
	if (IsTree())
	{
		if (const KMMModelNode* node = GetNode(item))
		{
			return node->IsGroup() && node->HasChildren();
		}
		return item.IsTreeRootItem();
	}
	else
	{
		const KMMModelNode* node = GetNode(item);
		if (node)
		{
			if (KModEntry* entry = node->GetEntry())
			{
				if (KPriorityGroupEntry* priorityGroup = entry->ToPriorityGroup())
				{
					return priorityGroup->IsBegin();
				}
			}
		}
	}
	return item.IsTreeRootItem();
}
bool KModManagerModel::HasContainerColumns(const KxDataViewItem& item) const
{
	return true;
}
KxDataViewItem KModManagerModel::GetParent(const KxDataViewItem& item) const
{
	if (const KMMModelNode* node = GetNode(item))
	{
		if (node->IsEntry() && node->HasParentNode())
		{
			return MakeItem(node->GetParentNode());
		}
	}
	return KxDataViewItem();
}

void KModManagerModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
{
	// Root item, read groups
	if (item.IsTreeRootItem())
	{
		children.reserve(m_DataVector.size());
		for (const KMMModelNode& node: m_DataVector)
		{
			if (IsTree())
			{
				if (node.IsGroup())
				{
					children.push_back(MakeItem(node));
				}
			}
			else
			{
				if (node.IsEntry())
				{
					children.push_back(MakeItem(node));
				}
			}
		}
	}
	else
	{
		// Group (priority group) item, read entries
		const KMMModelNode* node = GetNode(item);
		children.reserve(node->GetChildrenCount());

		const KModTag* group = node->GetGroup();
		if (group || node->GetEntry()->ToPriorityGroup())
		{
			for (const KMMModelNode& entryNode: node->GetChildren())
			{
				children.push_back(MakeItem(entryNode));
			}
		}
	}
}
void KModManagerModel::GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	const KMMModelNode* node = GetNode(item);
	if (KModEntry* entry = node->GetEntry())
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = entry->GetName();
				return;
			}
			case ColumnID::Version:
			{
				value = entry->GetVersion().ToString();
				return;
			}
		};
	}
	GetValue(value, item, column);
}
void KModManagerModel::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	const KMMModelNode* node = GetNode(item);
	if (const KModTag* group = node->GetGroup())
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = wxString::Format("%s (%zu)", group->GetLabel(), CountItemsInGroup(group));
				break;
			}
		};
	}
	else if (KModEntry* entry = node->GetEntry())
	{
		const KFixedModEntry* fixedEntry = entry->ToFixedEntry();
		const KPriorityGroupEntry* priorityGroupEntry = entry->ToPriorityGroup();

		if (priorityGroupEntry)
		{
			GetValue(value, item, column, priorityGroupEntry);
		}
		else if (fixedEntry)
		{
			GetValue(value, item, column, fixedEntry);
		}
		else
		{
			GetValue(value, item, column, entry);
		}
	}
}
void KModManagerModel::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column, const KModEntry* entry) const
{
	switch (column->GetID())
	{
		case ColumnID::Name:
		{
			KxDataViewBitmapTextToggleValue valueData(entry->IsEnabled(), KxDataViewBitmapTextToggleValue::CheckBox);
			if (entry->GetName() != entry->GetID())
			{
				valueData.SetText(wxString::Format("%s (%s)", entry->GetName(), entry->GetID()));
			}
			else
			{
				valueData.SetText(entry->GetName());
			}

			valueData.SetBitmap(KGetBitmap(entry->GetIcon()));
			value = valueData;
			break;
		}
		case ColumnID::Priority:
		{
			if (entry->IsEnabled())
			{
				value = entry->GetPriority();
			}
			break;
		}
		case ColumnID::Version:
		{
			const KxVersion& version = entry->GetVersion();
			KxDataViewBitmapTextValue valueData(version);
			if (version.IsOK())
			{
				switch (version.GetType())
				{
					case KxVERSION_DATETIME:
					{
						valueData.SetBitmap(KGetBitmap(KIMG_CALENDAR_DAY));
						break;
					}
					case KxVERSION_INTEGER:
					{
						valueData.SetBitmap(KGetBitmap(KIMG_NOTIFICATION_COUNTER_42));
						break;
					}
				};
			}
			value = valueData;
			break;
		}
		case ColumnID::Author:
		{
			value = entry->GetAuthor();
			break;
		}
		case ColumnID::Tags:
		{
			value = FormatTagList(entry);
			break;
		}
		case ColumnID::Sites:
		{
			SitesValue::ArrayT list;
			SitesValue::ClearArray(list);
			for (int i = 0; i < KNETWORK_PROVIDER_ID_MAX; i++)
			{
				if (entry->HasWebSite((KNetworkProviderID)i))
				{
					list[i] = KNetwork::GetInstance()->GetProvider(i)->GetIcon();
				}
			}
			if (!entry->GetWebSites().empty())
			{
				list[KNETWORK_PROVIDER_ID_MAX] = KNetworkProvider::GetGenericIcon();
			}
			value = SitesValue(list);
			break;
		}
		case ColumnID::Sites_TESALLID:
		case ColumnID::Sites_NexusID:
		case ColumnID::Sites_LoversLabID:
		{
			KNetworkProviderID index = (KNetworkProviderID)(column->GetID() - ColumnID::Sites - 1);
			if (entry->HasWebSite(index))
			{
				value = entry->GetWebSiteModID(index);
			}
			break;
		}
		case ColumnID::DateInstall:
		{
			value = KAux::FormatDateTime(entry->GetTime(KME_TIME_INSTALL));
			break;
		}
		case ColumnID::DateUninstall:
		{
			value = KAux::FormatDateTime(entry->GetTime(KME_TIME_UNINSTALL));
			break;
		}
		case ColumnID::ModFolder:
		{
			if (entry->IsLinkedMod())
			{
				value = entry->GetLocation(KMM_LOCATION_MOD_FILES);
			}
			else
			{
				value = entry->GetLocation(KMM_LOCATION_MOD_ROOT);
			}
			break;
		}
		case ColumnID::PackagePath:
		{
			value = entry->GetInstallPackageFile();
			break;
		}
		case ColumnID::Signature:
		{
			value = entry->GetSignature();
			break;
		}
	};
}
void KModManagerModel::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column, const KFixedModEntry* entry) const
{
	switch (column->GetID())
	{
		case ColumnID::Name:
		{
			value = KxDataViewBitmapTextToggleValue(true, entry->GetName(), KGetBitmap(entry->GetIcon()), KxDataViewBitmapTextToggleValue::InvalidType);
			break;
		}
		case ColumnID::Priority:
		{
			if (entry->IsEnabled())
			{
				value = entry->GetPriority();
			}
			break;
		}
		case ColumnID::ModFolder:
		{
			value = entry->GetLocation(KMM_LOCATION_MOD_ROOT);
			break;
		}
	};
}
void KModManagerModel::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column, const KPriorityGroupEntry* entry) const
{
	switch (column->GetID())
	{
		case ColumnID::Name:
		{
			bool isBegin = entry->IsBegin();
			if (!m_PriorityColumn->IsSortedAscending())
			{
				isBegin = !isBegin;
			}

			wxString label;
			if (isBegin)
			{
				label = wxString::Format("<%s>", entry->GetPriorityGroupTag());
			}
			else
			{
				label = wxString::Format("</%s>", entry->GetPriorityGroupTag());
			}
			value = KxDataViewBitmapTextToggleValue(true, label, KGetBitmap(entry->GetIcon()), KxDataViewBitmapTextToggleValue::InvalidType);
			break;
		}
	};
}

bool KModManagerModel::SetValue(const wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column)
{
	const KMMModelNode* node = GetNode(item);
	if (KModEntry* entry = node->GetEntry())
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				bool isChanged = false;
				if (value.CheckType<wxString>())
				{
					entry->SetName(value.As<wxString>());
					isChanged = true;
				}
				else
				{
					bool checked = value.As<bool>();
					entry->SetEnabled(checked);
					isChanged = true;

					KModEvent(KEVT_MOD_FILES_CHNAGED, *entry).Send();
				}

				if (isChanged)
				{
					entry->Save();
					KModManager::Get().SaveSate();
					KModManagerWorkspace::GetInstance()->RefreshPlugins();

					KModEvent(KEVT_MOD_CHNAGED, *entry).Send();
					return true;
				}
				return false;
			}
			case ColumnID::Version:
			{
				wxString newVersion = value.As<wxString>();
				if (newVersion != entry->GetVersion())
				{
					entry->SetVersion(newVersion);
					entry->Save();

					KModEvent(KEVT_MOD_CHNAGED, *entry).Send();
					return true;
				}
				return false;
			}
			case ColumnID::Author:
			{
				wxString author = value.As<wxString>();
				if (author != entry->GetAuthor())
				{
					entry->SetAuthor(author);
					entry->Save();

					KModEvent(KEVT_MOD_CHNAGED, *entry).Send();
					return true;
				}
				break;
			}
		};
	}
	return false;
}
bool KModManagerModel::IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	const KMMModelNode* node = GetNode(item);
	if (const KModTag* group = node->GetGroup())
	{
		return false;
	}
	else if (KModEntry* entry = node->GetEntry())
	{
		if (entry->ToFixedEntry())
		{
			return false;
		}

		bool bChangesAllowed = KModManagerWorkspace::GetInstance()->IsChangingModsAllowed();
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				return bChangesAllowed && entry->IsInstalled();
			}
			default:
			{
				return bChangesAllowed;
			}
		};
	}
	return false;
}
bool KModManagerModel::IsEditorEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	const KMMModelNode* node = GetNode(item);
	if (KModEntry* entry = node->GetEntry())
	{
		return !entry->ToFixedEntry();
	}
	return false;
}
bool KModManagerModel::GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
{
	int columnID = column->GetID();
	const KMMModelNode* node = GetNode(item);

	if (const KModTag* group = node->GetGroup())
	{
		if (GetView()->IsExpanded(item) || columnID == ColumnID::Name)
		{
			attributes.SetHeaderButtonBackgound();
			return true;
		}
	}
	else if (KModEntry* entry = node->GetEntry())
	{
		const KFixedModEntry* fixed = entry->ToFixedEntry();
		const KPriorityGroupEntry* priorityGroup = entry->ToPriorityGroup();

		if (columnID == ColumnID::Name && fixed && !priorityGroup)
		{
			attributes.SetItalic();
		}
		if (!fixed && (columnID == ColumnID::Name || IsSpecialSiteColumn(columnID)))
		{
			attributes.SetUnderlined(cellState & KxDATAVIEW_CELL_HIGHLIGHTED && column->IsHotTracked());
		}
		if (priorityGroup)
		{
			attributes.SetForegroundColor(m_PriortyGroupColor);
			attributes.SetAlignment(wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
		}
		return !attributes.IsDefault();
	}
	return false;
}
bool KModManagerModel::Compare(const KxDataViewItem& item1, const KxDataViewItem& item2, const KxDataViewColumn* column) const
{
	const KMMModelNode* pNode1 = GetNode(item1);
	const KMMModelNode* pNode2 = GetNode(item2);
	if (pNode1 && pNode2)
	{
		ColumnID columnID = column ? (ColumnID)column->GetID() : ColumnID::Priority;

		KModEntry* entry1 = pNode1->GetEntry();
		KModEntry* entry2 = pNode2->GetEntry();
		if (entry1 && entry2)
		{
			using KComparator::KCompare;

			if (IsSpecialSiteColumn(columnID))
			{
				KNetworkProviderID index = ColumnToSpecialSite(columnID);
				return entry1->GetWebSiteModID(index) < entry2->GetWebSiteModID(index);
			}

			switch (columnID)
			{
				case ColumnID::Name:
				{
					return KCompare(entry1->GetName(), entry2->GetName()) < 0;
				}
				case ColumnID::Priority:
				{
					return entry1->GetOrderIndex() < entry2->GetOrderIndex();
				}
				case ColumnID::Version:
				{
					return entry1->GetVersion() < entry2->GetVersion();
				}
				case ColumnID::Author:
				{
					return KCompare(entry1->GetAuthor(), entry2->GetAuthor()) < 0;
				}
				case ColumnID::Tags:
				{
					return KCompare(FormatTagList(entry1), FormatTagList(entry2)) < 0;
				}
				case ColumnID::DateInstall:
				{
					return entry1->GetTime(KME_TIME_INSTALL) < entry2->GetTime(KME_TIME_INSTALL);
				}
				case ColumnID::DateUninstall:
				{
					return entry1->GetTime(KME_TIME_UNINSTALL) < entry2->GetTime(KME_TIME_UNINSTALL);
				}
				case ColumnID::ModFolder:
				{
					return KCompare(entry1->GetLocation(KMM_LOCATION_MOD_FILES), entry2->GetLocation(KMM_LOCATION_MOD_FILES)) < 0;
				}
				case ColumnID::PackagePath:
				{
					return KCompare(entry1->GetInstallPackageFile(), entry2->GetInstallPackageFile()) < 0;
				}
				case ColumnID::Signature:
				{
					return KCompare(entry1->GetSignature(), entry2->GetSignature()) < 0;
				}
			};
		}
	}
	return false;
}

void KModManagerModel::OnSelectItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	KModEntry* entry = GetModEntry(item);

	if (entry && column)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				if (KPluginManager* manager = KPluginManager::GetInstance())
				{
					KPluginManagerWorkspace* workspace = KPluginManagerWorkspace::GetInstance();
					wxWindowUpdateLocker lock(workspace);

					workspace->HighlightPlugin();
					for (auto& pluginEntry: manager->GetEntries())
					{
						if (pluginEntry->GetParentMod() == entry)
						{
							workspace->HighlightPlugin(pluginEntry.get());
						}
					}
				}
				break;
			}
		};
	};

	KModManagerWorkspace::GetInstance()->ProcessSelection(entry);
}
void KModManagerModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	const KMMModelNode* node = GetNode(item);
	KModEntry* entry = node->GetEntry();

	if (node->IsGroup())
	{
		GetView()->ToggleItemExpanded(item);
	}
	else if (column && entry)
	{
		int nColumn = column->GetID();

		// If this is a site open click
		KNetworkProviderID providerID = ColumnToSpecialSite(nColumn);
		if (IsSpecialSiteColumn(nColumn))
		{
			if (entry->HasWebSite(providerID))
			{
				KAux::AskOpenURL(entry->GetWebSite(providerID).GetValue(), GetViewTLW());
			}
			else
			{
				AskOpenSites(entry);
			}
			return;
		}

		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				if (!entry->ToFixedEntry() && entry->IsInstallPackageFileExist())
				{
					KModManagerWorkspace::GetInstance()->OpenPackage(entry->GetInstallPackageFile());
				}
				break;
			}
			case ColumnID::Sites:
			{
				AskOpenSites(entry);
				break;
			}
			default:
			{
				GetView()->EditItem(item, column);
				break;
			}
		};
	}

	event.Skip();
}
void KModManagerModel::OnContextMenu(KxDataViewEvent& event)
{
	const KMMModelNode* node = GetNode(event.GetItem());
	KxDataViewColumn* column = event.GetColumn();
	if (node && column)
	{
		if (const KModTag* group = node->GetGroup())
		{
			KModManagerWorkspace::GetInstance()->ShowViewContextMenu(group);
			return;
		}
	}
	KModManagerWorkspace::GetInstance()->ShowViewContextMenu(node && column ? node->GetEntry() : NULL);
}
void KModManagerModel::OnHeaderContextMenu(KxDataViewEvent& event)
{
	KxMenu menu;
	if (GetView()->CreateColumnSelectionMenu(menu))
	{
		GetView()->OnColumnSelectionMenu(menu);
	}
}
void KModManagerModel::OnColumnSorted(KxDataViewEvent& event)
{
	KxDataViewColumn* column = event.GetColumn();
	int id = column->GetID();
	bool suppressed = m_ShowPriorityGroupsSuppress;

	if (m_ShowPriorityGroups && id != ColumnID::Priority)
	{
		m_ShowPriorityGroupsSuppress = true;
		RefreshItems();
	}
	else
	{
		m_ShowPriorityGroupsSuppress = false;
		if (suppressed)
		{
			RefreshItems();
		}
	}
}

void KModManagerModel::AskOpenSites(const KModEntry* entry) const
{
	// Copy array
	KLabeledValueArray urlList = entry->GetWebSites();
	
	// Add fixed
	for (int i = 0; i < KNETWORK_PROVIDER_ID_MAX; i++)
	{
		urlList.emplace_back(entry->GetWebSite((KNetworkProviderID)i));
	};

	// Show dialog
	KAux::AskOpenURL(urlList, GetViewTLW());
}

bool KModManagerModel::OnDragItems(KxDataViewEventDND& event)
{
	if (CanDragDropNow())
	{
		KxDataViewItem::Vector selected;
		if (GetView()->GetSelections(selected) > 0)
		{
			std::unique_ptr<KModManagerModelDataObject> dataObject;
			for (const auto& item: selected)
			{
				const KMMModelNode* node = GetNode(item);
				if (KModEntry* entry = node->GetEntry())
				{
					if (!entry->ToFixedEntry())
					{
						if (!dataObject)
						{
							dataObject = std::make_unique<KModManagerModelDataObject>(selected.size());
						}
						dataObject->AddEntry(entry);
					}
				}
			}

			if (dataObject)
			{
				SetDragDropDataObject(dataObject.release());
				event.SetDragFlags(wxDrag_AllowMove);
				event.SetDropEffect(wxDragMove);
				return true;
			}
		}
	}

	event.SetDropEffect(wxDragError);
	return false;
}
bool KModManagerModel::OnDropItems(KxDataViewEventDND& event)
{
	const KMMModelNode* node = GetNode(event.GetItem());
	if (node)
	{
		KModEntry* entry = node->GetEntry();
		if (entry && !entry->ToFixedEntry() && HasDragDropDataObject())
		{
			const KModEntryArray& entriesToMove = GetDragDropDataObject()->GetEntries();

			// Move and refresh
			if (KModManager::Get().MoveModsIntoThis(entriesToMove, entry))
			{
				RefreshItems();

				// Select moved items and Event-select the first one
				for (KModEntry* entry: entriesToMove)
				{
					entry->Save();
					GetView()->Select(GetItemByEntry(entry));
				}

				SelectItem(GetItemByEntry(entriesToMove.front()));
				event.SetDropEffect(wxDragMove);

				KModEvent(KEVT_MOD_CHNAGED).Send();
				return true;
			}
		}
	}

	wxBell();
	event.SetDropEffect(wxDragError);
	return false;
}
bool KModManagerModel::CanDragDropNow() const
{
	if (KModManagerWorkspace::GetInstance()->IsMovingModsAllowed())
	{
		if (KxDataViewColumn* column = GetView()->GetSortingColumn())
		{
			return column->GetID() == ColumnID::Priority && column->IsSortedAscending();
		}
		return true;
	}
	return false;
}

KModManagerModel::KModManagerModel()
	:m_NoneTag(wxEmptyString, KAux::MakeNoneLabel(), true),
	m_SearchFilterOptions(KModManagerWorkspace::GetInstance(), "SearchFilter")
{
	SetDataViewFlags(KxDataViewCtrl::DefaultStyle|KxDV_MULTIPLE_SELECTION|KxDV_NO_TIMEOUT_EDIT|KxDV_VERT_RULES|KxDV_DOUBLE_CLICK_EXPAND);
}

void KModManagerModel::SetDisplayMode(KModManagerModelType mode)
{
	switch (mode)
	{
		case KMM_TYPE_CONNECTOR:
		{
			m_DisplayMode = mode;
			GetView()->SetIndent(0);
			break;
		}
		case KMM_TYPE_MANAGER:
		{
			m_DisplayMode = mode;
			GetView()->SetIndent(KGetImageList()->GetSize().GetWidth());
			break;
		}
	};
}

const KModTagArray& KModManagerModel::GetTags() const
{
	return KModManager::GetTagManager().GetTagList();
}
void KModManagerModel::SetDataVector()
{
	m_Entries = NULL;
	m_DataVector.clear();
	ItemsCleared();
}
void KModManagerModel::SetDataVector(KModEntryArray& array)
{
	SetDataVector();

	m_Entries = &array;
	RefreshItems();
}
void KModManagerModel::RefreshItems()
{
	m_DataVector.clear();
	m_PriortyGroups.clear();
	ItemsCleared();

	if (IsTree())
	{
		m_DataVector.reserve(GetTags().size() + 1);
		std::unordered_map<wxString, std::pair<KMMModelNode*, size_t>> tagsMap;
		size_t noneCount = 0;

		/* Calculate item count for every tag */
		for (KModEntry* modEntry: *m_Entries)
		{
			if (FilterMod(modEntry) && !modEntry->GetTags().empty())
			{
				for (const wxString& tagName: modEntry->GetTags())
				{
					// Insert or get element for this tag and increment its count
					auto& it = tagsMap.emplace(tagName, std::make_pair((KMMModelNode*)NULL, 0)).first;
					it->second.second++;
				}
			}
			else
			{
				noneCount++;
			}
		}

		/* Add tags nodes */
		KxDataViewItem::Vector groupItems;
		for (const KModTag& tag: GetTags())
		{
			const wxString& tagName = tag.GetValue();
			auto& it = tagsMap.find(tagName);
			if (it != tagsMap.end())
			{
				if (it->second.second != 0)
				{
					KMMModelNode& groupNode = m_DataVector.emplace_back(tag);
					groupItems.push_back(MakeItem(groupNode));

					it->second.first = &groupNode;
				}
			}
		}

		// Add "none" node
		KMMModelNode* noneNode = NULL;
		if (noneCount != 0)
		{
			noneNode = &m_DataVector.emplace_back(m_NoneTag);
			groupItems.push_back(MakeItem(noneNode));
		}
		ItemsAdded(groupItems);

		/* Add actual elements */
		for (KModEntry* modEntry: *m_Entries)
		{
			if (FilterMod(modEntry))
			{
				if (!modEntry->GetTags().empty())
				{
					for (const wxString& tagName: modEntry->GetTags())
					{
						KMMModelNode* groupNode = noneNode;
						size_t count = noneCount;

						auto& it = tagsMap.find(tagName);
						if (it != tagsMap.end())
						{
							groupNode = it->second.first;
							count = it->second.second;
						}

						if (groupNode)
						{
							groupNode->GetChildren().reserve(count);

							KMMModelNode& entryNode = groupNode->GetChildren().emplace_back(modEntry);
							entryNode.SetParentNode(*groupNode);
							ItemAdded(MakeItem(groupNode), MakeItem(entryNode));
						}
					}
				}
				else if (noneNode)
				{
					noneNode->GetChildren().reserve(noneCount);

					KMMModelNode& entryNode = noneNode->GetChildren().emplace_back(modEntry);
					ItemAdded(MakeItem(noneNode), MakeItem(entryNode));
				}
			}
		}
	}
	else
	{
		auto& mandatoryLocations = KModManager::Get().GetModEntry_Mandatory();

		// +2 for base game and overwrite folder
		m_PriortyGroups.reserve(m_Entries->size() + 1);
		m_DataVector.reserve(m_Entries->size() + mandatoryLocations.size() + 2 + m_PriortyGroups.capacity());

		// Add base game
		KMMModelNode& baseGameNode = m_DataVector.emplace_back(KModManager::Get().GetModEntry_BaseGame());
		ItemAdded(MakeItem(baseGameNode));

		// Add mandatory locations
		for (KModEntry& entry: mandatoryLocations)
		{
			KMMModelNode& node = m_DataVector.emplace_back(&entry);
			ItemAdded(MakeItem(node));
		}

		// Actual mods
		KModEntry* lastEntry = baseGameNode.GetEntry();
		KMMModelNode* priorityGroupNode = NULL;

		for (KModEntry* currentEntry: *m_Entries)
		{
			if (FilterMod(currentEntry) && currentEntry->IsInstalled())
			{
				// Add priority group
				if (CanShowPriorityGroups())
				{
					if (lastEntry->GetPriorityGroupTag() != currentEntry->GetPriorityGroupTag())
					{
						bool begin = m_PriortyGroups.empty() || m_PriortyGroups.back().IsEnd();

						// If next item is from different group, start new group immediately.
						if (!currentEntry->GetPriorityGroupTag().IsEmpty())
						{
							begin = true;
						}

						KModEntry* anchorEntry = begin ? currentEntry : lastEntry;
						KPriorityGroupEntry& entry = m_PriortyGroups.emplace_back(KPriorityGroupEntry(anchorEntry, begin));
						if (const KModTag* tag = KModManager::GetTagManager().FindModTag(anchorEntry->GetPriorityGroupTag()))
						{
							// Save localized tag name
							entry.SetPriorityGroupTag(tag->GetLabel());
						}

						KMMModelNode& node = m_DataVector.emplace_back(&entry);
						KxDataViewItem item = MakeItem(node);
						ItemAdded(item);
						GetView()->Expand(item);

						if (begin)
						{
							priorityGroupNode = &node;

							// Preallocate this size, a bit excessive, but whatever.
							// I need to rewrite all this anyway.
							node.GetChildren().reserve(m_DataVector.capacity());
						}
						else
						{
							priorityGroupNode = NULL;
						}
					}
					lastEntry = currentEntry;
				}

				KMMModelNode& node = (priorityGroupNode ? priorityGroupNode->GetChildren() : m_DataVector).emplace_back(currentEntry);
				node.SetParentNode(*priorityGroupNode);
				ItemAdded(MakeItem(priorityGroupNode), MakeItem(node));
			}
		}

		// If priority group was opened, but wasn't closed, close it manually
		if (CanShowPriorityGroups())
		{
			if (!m_PriortyGroups.empty() && m_PriortyGroups.back().IsBegin())
			{
				const wxString& groupName = m_PriortyGroups.back().GetPriorityGroupTag();
				KPriorityGroupEntry& entry = m_PriortyGroups.emplace_back(KPriorityGroupEntry(m_Entries->back(), false));
				entry.SetPriorityGroupTag(groupName);

				KMMModelNode& node = m_DataVector.emplace_back(&entry);
				ItemAdded(MakeItem(node));
			}
		}

		// WriteTargetRoot
		KMMModelNode& writeTargetRootNode = m_DataVector.emplace_back(KModManager::Get().GetModEntry_WriteTarget());
		ItemAdded(MakeItem(writeTargetRootNode));
	}
	GetView()->SetFocus();
}
void KModManagerModel::UpdateUI()
{
	//SelectItem(GetView()->GetSelection());
	GetView()->Refresh();
}

void KModManagerModel::CreateSearchColumnsMenu(KxMenu& menu)
{
	auto AddItem = [this, &menu](ColumnID id, bool enable = false) -> KxMenuItem*
	{
		KxDataViewColumn* column = GetView()->GetColumnByID(id);
		if (column)
		{
			wxString sTitle = column->GetTitle();
			if (sTitle.IsEmpty())
			{
				sTitle << '<' << GetView()->GetColumnIndex(column) + 1 << '>';
			}

			enable = enable || m_SearchFilterOptions.GetAttributeBool(std::to_string(id));

			KxMenuItem* menuItem = menu.Add(new KxMenuItem(sTitle, wxEmptyString, wxITEM_CHECK));
			menuItem->Check(enable);
			menuItem->SetClientData(column);
			if (enable)
			{
				m_SearchColumns.push_back(column);
			}

			return menuItem;
		}
		return NULL;
	};

	AddItem(ColumnID::Name, true);
	AddItem(ColumnID::Author);
	AddItem(ColumnID::Version);
	AddItem(ColumnID::Tags);
	AddItem(ColumnID::PackagePath);
	AddItem(ColumnID::Signature);
}
void KModManagerModel::SetSearchColumns(const KxDataViewColumn::Vector& columns)
{
	auto Save = [this](bool value)
	{
		for (const KxDataViewColumn* column: m_SearchColumns)
		{
			m_SearchFilterOptions.SetAttribute(std::to_string(column->GetID()), value);
		}
	};

	Save(false);
	m_SearchColumns = columns;
	Save(true);
}

bool KModManagerModel::FilterMod(const KModEntry* modEntry) const
{
	if (m_SearchColumns.empty() || m_SearchMask.IsEmpty())
	{
		return true;
	}

	bool found = false;
	for (const KxDataViewColumn* column: m_SearchColumns)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				found = KAux::CheckSearchMask(m_SearchMask, modEntry->GetName()) || KAux::CheckSearchMask(m_SearchMask, modEntry->GetID());
				break;
			}
			case ColumnID::Author:
			{
				found = KAux::CheckSearchMask(m_SearchMask, modEntry->GetAuthor());
				break;
			}
			case ColumnID::Version:
			{
				found = KAux::CheckSearchMask(m_SearchMask, modEntry->GetVersion());
				break;
			}
			case ColumnID::Tags:
			{
				found = KAux::CheckSearchMask(m_SearchMask, FormatTagList(modEntry));
				break;
			}
			case ColumnID::PackagePath:
			{
				found = KAux::CheckSearchMask(m_SearchMask, modEntry->GetInstallPackageFile());
				break;
			}
			case ColumnID::Signature:
			{
				found = KAux::CheckSearchMask(m_SearchMask, modEntry->GetSignature());
				break;
			}
		};

		if (found)
		{
			break;
		}
	}
	return found;
}

KxDataViewItem KModManagerModel::MakeItem(const KMMModelNode* node) const
{
	return KxDataViewItem(node);
}
KMMModelNode* KModManagerModel::GetNode(const KxDataViewItem& item) const
{
	return item.GetValuePtr<KMMModelNode>();
}
KxDataViewItem KModManagerModel::GetItemByEntry(const KModEntry* entry) const
{
	if (entry)
	{
		auto FindIn = [this, entry](const KMMLogModelNodeVector& vector, const KMMModelNode*& nodeOut)
		{
			auto it = std::find_if(vector.begin(), vector.end(), [entry](const KMMModelNode& node)
			{
				return node.GetEntry() == entry;
			});

			nodeOut = it != vector.end() ? &*it : NULL;
			return nodeOut != NULL;
		};

		const KMMModelNode* itemNode = NULL;
		if (FindIn(m_DataVector, itemNode))
		{
			return MakeItem(itemNode);
		}

		for (const KMMModelNode& node: m_DataVector)
		{
			if (FindIn(node.GetChildren(), itemNode))
			{
				return MakeItem(itemNode);
			}
		}
	}
	return KxDataViewItem();
}
size_t KModManagerModel::CountItemsInGroup(const KModTag* group) const
{
	for (const KMMModelNode& groupNode: m_DataVector)
	{
		if (groupNode.GetGroup() == group)
		{
			return groupNode.GetChildren().size();
		}
	}
	return 0;
}
