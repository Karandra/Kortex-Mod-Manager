#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/PluginManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/ModTagManager.hpp>
#include <Kortex/ModProvider.hpp>
#include "DisplayModel.h"
#include "Workspace.h"
#include "PriorityGroup.h"
#include "UI/KMainWindow.h"
#include "UI/KImageViewerDialog.h"
#include "Utility/KAux.h"
#include <KxFramework/KxComparator.h>
#include <KxFramework/DataView/KxDataViewMainWindow.h>

namespace
{
	enum ColumnID
	{
		Name,
		Color,
		Bitmap,
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

	using SitesRenderer = KxDataViewImageListRenderer<Kortex::NetworkProviderIDs::MAX_SYSTEM + 1>;
	using SitesValue = typename SitesRenderer::ValueT;
}

namespace Kortex::ModManager
{
	bool DisplayModel::IsSpecialSiteColumn(int column) const
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
	NetworkProviderID DisplayModel::ColumnToSpecialSite(int column) const
	{
		return (NetworkProviderID)(column - ColumnID::Sites - 1);
	}
	wxString DisplayModel::FormatTagList(const IGameMod& entry) const
	{
		return KxString::Join(entry.GetTagStore().GetNames(), wxS("; "));
	}

	void DisplayModel::OnInitControl()
	{
		/* View */
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &DisplayModel::OnSelectItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &DisplayModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_EXPANDED, &DisplayModel::OnExpandCollapseItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_COLLAPSED, &DisplayModel::OnExpandCollapseItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &DisplayModel::OnContextMenu, this);
		GetView()->Bind(KxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, &DisplayModel::OnHeaderContextMenu, this);
		GetView()->Bind(KxEVT_DATAVIEW_COLUMN_SORTED, &DisplayModel::OnColumnSorted, this);
		GetView()->Bind(KxEVT_DATAVIEW_CACHE_HINT, &DisplayModel::OnCacheHint, this);
		GetView()->SetIndent(0);
		EnableDragAndDrop();

		/* Columns */
		KxDataViewColumnFlags defaultFlags = KxDV_COL_RESIZEABLE|KxDV_COL_REORDERABLE|KxDV_COL_SORTABLE;
		KxDataViewColumnFlags defaultFlagsNoSort = (KxDataViewColumnFlags)(defaultFlags & ~KxDV_COL_SORTABLE);
		KxDataViewColumnFlags defaultFlagsNoOrder = (KxDataViewColumnFlags)(defaultFlags & ~KxDV_COL_REORDERABLE);
		KxDataViewColumnFlags defaultFlagsNoSortNoOrder = KxDV_COL_RESIZEABLE;

		{
			auto info = GetView()->AppendColumn<KxDataViewBitmapTextToggleRenderer, KxDataViewTextEditor>(KTr("ModManager.ModList.Name"), ColumnID::Name, KxDATAVIEW_CELL_ACTIVATABLE|KxDATAVIEW_CELL_EDITABLE, 400, defaultFlags);;
			m_NameColumn = info.GetColumn();
		}
		{
			auto info = GetView()->AppendColumn<KxDataViewBitmapRenderer>(KTr("Generic.Image"), ColumnID::Bitmap, KxDATAVIEW_CELL_INERT, m_BitmapSize.GetWidth() + 4, KxDV_COL_REORDERABLE);
			m_BitmapColumn = info.GetColumn();
		}
		{
			KBitmapSize size;
			size.FromSystemIcon();

			GetView()->AppendColumn<KxDataViewNullRenderer>(KTr("Generic.Color"), ColumnID::Color, KxDATAVIEW_CELL_INERT, size.GetWidth(), defaultFlagsNoSort);
		}
		{
			auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Priority"), ColumnID::Priority, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE, defaultFlags);
			m_PriorityColumn = info.GetColumn();
			m_PriorityColumn->SortAscending();
		}

		GetView()->AppendColumn<KxDataViewBitmapTextRenderer, KxDataViewTextEditor>(KTr("ModManager.ModList.Version"), ColumnID::Version, KxDATAVIEW_CELL_EDITABLE, 100, defaultFlags);
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("ModManager.ModList.Author"), ColumnID::Author, KxDATAVIEW_CELL_EDITABLE, 100, defaultFlags);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("ModManager.ModList.Tags"), ColumnID::Tags, KxDATAVIEW_CELL_INERT, 100, defaultFlags);
	
		{
			int spacing = 1;
			int width = (spacing +  16) * (NetworkProviderIDs::MAX_SYSTEM + 1);
			auto info = GetView()->AppendColumn<SitesRenderer>(KTr("ModManager.ModList.Sites"), ColumnID::Sites, KxDATAVIEW_CELL_INERT, width, defaultFlagsNoSort);

			info.GetRenderer()->SetImageList(&IApplication::GetInstance()->GetImageList());
			info.GetRenderer()->SetSpacing(1);
			info.GetRenderer()->SetAlignment(wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
		}

		for (const auto& provider: INetworkManager::GetInstance()->GetProviders())
		{
			int id = ColumnID::Sites + provider->GetID() + 1;
			auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(wxEmptyString, id, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE, defaultFlags);

			info.GetColumn()->SetTitle(provider->GetName());
			info.GetColumn()->SetBitmap(KGetBitmap(provider->GetIcon()));
		}

		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("ModManager.ModList.DateInstall"), ColumnID::DateInstall, KxDATAVIEW_CELL_INERT, 125, defaultFlags);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("ModManager.ModList.DateUninstall"), ColumnID::DateUninstall, KxDATAVIEW_CELL_INERT,  125, defaultFlags);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("ModManager.ModList.ModFolder"), ColumnID::ModFolder, KxDATAVIEW_CELL_INERT, 125, defaultFlags);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("ModManager.ModList.PackagePath"), ColumnID::PackagePath, KxDATAVIEW_CELL_INERT, 125, defaultFlags);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("ModManager.ModList.Signature"), ColumnID::Signature, KxDATAVIEW_CELL_INERT, 125, defaultFlags);

		// UI
		m_PriorityGroupRowHeight = GetView()->GetUniformRowHeight() * 1.2;
		m_PriortyGroupColor = KxUtility::GetThemeColor_Caption(GetView());
	}

	bool DisplayModel::IsListModel() const
	{
		return false;
	}
	bool DisplayModel::IsContainer(const KxDataViewItem& item) const
	{
		if (IsTree())
		{
			if (const DisplayModelNode* node = GetNode(item))
			{
				return node->IsGroup() && node->HasChildren();
			}
			return item.IsTreeRootItem();
		}
		else
		{
			const DisplayModelNode* node = GetNode(item);
			if (node)
			{
				if (IGameMod* entry = node->GetEntry())
				{
					if (IPriorityGroup* priorityGroup = entry->QueryInterface<IPriorityGroup>())
					{
						return priorityGroup->IsBegin();
					}
				}
			}
		}
		return item.IsTreeRootItem();
	}
	bool DisplayModel::HasContainerColumns(const KxDataViewItem& item) const
	{
		return true;
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
			children.reserve(m_DataVector.size());
			for (const DisplayModelNode& node: m_DataVector)
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
			const DisplayModelNode* node = GetNode(item);
			children.reserve(node->GetChildrenCount());

			const IModTag* group = node->GetGroup();
			if (group || node->GetEntry()->QueryInterface<IPriorityGroup>())
			{
				for (const DisplayModelNode& entryNode: node->GetChildren())
				{
					children.push_back(MakeItem(entryNode));
				}
			}
		}
	}
	void DisplayModel::GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		const DisplayModelNode* node = GetNode(item);
		if (IGameMod* entry = node->GetEntry())
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
	void DisplayModel::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		const DisplayModelNode* node = GetNode(item);
		if (const IModTag* group = node->GetGroup())
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = wxString::Format("%s (%zu)", group->GetName(), CountItemsInGroup(group));
					break;
				}
			};
		}
		else if (IGameMod* mod = node->GetEntry())
		{
			IGameModWithImage* withImage = nullptr;
			if (column->GetID() == ColumnID::Bitmap && mod->QueryInterface(withImage))
			{
				value = withImage->HasBitmap() ? withImage->GetBitmap() : KGetBitmap(KIMG_CROSS_WHITE);
				return;
			}

			if (const IPriorityGroup* priorityGroup = mod->QueryInterface<IPriorityGroup>())
			{
				GetValuePriorityGroup(value, item, column, mod, priorityGroup);
			}
			else if (const IFixedGameMod* fixedMod = mod->QueryInterface<IFixedGameMod>())
			{
				GetValueFixedMod(value, item, column, mod);
			}
			else
			{
				GetValueMod(value, item, column, mod);
			}
		}
	}
	void DisplayModel::GetValueMod(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column, const IGameMod* mod) const
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				KxDataViewBitmapTextToggleValue valueData(mod->IsActive(), KxDataViewBitmapTextToggleValue::CheckBox);
				if (mod->GetName() != mod->GetID())
				{
					valueData.SetText(wxString::Format("%s (%s)", mod->GetName(), mod->GetID()));
				}
				else
				{
					valueData.SetText(mod->GetName());
				}

				valueData.SetBitmap(KGetBitmap(mod->GetIcon()));
				value = valueData;
				break;
			}
			case ColumnID::Priority:
			{
				value = mod->GetPriority();
				break;
			}
			case ColumnID::Version:
			{
				const KxVersion& version = mod->GetVersion();
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
				value = mod->GetAuthor();
				break;
			}
			case ColumnID::Tags:
			{
				value = FormatTagList(*mod);
				break;
			}
			case ColumnID::Sites:
			{
				SitesValue::ArrayT list;
				SitesValue::ClearArray(list);
				
				mod->GetProviderStore().Visit([&list](const ModProviderItem& item)
				{
					INetworkProvider* provider = nullptr;
					if (item.TryGetProvider(provider))
					{
						list[provider->GetID()] = provider->GetIcon();
					}
					else
					{
						list[NetworkProviderIDs::MAX_SYSTEM] = INetworkProvider::GetGenericIcon();
					}
					return true;
				});
				value = SitesValue(list);
				break;
			}
			case ColumnID::Sites_TESALLID:
			case ColumnID::Sites_NexusID:
			case ColumnID::Sites_LoversLabID:
			{
				const NetworkProviderID index = (NetworkProviderID)(column->GetID() - ColumnID::Sites - 1);
				const INetworkProvider* provider = INetworkManager::GetInstance()->GetProvider(index);
				if (provider)
				{
					const ModProviderItem* providerItem = mod->GetProviderStore().GetItem(*provider);
					
					NetworkModInfo modInfo;
					if (providerItem && providerItem->TryGetModInfo(modInfo))
					{
						value = modInfo.ToString();
					}
				}
				break;
			}
			case ColumnID::DateInstall:
			{
				value = KAux::FormatDateTime(mod->GetInstallTime());
				break;
			}
			case ColumnID::DateUninstall:
			{
				value = KAux::FormatDateTime(mod->GetUninstallTime());
				break;
			}
			case ColumnID::ModFolder:
			{
				if (mod->IsLinkedMod())
				{
					value = mod->GetModFilesDir();
				}
				else
				{
					value = mod->GetRootDir();
				}
				break;
			}
			case ColumnID::PackagePath:
			{
				value = mod->GetPackageFile();
				break;
			}
			case ColumnID::Signature:
			{
				value = mod->GetSignature();
				break;
			}
		};
	}
	void DisplayModel::GetValueFixedMod(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column, const IGameMod* mod) const
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = KxDataViewBitmapTextToggleValue(true, mod->GetName(), KGetBitmap(mod->GetIcon()), KxDataViewBitmapTextToggleValue::InvalidType);
				break;
			}
			case ColumnID::Priority:
			{
				if (mod->IsActive())
				{
					value = mod->GetPriority();
				}
				break;
			}
			case ColumnID::ModFolder:
			{
				value = mod->GetRootDir();
				break;
			}
		};
	}
	void DisplayModel::GetValuePriorityGroup(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column, const IGameMod* mod, const IPriorityGroup* group) const
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				const IModTag* tag = group->GetTag();
				if (tag)
				{
					bool isBegin = group->IsBegin();
					if (!m_PriorityColumn->IsSortedAscending())
					{
						isBegin = !isBegin;
					}

					wxString name;
					if (isBegin)
					{
						name = tag->GetName();
					}
					else
					{
						name = KxString::Format(wxS("^ %1 ^"), tag->GetName());
					}
					value = KxDataViewBitmapTextToggleValue(true, name, KGetBitmap(mod->GetIcon()), KxDataViewBitmapTextToggleValue::InvalidType);
				}
				break;
			}
		};
	}

	bool DisplayModel::SetValue(const wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column)
	{
		const DisplayModelNode* node = GetNode(item);
		if (IGameMod* mod = node->GetEntry())
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					if (value.CheckType<wxString>())
					{
						mod->SetName(value.As<wxString>());
						IEvent::MakeSend<ModEvent>(Events::ModChanged, *mod);
					}
					else
					{
						mod->SetActive(value.As<bool>());
						IEvent::MakeSend<ModEvent>(Events::ModToggled, *mod);
					}

					mod->Save();
					IModManager::GetInstance()->Save();
					return true;
				}
				case ColumnID::Version:
				{
					wxString newVersion = value.As<wxString>();
					if (newVersion != mod->GetVersion())
					{
						mod->SetVersion(newVersion);
						mod->Save();

						IEvent::MakeSend<ModEvent>(Events::ModChanged, *mod);
						return true;
					}
					return false;
				}
				case ColumnID::Author:
				{
					wxString author = value.As<wxString>();
					if (author != mod->GetAuthor())
					{
						mod->SetAuthor(author);
						mod->Save();

						IEvent::MakeSend<ModEvent>(Events::ModChanged, *mod);
						return true;
					}
					break;
				}
			};
		}
		return false;
	}
	bool DisplayModel::IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		const DisplayModelNode* node = GetNode(item);
		if (const IModTag* group = node->GetGroup())
		{
			return false;
		}
		else if (IGameMod* entry = node->GetEntry())
		{
			if (entry->QueryInterface<IFixedGameMod>())
			{
				return false;
			}

			bool bChangesAllowed = Workspace::GetInstance()->IsChangingModsAllowed();
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
	bool DisplayModel::IsEditorEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		const DisplayModelNode* node = GetNode(item);
		if (IGameMod* entry = node->GetEntry())
		{
			return !entry->QueryInterface<IFixedGameMod>();
		}
		return false;
	}
	bool DisplayModel::GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
	{
		int columnID = column->GetID();
		const DisplayModelNode* node = GetNode(item);

		if (const IModTag* group = node->GetGroup())
		{
			if (GetView()->IsExpanded(item) || columnID == ColumnID::Name)
			{
				attributes.SetHeaderButtonBackgound();
				return true;
			}
		}
		else if (IGameMod* mod = node->GetEntry())
		{
			const IFixedGameMod* fixed = mod->QueryInterface<IFixedGameMod>();
			const IPriorityGroup* priorityGroup = mod->QueryInterface<IPriorityGroup>();

			if (columnID == ColumnID::Name && fixed && !priorityGroup)
			{
				attributes.SetItalic();
			}
			if (columnID == ColumnID::Color && !priorityGroup)
			{
				attributes.SetBackgroundColor(mod->GetColor());
			}
			if (!fixed && (columnID == ColumnID::Name || IsSpecialSiteColumn(columnID)))
			{
				attributes.SetUnderlined(cellState & KxDATAVIEW_CELL_HIGHLIGHTED && column->IsHotTracked());
			}
			if (priorityGroup)
			{
				KxColor color = mod->GetColor();
				if (color.IsOk())
				{
					attributes.SetForegroundColor(color.GetContrastColor(GetView()));
				}
				else
				{
					attributes.SetForegroundColor(m_PriortyGroupColor);
				}

				attributes.SetBackgroundColor(color);
				attributes.SetBold(m_BoldPriorityGroupLabels);
				attributes.SetAlignment(m_PriorityGroupLabelAlignment);
			}
			return !attributes.IsDefault();
		}
		return false;
	}
	bool DisplayModel::GetCellHeight(const KxDataViewItem& item, int& height) const
	{
		if (const DisplayModelNode* node = GetNode(item))
		{
			IGameMod* entry = node->GetEntry();
			if (entry)
			{
				if (entry->QueryInterface<IPriorityGroup>())
				{
					height = m_PriorityGroupRowHeight;
					return true;
				}
				else if (!entry->QueryInterface<IFixedGameMod>())
				{
					if (m_BitmapColumn->IsVisible())
					{
						height = m_BitmapSize.GetHeight() + 2;
						return true;
					}
				}
			}
		}
		return false;
	}
	bool DisplayModel::Compare(const KxDataViewItem& item1, const KxDataViewItem& item2, const KxDataViewColumn* column) const
	{
		const DisplayModelNode* node1 = GetNode(item1);
		const DisplayModelNode* node2 = GetNode(item2);
		if (node1 && node2)
		{
			ColumnID columnID = column ? (ColumnID)column->GetID() : ColumnID::Priority;

			IGameMod* entry1 = node1->GetEntry();
			IGameMod* entry2 = node2->GetEntry();
			if (entry1 && entry2)
			{
				if (IsSpecialSiteColumn(columnID))
				{
					INetworkProvider* provider = INetworkManager::GetInstance()->GetProvider(ColumnToSpecialSite(columnID));
					if (provider)
					{
						ModProviderItem* item1 = entry1->GetProviderStore().GetItem(*provider);
						ModProviderItem* item2 = entry2->GetProviderStore().GetItem(*provider);

						return item1 && item2 && (item1->GetModInfo().GetModID().GetValue() < item2->GetModInfo().GetModID().GetValue());
					}
					return false;
				}

				switch (columnID)
				{
					case ColumnID::Name:
					{
						return KxComparator::IsLess(entry1->GetName(), entry2->GetName());
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
						return KxComparator::IsLess(entry1->GetAuthor(), entry2->GetAuthor());
					}
					case ColumnID::Tags:
					{
						return KxComparator::IsLess(FormatTagList(*entry1), FormatTagList(*entry2));
					}
					case ColumnID::DateInstall:
					{
						return entry1->GetInstallTime() < entry2->GetInstallTime();
					}
					case ColumnID::DateUninstall:
					{
						return entry1->GetUninstallTime() < entry2->GetUninstallTime();
					}
					case ColumnID::ModFolder:
					{
						return KxComparator::IsLess(entry1->GetModFilesDir(), entry2->GetModFilesDir());
					}
					case ColumnID::PackagePath:
					{
						return KxComparator::IsLess(entry1->GetPackageFile(), entry2->GetPackageFile());
					}
					case ColumnID::Signature:
					{
						return KxComparator::IsLess(entry1->GetSignature(), entry2->GetSignature());
					}
				};
			}
		}
		return false;
	}

	void DisplayModel::OnSelectItem(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		KxDataViewColumn* column = event.GetColumn();
		IGameMod* entry = GetModEntry(item);

		if (entry && column)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					if (Kortex::IPluginManager* manager = Kortex::IPluginManager::GetInstance())
					{
						Kortex::PluginManager::Workspace* workspace = Kortex::PluginManager::Workspace::GetInstance();
						wxWindowUpdateLocker lock(workspace);

						workspace->HighlightPlugin();
						for (auto& pluginEntry: manager->GetPlugins())
						{
							if (pluginEntry->GetOwningMod() == entry)
							{
								workspace->HighlightPlugin(pluginEntry.get());
							}
						}
					}
					break;
				}
			};
		};

		Workspace::GetInstance()->ProcessSelection(entry);
	}
	void DisplayModel::OnActivateItem(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		KxDataViewColumn* column = event.GetColumn();
		const DisplayModelNode* node = GetNode(item);
		IGameMod* entry = node->GetEntry();

		if (node->IsGroup() || node->GetEntry()->QueryInterface<IPriorityGroup>())
		{
			GetView()->ToggleItemExpanded(item);
		}
		else if (column && entry)
		{
			int columnID = column->GetID();

			// If this is a site open click
			NetworkProviderID providerID = ColumnToSpecialSite(columnID);
			if (IsSpecialSiteColumn(columnID))
			{
				const ModProviderStore& store = entry->GetProviderStore();
				const INetworkProvider* provider = INetworkManager::GetInstance()->GetProvider(providerID);
				if (provider)
				{
					if (const ModProviderItem* providerItem = store.GetItem(*provider))
					{
						KAux::AskOpenURL(providerItem->GetURL(), GetViewTLW());
					}
					else if (!store.IsEmpty())
					{
						KAux::AskOpenURL(store.GetModNamedURLs(), GetViewTLW());
					}
				}
				return;
			}

			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					break;
				}
				case ColumnID::Bitmap:
				{
					IGameModWithImage* withImage = nullptr;
					if (entry->QueryInterface(withImage) && withImage->HasBitmap())
					{
						KImageViewerDialog dialog(GetViewTLW(), entry->GetName());

						KImageViewerEvent imageEvent(wxEVT_NULL, entry->GetImageFile());
						dialog.Navigate(imageEvent);
						dialog.ShowModal();
					}
					break;
				}
				case ColumnID::Sites:
				{
					const ModProviderStore& store = entry->GetProviderStore();
					if (!store.IsEmpty())
					{
						KAux::AskOpenURL(store.GetModNamedURLs(), GetViewTLW());
					}
					break;
				}
				default:
				{
					GetView()->EditItem(item, column);
					break;
				}
			};
		}
	}
	void DisplayModel::OnExpandCollapseItem(KxDataViewEvent& event)
	{
		if (const DisplayModelNode* node = GetNode(event.GetItem()))
		{
			IPriorityGroup* priorityGroup = nullptr;
			if (node->IsEntry() && node->GetEntry()->QueryInterface(priorityGroup))
			{
				if (IModTag* tag = priorityGroup->GetTag())
				{
					tag->SetExpanded(event.GetEventType() == KxEVT_DATAVIEW_ITEM_EXPANDED);
				}
			}
		}
	}
	void DisplayModel::OnContextMenu(KxDataViewEvent& event)
	{
		const DisplayModelNode* node = GetNode(event.GetItem());
		KxDataViewColumn* column = event.GetColumn();
		IGameMod* entry = nullptr;

		if (node && column)
		{
			entry = node->GetEntry();

			if (const IModTag* group = node->GetGroup())
			{
				Workspace::GetInstance()->ShowViewContextMenu(group);
				return;
			}
		}
		Workspace::GetInstance()->ShowViewContextMenu(node && column ? entry : nullptr);
	}
	void DisplayModel::OnHeaderContextMenu(KxDataViewEvent& event)
	{
		KxMenu menu;
		if (GetView()->CreateColumnSelectionMenu(menu))
		{
			GetView()->OnColumnSelectionMenu(menu);
			UpdateRowHeight();
		}
	}
	void DisplayModel::OnColumnSorted(KxDataViewEvent& event)
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
	void DisplayModel::OnCacheHint(KxDataViewEvent& event)
	{
		if (m_BitmapColumn->IsVisible())
		{
			for (size_t row = event.GetCacheHintFrom(); row <= event.GetCacheHintTo(); row++)
			{
				KxDataViewItem item = GetView()->GetMainWindow()->GetItemByRow(row);
				if (const DisplayModelNode* node = GetNode(item))
				{
					IGameMod* entry = node->GetEntry();
					IGameModWithImage* withImage = nullptr;
					if (entry && entry->QueryInterface(withImage))
					{
						if (!withImage->IsNoBitmap() && !withImage->HasBitmap())
						{
							withImage->SetBitmap(CreateModThumbnail(*entry));
							withImage->SetNoBitmap(!withImage->HasBitmap());
						}
					}
				}
			}
		}
	}

	wxBitmap DisplayModel::CreateModThumbnail(const IGameMod& entry) const
	{
		const int magrinX = 2;
		const int magrinY = 2;
		switch (Workspace::GetInstance()->GetImageResizeMode())
		{
			case Workspace::ImageResizeMode::Scale:
			{
				return m_BitmapSize.ScaleBitmapAspect(wxBitmap(entry.GetImageFile(), wxBITMAP_TYPE_ANY), magrinX, magrinY);
			}
			case Workspace::ImageResizeMode::Stretch:
			{
				return m_BitmapSize.ScaleBitmapStretch(wxBitmap(entry.GetImageFile(), wxBITMAP_TYPE_ANY), magrinX, magrinY);
			}
			case Workspace::ImageResizeMode::Fill:
			{
				wxImage image = wxImage(entry.GetImageFile(), wxBITMAP_TYPE_ANY);
				image = KAux::ScaleImageAspect(image, m_BitmapSize.GetWidth());

				if (image.GetHeight() >= m_BitmapSize.GetHeight())
				{
					image.Resize(wxSize(image.GetWidth(), m_BitmapSize.GetHeight()), wxPoint(0, 0));
				}
				return wxBitmap(image, 32);
			}
		};
		return wxNullBitmap;
	}
	bool DisplayModel::OnDragItems(KxDataViewEventDND& event)
	{
		if (CanDragDropNow())
		{
			KxDataViewItem::Vector selected;
			if (GetView()->GetSelections(selected) > 0)
			{
				std::unique_ptr<DisplayModelDNDObject> dataObject;
				for (const auto& item: selected)
				{
					const DisplayModelNode* node = GetNode(item);
					if (IGameMod* entry = node->GetEntry())
					{
						if (!entry->QueryInterface<IFixedGameMod>())
						{
							if (!dataObject)
							{
								dataObject = std::make_unique<DisplayModelDNDObject>(selected.size());
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
	bool DisplayModel::OnDropItems(KxDataViewEventDND& event)
	{
		const DisplayModelNode* node = GetNode(event.GetItem());
		if (node)
		{
			IGameMod* thisEntry = node->GetEntry();
			if (thisEntry && HasDragDropDataObject())
			{
				const IGameMod::RefVector& toMove = GetDragDropDataObject()->GetEntries();
				IPriorityGroup* priorityGroup = thisEntry->QueryInterface<IPriorityGroup>();
				if (priorityGroup)
				{
					thisEntry = &priorityGroup->GetBaseMod();
				}

				// Move and refresh
				bool moved = false;
				if (toMove.size() > 1)
				{
					moved = IModManager::GetInstance()->MoveModsAfter(toMove, *thisEntry);
				}
				else
				{
					moved = IModManager::GetInstance()->MoveModsBefore(toMove, *thisEntry);
				}

				if (moved)
				{
					// If items dragged over priority group, assign them to it
					if (priorityGroup)
					{
						for (IGameMod* entry: toMove)
						{
							entry->SetPriorityGroupTag(thisEntry->GetPriorityGroupTag());
						}
					}

					// Reload control data
					RefreshItems();

					// Select moved items and Event-select the first one
					for (IGameMod* entry: toMove)
					{
						entry->Save();
						GetView()->Select(GetItemByEntry(entry));
					}
					SelectItem(GetItemByEntry(toMove.front()));

					ModEvent(Events::ModsReordered, toMove).Send();
					return true;
				}
			}
		}

		return false;
	}
	bool DisplayModel::CanDragDropNow() const
	{
		if (Workspace::GetInstance()->IsMovingModsAllowed())
		{
			if (KxDataViewColumn* column = GetView()->GetSortingColumn())
			{
				return column->GetID() == ColumnID::Priority && column->IsSortedAscending();
			}
			return true;
		}
		return false;
	}

	DisplayModel::DisplayModel()
		//:m_SearchFilterOptions(Workspace::GetInstance(), "SearchFilter")
	{
		m_NoneTag = IModTagManager::GetInstance()->NewTag();
		m_NoneTag->SetName(KAux::MakeNoneLabel());

		m_BitmapSize.FromHeight(80, KBitmapSize::r16_9);
		SetDataViewFlags(KxDataViewCtrl::DefaultStyle|KxDV_MULTIPLE_SELECTION|KxDV_NO_TIMEOUT_EDIT|KxDV_VERT_RULES);
	}

	void DisplayModel::SetDisplayMode(DisplayModelType mode)
	{
		switch (mode)
		{
			case DisplayModelType::Connector:
			{
				m_DisplayMode = mode;
				break;
			}
			case DisplayModelType::Manager:
			{
				m_DisplayMode = mode;
				break;
			}
		};
	}

	DisplayModel::PriorityGroupLabelAlignment DisplayModel::GetPriorityGroupLabelAlignment() const
	{
		return m_PriorityGroupLabelAlignmentType;
	}
	void DisplayModel::SetPriorityGroupLabelAlignment(PriorityGroupLabelAlignment value)
	{
		m_PriorityGroupLabelAlignmentType = value;
		switch (value)
		{
			case PriorityGroupLabelAlignment::Left:
			{
				m_PriorityGroupLabelAlignment = wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL;
				break;
			}
			case PriorityGroupLabelAlignment::Right:
			{
				m_PriorityGroupLabelAlignment = wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL;
				break;
			}
			default:
			{
				m_PriorityGroupLabelAlignment = wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL;
				break;
			}
		};
	}

	void DisplayModel::SetDataVector()
	{
		m_Entries = nullptr;
		m_DataVector.clear();
		ItemsCleared();
	}
	void DisplayModel::SetDataVector(BasicGameMod::Vector& array)
	{
		SetDataVector();

		m_Entries = &array;
		RefreshItems();
	}
	void DisplayModel::RefreshItems()
	{
		IModTagManager* tagManager = IModTagManager::GetInstance();
		m_DataVector.clear();
		m_PriortyGroups.clear();
		ItemsCleared();

		if (IsTree())
		{
			m_DataVector.reserve(tagManager->GetTagsCount() + 1);
			std::unordered_map<wxString, std::pair<DisplayModelNode*, size_t>> tagsMap;
			size_t noneCount = 0;

			/* Calculate item count for every tag */
			for (auto& modEntry: *m_Entries)
			{
				if (FilterMod(*modEntry))
				{
					modEntry->GetTagStore().Visit([&tagsMap](const IModTag& tag)
					{
						// Insert or get element for this tag and increment its count
						auto& it = tagsMap.emplace(tag.GetID(), std::make_pair((DisplayModelNode*)nullptr, 0)).first;
						it->second.second++;
						return true;
					});
				}
				else
				{
					noneCount++;
				}
			}

			/* Add tags nodes */
			KxDataViewItem::Vector groupItems;
			for (const auto& tag: tagManager->GetTags())
			{
				const wxString& tagID = tag->GetID();
				auto& it = tagsMap.find(tagID);
				if (it != tagsMap.end())
				{
					if (it->second.second != 0)
					{
						DisplayModelNode& groupNode = m_DataVector.emplace_back(*tag);
						groupItems.push_back(MakeItem(groupNode));

						it->second.first = &groupNode;
					}
				}
			}

			// Add "none" node
			DisplayModelNode* noneNode = nullptr;
			if (noneCount != 0)
			{
				noneNode = &m_DataVector.emplace_back(*m_NoneTag);
				groupItems.push_back(MakeItem(noneNode));
			}
			ItemsAdded(groupItems);

			/* Add actual elements */
			for (auto& modEntry: *m_Entries)
			{
				if (FilterMod(*modEntry))
				{
					const ModTagStore& tagStore = modEntry->GetTagStore();
					if (!tagStore.IsEmpty())
					{
						tagStore.Visit([this, &tagsMap, &modEntry, noneNode, noneCount](const IModTag& tag)
						{
							DisplayModelNode* groupNode = noneNode;
							size_t count = noneCount;

							auto& it = tagsMap.find(tag.GetID());
							if (it != tagsMap.end())
							{
								groupNode = it->second.first;
								count = it->second.second;
							}

							if (groupNode)
							{
								groupNode->GetChildren().reserve(count);

								DisplayModelNode& entryNode = groupNode->GetChildren().emplace_back(*modEntry);
								entryNode.SetParentNode(*groupNode);
								ItemAdded(MakeItem(groupNode), MakeItem(entryNode));
							}

							return true;
						});
					}
					else if (noneNode)
					{
						noneNode->GetChildren().reserve(noneCount);

						DisplayModelNode& entryNode = noneNode->GetChildren().emplace_back(*modEntry);
						ItemAdded(MakeItem(noneNode), MakeItem(entryNode));
					}
				}
			}
		}
		else
		{
			auto& mandatoryLocations = IModManager::GetInstance()->GetMandatoryMods();

			// +2 for base game and overwrite folder
			m_PriortyGroups.reserve(m_Entries->size() + 1);
			m_DataVector.reserve(m_Entries->size() + mandatoryLocations.size() + 2 + m_PriortyGroups.capacity());

			// Add base game
			DisplayModelNode& baseGameNode = m_DataVector.emplace_back(IModManager::GetInstance()->GetBaseGame());
			ItemAdded(MakeItem(baseGameNode));

			// Add mandatory locations
			for (IGameMod* entry: mandatoryLocations)
			{
				DisplayModelNode& node = m_DataVector.emplace_back(*entry);
				ItemAdded(MakeItem(node));
			}

			// Actual mods
			IGameMod* lastEntry = baseGameNode.GetEntry();
			DisplayModelNode* priorityGroupNode = nullptr;

			for (auto& currentEntry: *m_Entries)
			{
				if (FilterMod(*currentEntry))
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

							IGameMod* anchorEntry = begin ? &*currentEntry : lastEntry;
							if (begin)
							{
								PriorityGroup& entry = m_PriortyGroups.emplace_back(*anchorEntry, begin);
								IModTag* tag = tagManager->FindTagByID(anchorEntry->GetPriorityGroupTag());
								if (tag)
								{
									entry.SetTag(tag);
								}

								DisplayModelNode& node = m_DataVector.emplace_back(entry);
								KxDataViewItem item = MakeItem(node);
								ItemAdded(item);

								priorityGroupNode = &node;
								GetView()->SetItemExpanded(item, tag && tag->IsExpanded());

								// Preallocate this size, a bit excessive, but whatever. I need to rewrite all this anyway.
								node.GetChildren().reserve(m_DataVector.capacity());
							}
							else
							{
								priorityGroupNode = nullptr;
							}
						}
						lastEntry = &*currentEntry;
					}

					DisplayModelNode& node = (priorityGroupNode ? priorityGroupNode->GetChildren() : m_DataVector).emplace_back(*currentEntry);
					node.SetParentNode(*priorityGroupNode);
					ItemAdded(MakeItem(priorityGroupNode), MakeItem(node));
				}
			}

			// If priority group was opened, but wasn't closed, close it manually
			#if 0
			if (CanShowPriorityGroups())
			{
				if (!m_PriortyGroups.empty() && m_PriortyGroups.back().IsBegin())
				{
					IModTag* lastTag = m_PriortyGroups.back().GetTag();
					PriorityGroup& entry = m_PriortyGroups.emplace_back(*m_Entries->back(), false);
					entry.SetTag(lastTag);

					DisplayModelNode& node = m_DataVector.emplace_back(entry);
					ItemAdded(MakeItem(node));
				}
			}
			#endif

			// WriteTargetRoot
			DisplayModelNode& writeTargetRootNode = m_DataVector.emplace_back(IModManager::GetInstance()->GetOverwrites());
			ItemAdded(MakeItem(writeTargetRootNode));
		}
		GetView()->SetFocus();
	}
	void DisplayModel::UpdateUI()
	{
		GetView()->Refresh();
	}
	void DisplayModel::UpdateRowHeight()
	{
		KxDataViewColumn* column = GetView()->GetColumnByID(ColumnID::Bitmap);
		if (column)
		{
			auto EnableFlag = [this](bool enable)
			{
				GetView()->SetWindowStyle(KxUtility::ModFlag(GetView()->GetWindowStyle(), KxDV_MODEL_ROW_HEIGHT, enable));
			};
			EnableFlag(true);
			UpdateUI();
		}
	}

	void DisplayModel::CreateSearchColumnsMenu(KxMenu& menu)
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

				//enable = enable || m_SearchFilterOptions.GetAttributeBool(std::to_string(id));

				KxMenuItem* menuItem = menu.Add(new KxMenuItem(sTitle, wxEmptyString, wxITEM_CHECK));
				menuItem->Check(enable);
				menuItem->SetClientData(column);
				if (enable)
				{
					m_SearchColumns.push_back(column);
				}

				return menuItem;
			}
			return nullptr;
		};

		AddItem(ColumnID::Name, true);
		AddItem(ColumnID::Author);
		AddItem(ColumnID::Version);
		AddItem(ColumnID::Tags);
		AddItem(ColumnID::PackagePath);
		AddItem(ColumnID::Signature);
	}
	void DisplayModel::SetSearchColumns(const KxDataViewColumn::Vector& columns)
	{
		auto Save = [this](bool value)
		{
			for (const KxDataViewColumn* column: m_SearchColumns)
			{
				//m_SearchFilterOptions.SetAttribute(std::to_string(column->GetID()), value);
			}
		};

		Save(false);
		m_SearchColumns = columns;
		Save(true);
	}
	bool DisplayModel::FilterMod(const IGameMod& modEntry) const
	{
		if (!modEntry.IsInstalled() && !ShouldShowNotInstalledMods())
		{
			return false;
		}
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
					found = KAux::CheckSearchMask(m_SearchMask, modEntry.GetName()) || KAux::CheckSearchMask(m_SearchMask, modEntry.GetID());
					break;
				}
				case ColumnID::Author:
				{
					found = KAux::CheckSearchMask(m_SearchMask, modEntry.GetAuthor());
					break;
				}
				case ColumnID::Version:
				{
					found = KAux::CheckSearchMask(m_SearchMask, modEntry.GetVersion());
					break;
				}
				case ColumnID::Tags:
				{
					found = KAux::CheckSearchMask(m_SearchMask, FormatTagList(modEntry));
					break;
				}
				case ColumnID::PackagePath:
				{
					found = KAux::CheckSearchMask(m_SearchMask, modEntry.GetPackageFile());
					break;
				}
				case ColumnID::Signature:
				{
					found = KAux::CheckSearchMask(m_SearchMask, modEntry.GetSignature());
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

	KxDataViewItem DisplayModel::MakeItem(const DisplayModelNode* node) const
	{
		return KxDataViewItem(node);
	}
	DisplayModelNode* DisplayModel::GetNode(const KxDataViewItem& item) const
	{
		return item.GetValuePtr<DisplayModelNode>();
	}
	KxDataViewItem DisplayModel::GetItemByEntry(const IGameMod* entry) const
	{
		if (entry)
		{
			auto FindIn = [this, entry](const DisplayModelNode::Vector& vector, const DisplayModelNode*& nodeOut)
			{
				auto it = std::find_if(vector.begin(), vector.end(), [entry](const DisplayModelNode& node)
				{
					return node.GetEntry() == entry;
				});

				nodeOut = it != vector.end() ? &*it : nullptr;
				return nodeOut != nullptr;
			};

			const DisplayModelNode* itemNode = nullptr;
			if (FindIn(m_DataVector, itemNode))
			{
				return MakeItem(itemNode);
			}

			for (const DisplayModelNode& node: m_DataVector)
			{
				if (FindIn(node.GetChildren(), itemNode))
				{
					return MakeItem(itemNode);
				}
			}
		}
		return KxDataViewItem();
	}
	size_t DisplayModel::CountItemsInGroup(const IModTag* group) const
	{
		for (const DisplayModelNode& groupNode: m_DataVector)
		{
			if (groupNode.GetGroup() == group)
			{
				return groupNode.GetChildren().size();
			}
		}
		return 0;
	}
}
