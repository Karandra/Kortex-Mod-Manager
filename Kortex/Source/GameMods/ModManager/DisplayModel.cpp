#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/PluginManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/ModTagManager.hpp>
#include "DisplayModel.h"
#include "DisplayModelDND.h"
#include "Workspace.h"
#include "PriorityGroup.h"
#include "UI/KMainWindow.h"
#include "UI/KImageViewerDialog.h"
#include "Utility/KAux.h"
#include <KxFramework/KxComparator.h>

namespace Kortex::ModManager
{
	wxString DisplayModel::FormatTagList(const IGameMod& entry)
	{
		return KxString::Join(entry.GetTagStore().GetNames(), wxS("; "));
	}

	void DisplayModel::CreateView(wxWindow* parent, wxSizer* sizer)
	{
		using namespace KxDataView2;

		// View
		KxDataView2::View* view = new KxDataView2::View(parent, KxID_NONE, CtrlStyle::MultipleSelection|CtrlStyle::VerticalRules|CtrlStyle::CellFocus);
		view->AssignModel(this);
		if (sizer)
		{
			sizer->Add(view, 1, wxEXPAND);
		}

		// Columns
		ColumnStyle columnStyleDefault = ColumnStyle::Size|ColumnStyle::Move|ColumnStyle::Sort;
		ColumnStyle columnStyleNoSort = ColumnStyle::Size|ColumnStyle::Move;
		ColumnStyle columnStyleNoMove = ColumnStyle::Size|ColumnStyle::Sort;
		ColumnStyle columnStyleResizeOnly = ColumnStyle::Size;

		{
			auto [column, r, e] = view->AppendColumn<BitmapTextToggleRenderer, TextEditor>(KTr("ModManager.ModList.Name"), ColumnID::Name, {}, columnStyleDefault);
			m_NameColumn = &column;
		}
		{
			auto [column, r, editor] = view->AppendColumn<TextRenderer, TextEditor>(KTr("Generic.Priority"), ColumnID::Priority, {}, columnStyleDefault);
			m_PriorityColumn = &column;
			m_PriorityColumn->SortAscending();

			editor.NewValidator<wxIntegerValidator<intptr_t>>();
		}
		{
			view->AppendColumn<BitmapTextRenderer, TextEditor>(KTr("ModManager.ModList.Version"), ColumnID::Version, {}, columnStyleDefault);
			view->AppendColumn<TextRenderer, TextEditor>(KTr("ModManager.ModList.Author"), ColumnID::Author, {}, columnStyleDefault);
			view->AppendColumn<TextRenderer>(KTr("ModManager.ModList.Tags"), ColumnID::Tags, {}, columnStyleDefault);
		}
		{
			auto [column, renderer] = view->AppendColumn<BitmapListRenderer>(KTr("ModManager.ModList.Sources"), ColumnID::ModSource, {}, columnStyleDefault);
			renderer.SetAlignment(wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
		}
		{
			view->AppendColumn<TextRenderer>(KTr("ModManager.ModList.DateInstall"), ColumnID::DateInstall, {}, columnStyleDefault);
			view->AppendColumn<TextRenderer>(KTr("ModManager.ModList.DateUninstall"), ColumnID::DateUninstall, {}, columnStyleDefault);
			view->AppendColumn<TextRenderer>(KTr("ModManager.ModList.ModFolder"), ColumnID::ModFolder, {}, columnStyleDefault);
			view->AppendColumn<TextRenderer>(KTr("ModManager.ModList.PackagePath"), ColumnID::PackagePath, {}, columnStyleDefault);
			view->AppendColumn<TextRenderer>(KTr("ModManager.ModList.Signature"), ColumnID::Signature, {}, columnStyleDefault);
		}

		// UI
		m_PriorityGroupRowHeight = view->GetUniformRowHeight() * 1.1;
		m_PriortyGroupColor = KxUtility::GetThemeColor_Caption(GetView());
		
		// Events
		view->Bind(KxDataView2::EVENT_ITEM_SELECTED, &DisplayModel::OnSelectItem, this);
		view->Bind(KxDataView2::EVENT_ITEM_ACTIVATED, &DisplayModel::OnActivateItem, this);
		view->Bind(KxDataView2::EVENT_ITEM_EXPANDED, &DisplayModel::OnExpandCollapseItem, this);
		view->Bind(KxDataView2::EVENT_ITEM_COLLAPSED, &DisplayModel::OnExpandCollapseItem, this);
		view->Bind(KxDataView2::EVENT_ITEM_CONTEXT_MENU, &DisplayModel::OnContextMenu, this);
		view->Bind(KxDataView2::EVENT_COLUMN_HEADER_RCLICK, &DisplayModel::OnHeaderContextMenu, this);

		// Drag-and-drop
		using KxDataView2::DNDOpType;
		view->EnableDND(std::make_unique<DisplayModelDNDObject>(), DNDOpType::Drag|DNDOpType::Drop);

		view->Bind(KxDataView2::EVENT_ITEM_DRAG, &DisplayModel::OnDragItems, this);
		view->Bind(KxDataView2::EVENT_ITEM_DROP, &DisplayModel::OnDropItems, this);
		view->Bind(KxDataView2::EVENT_ITEM_DROP_POSSIBLE, &DisplayModel::OnDropItemsPossible, this);
	}
	void DisplayModel::ClearView()
	{
		if (KxDataView2::View* view = GetView())
		{
			view->UnselectAll();
			view->GetRootNode().DetachAllChildren();
		}

		m_TagNodes.clear();
		m_ModNodes.clear();
		m_PriortyGroups.clear();
	}
	void DisplayModel::LoadView()
	{
		wxWindowUpdateLocker lock(GetView());
		ClearView();

		KxDataView2::Node& rootNode = GetView()->GetRootNode();
		IModTagManager* tagManager = IModTagManager::GetInstance();
		IModManager* modManager = IModManager::GetInstance();

		if (m_DisplayMode == DisplayModelType::Connector)
		{
			// Add base game
			m_ModNodes.emplace_back(modManager->GetBaseGame());

			// Add mandatory locations
			for (IGameMod* mod: modManager->GetMandatoryMods())
			{
				m_ModNodes.emplace_back(*mod);
			}

			// Add regular mods
			IGameMod* lastMod = &modManager->GetBaseGame();
			DisplayModelModNode* lastPriorityGroupNode = nullptr;

			for (auto& currentMod: modManager->GetMods())
			{
				if (FilterMod(*currentMod))
				{
					// Add priority group
					if (CanShowPriorityGroups())
					{
						if (lastMod->GetPriorityGroupTag() != currentMod->GetPriorityGroupTag())
						{
							bool begin = m_PriortyGroups.empty() || m_PriortyGroups.back()->IsEnd();

							// If next item is from different group, start new group immediately.
							if (!currentMod->GetPriorityGroupTag().IsEmpty())
							{
								begin = true;
							}
							if (!begin)
							{
								lastPriorityGroupNode = nullptr;
							}

							IGameMod* anchorMod = begin ? currentMod.get() : lastMod;
							if (begin)
							{
								PriorityGroup& priorityGroup = *m_PriortyGroups.emplace_back(std::make_unique<PriorityGroup>(*anchorMod, begin));
								IModTag* tag = tagManager->FindTagByID(anchorMod->GetPriorityGroupTag());
								if (tag)
								{
									priorityGroup.SetTag(tag);
								}

								lastPriorityGroupNode = &m_ModNodes.emplace_back(priorityGroup);
							}
						}
						lastMod = currentMod.get();

						if (lastPriorityGroupNode)
						{
							lastPriorityGroupNode->AddModNode(*currentMod);
						}
						else
						{
							m_ModNodes.emplace_back(*currentMod);
						}
					}
					else
					{
						m_ModNodes.emplace_back(*currentMod);
					}
				}
			}

			// Write target
			m_ModNodes.emplace_back(IModManager::GetInstance()->GetWriteTarget());

			// Attach nodes
			for (DisplayModelModNode& modNode: m_ModNodes)
			{
				rootNode.AttachChild(modNode);
				modNode.OnAttachNode();

				// Expand priority groups according to saved state
				const PriorityGroup* priorityGroup = nullptr;
				if (modNode.GetMod().QueryInterface(priorityGroup) && priorityGroup->GetTag())
				{
					modNode.SetExpanded(priorityGroup->GetTag()->IsExpanded());
				}
			}
		}

		// Tree-like representation
		if (m_DisplayMode == DisplayModelType::Manager)
		{
			// Add node for "none" pseudo-tag
			DisplayModelTagNode& noneTagNode = m_TagNodes.insert_or_assign(wxEmptyString, *m_NoneTag).first->second;

			// Add nodes for real tags
			for (auto& mod: modManager->GetMods())
			{
				if (FilterMod(*mod))
				{
					ModTagStore& tagStore = mod->GetTagStore();
					if (!tagStore.IsEmpty())
					{
						tagStore.Visit([this, &mod](IModTag& tag)
						{
							auto [it, inserted] = m_TagNodes.try_emplace(tag.GetID(), tag);
							it->second.AddModNode(*mod);
							return true;
						});
					}
					else
					{
						// None tag is always first
						noneTagNode.AddModNode(*mod);
					}
				}
			}

			// Attach nodes
			for (auto& [id, tagNode]: m_TagNodes)
			{
				if (tagNode.HasMods())
				{
					rootNode.AttachChild(tagNode);
					tagNode.OnAttachNode();
				}
			}
		}

		GetView()->ItemsChanged();
	}
	void DisplayModel::UpdateUI()
	{
		if (KxDataView2::View* view = GetView())
		{
			view->Refresh();
		}
	}

	DisplayModelModNode* DisplayModel::ModToNode(const IGameMod& mod)
	{
		for (DisplayModelModNode& node: m_ModNodes)
		{
			if (&node.GetMod() == &mod)
			{
				return &node;
			}
			for (KxDataView2::Node* node: node.GetChildren())
			{
				DisplayModelModNode* modNode = nullptr;
				if (node->QueryInterface(modNode) && &modNode->GetMod() == &mod)
				{
					return modNode;
				}
			}
		}
		return nullptr;
	}

	void DisplayModel::OnSelectItem(KxDataView2::Event& event)
	{
		IGameMod* mod = nullptr;
		DisplayModelModNode* modNode = nullptr;
		if (event.GetNode() && event.GetNode()->QueryInterface(modNode))
		{
			KxDataView2::Column* column = event.GetColumn();
			mod = &modNode->GetMod();

			switch (column->GetID<ColumnID>())
			{
				case ColumnID::Name:
				{
					if (IPluginManager* manager = IPluginManager::GetInstance())
					{
						PluginManager::Workspace* workspace = PluginManager::Workspace::GetInstance();
						wxWindowUpdateLocker lock(workspace);

						workspace->HighlightPlugin();
						for (auto& pluginEntry: manager->GetPlugins())
						{
							if (pluginEntry->GetOwningMod() == mod)
							{
								workspace->HighlightPlugin(pluginEntry.get());
							}
						}
					}
					break;
				}
			};
		}
		Workspace::GetInstance()->ProcessSelection(mod);
	}
	void DisplayModel::OnActivateItem(KxDataView2::Event& event)
	{
		DisplayModelModNode* modNode = nullptr;
		if (event.GetNode()->QueryInterface(modNode))
		{
			KxDataView2::Column* column = event.GetColumn();

			if (modNode->GetMod().QueryInterface<PriorityGroup>())
			{
				event.GetNode()->ToggleExpanded();
			}
			else
			{
				switch (column->GetID<ColumnID>())
				{
					case ColumnID::ModSource:
					{
						const ModSourceStore& store = modNode->GetMod().GetModSourceStore();
						if (!store.IsEmpty())
						{
							KAux::AskOpenURL(store.GetLabeledModURIs(), GetView());
						}
						break;
					}
				};
			}
		}
	}
	void DisplayModel::OnExpandCollapseItem(KxDataView2::Event& event)
	{
		DisplayModelModNode* modNode = nullptr;
		if (event.GetNode()->QueryInterface(modNode))
		{
			PriorityGroup* priorityGroup = nullptr;
			if (modNode->GetMod().QueryInterface(priorityGroup))
			{
				if (IModTag* tag = priorityGroup->GetTag())
				{
					tag->SetExpanded(event.GetEventType() == KxDataView2::EVENT_ITEM_EXPANDED);
				}
			}
		}
	}
	void DisplayModel::OnContextMenu(KxDataView2::Event& event)
	{
		if (event.GetColumn())
		{
			KxDataView2::Node::Vector selectedNodes = GetView()->GetSelections();
			if (!selectedNodes.empty())
			{
				IGameMod* currentMod = nullptr;
				IGameMod::RefVector selectedMods;

				for (KxDataView2::Node* node: selectedNodes)
				{
					DisplayModelModNode* modNode = nullptr;
					if (node->QueryInterface(modNode))
					{
						selectedMods.push_back(&modNode->GetMod());
						if (node == event.GetNode())
						{
							currentMod = &modNode->GetMod();
						}
					}
				}

				if (!selectedMods.empty())
				{
					Workspace::GetInstance()->OnModsContextMenu(selectedMods, currentMod);
					return;
				}
			}
		}

		// Menu for empty out of items area selection
		Workspace::GetInstance()->OnModsContextMenu({}, nullptr);
	}
	void DisplayModel::OnHeaderContextMenu(KxDataView2::Event& event)
	{
		KxMenu menu;
		if (GetView()->CreateColumnSelectionMenu(menu))
		{
			GetView()->OnColumnSelectionMenu(menu);
		}
	}

	bool DisplayModel::CanStartDragOperation() const
	{
		if (Workspace::GetInstance()->IsMovingModsAllowed())
		{
			if (KxDataView2::Column* column = GetView()->GetSortingColumn())
			{
				return column->GetID() == ColumnID::Priority && column->IsSortedAscending();
			}
			return true;
		}
		return false;
	}
	IGameMod* DisplayModel::TestDNDNode(KxDataView2::Node& node, bool allowPriorityGroup) const
	{
		DisplayModelModNode* modNode = nullptr;
		if (node.QueryInterface(modNode))
		{
			IGameMod& mod = modNode->GetMod();
			if (!mod.QueryInterface<FixedGameMod>() && (allowPriorityGroup || !mod.QueryInterface<PriorityGroup>()))
			{
				return &mod;
			}
		}
		return nullptr;
	}

	void DisplayModel::OnDragItems(KxDataView2::EventDND& event)
	{
		DisplayModelDNDObject* dataObject = event.GetDragObject<DisplayModelDNDObject>(DisplayModelDNDObject::GetFormat());
		if (dataObject && CanStartDragOperation())
		{
			dataObject->Clear();

			KxDataView2::Node::Vector selected;
			if (GetView()->GetSelections(selected) > 0)
			{
				for (KxDataView2::Node* node: selected)
				{
					if (IGameMod* mod = TestDNDNode(*node))
					{
						dataObject->AddMod(*mod);
					}
				}

				if (dataObject && !dataObject->IsEmpty())
				{
					event.DragDone(*dataObject, wxDrag_AllowMove);
					return;
				}
			}
		}

		wxBell();
		event.DragCancel();
	}
	void DisplayModel::OnDropItems(KxDataView2::EventDND& event)
	{
		DisplayModelModNode* droppenOnNode = nullptr;
		if (event.GetNode() && event.GetNode()->QueryInterface(droppenOnNode))
		{
			if (DisplayModelDNDObject* dataObject = event.GetRecievedDataObject<DisplayModelDNDObject>())
			{
				IGameMod* droppedOnMod = &droppenOnNode->GetMod();
				PriorityGroup* droppedOnPriorityGroup = nullptr;
				if (droppedOnMod->QueryInterface(droppedOnPriorityGroup))
				{
					droppedOnMod = &droppedOnPriorityGroup->GetBaseMod();
				}

				// Move and refresh
				bool isMoved = false;
				IGameMod::RefVector& modsToMove = dataObject->GetMods();

				if (droppedOnPriorityGroup)
				{
					isMoved = IModManager::GetInstance()->MoveModsBefore(modsToMove, *droppedOnMod);
				}
				else
				{
					isMoved = IModManager::GetInstance()->MoveModsAfter(modsToMove, *droppedOnMod);
				}

				// Update control
				if (isMoved)
				{
					IModManager::GetInstance()->Save();

					// If items dragged over priority group, assign them to it
					if (droppedOnPriorityGroup)
					{
						for (IGameMod* mod: modsToMove)
						{
							mod->SetPriorityGroupTag(droppedOnMod->GetPriorityGroupTag());
							mod->Save();
						}
					}

					// Reload control data
					LoadView();

					// Select moved items
					for (IGameMod* mod: dataObject->GetMods())
					{
						SelectMod(mod);
					}

					// Send event
					ModEvent(Events::ModsReordered, std::move(modsToMove)).Send();
					
					event.DropDone();
					return;
				}
			}
		}

		wxBell();
		event.DropError();
	}
	void DisplayModel::OnDropItemsPossible(KxDataView2::EventDND& event)
	{
		if (KxDataView2::Node* node = event.GetNode(); node && TestDNDNode(*node, true))
		{
			event.Allow();
		}
		else
		{
			event.Veto();
		}
	}

	DisplayModel::DisplayModel()
	{
		m_NoneTag = IModTagManager::GetInstance()->NewTag();
		m_NoneTag->SetName(KAux::MakeNoneLabel());
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

	void DisplayModel::CreateSearchColumnsMenu(KxMenu& menu)
	{
		auto AddItem = [this, &menu](ColumnID id, bool enable = false) -> KxMenuItem*
		{
			KxDataView2::Column* column = GetView()->GetColumnByID(id);
			if (column)
			{
				wxString title = column->GetTitle();
				if (title.IsEmpty())
				{
					title << '<' << (column->GetDisplayIndex() + 1) << '>';
				}

				//enable = enable || m_SearchFilterOptions.GetAttributeBool(std::to_string(id));

				KxMenuItem* menuItem = menu.Add(new KxMenuItem(title, wxEmptyString, wxITEM_CHECK));
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
	void DisplayModel::SetSearchColumns(const std::vector<KxDataView2::Column*>& columns)
	{
		auto Save = [this](bool value)
		{
			for (const KxDataView2::Column* column: m_SearchColumns)
			{
				//m_SearchFilterOptions.SetAttribute(std::to_string(column->GetID()), value);
			}
		};

		Save(false);
		m_SearchColumns = columns;
		Save(true);
	}
	bool DisplayModel::FilterMod(const IGameMod& mod) const
	{
		if (!mod.IsInstalled() && !ShouldShowNotInstalledMods())
		{
			return false;
		}
		if (m_SearchColumns.empty() || m_SearchMask.IsEmpty())
		{
			return true;
		}

		bool found = false;
		for (const KxDataView2::Column* column: m_SearchColumns)
		{
			switch (column->GetID<ColumnID>())
			{
				case ColumnID::Name:
				{
					found = KAux::CheckSearchMask(m_SearchMask, mod.GetName()) || KAux::CheckSearchMask(m_SearchMask, mod.GetID());
					break;
				}
				case ColumnID::Author:
				{
					found = KAux::CheckSearchMask(m_SearchMask, mod.GetAuthor());
					break;
				}
				case ColumnID::Version:
				{
					found = KAux::CheckSearchMask(m_SearchMask, mod.GetVersion());
					break;
				}
				case ColumnID::Tags:
				{
					found = KAux::CheckSearchMask(m_SearchMask, FormatTagList(mod));
					break;
				}
				case ColumnID::PackagePath:
				{
					found = KAux::CheckSearchMask(m_SearchMask, mod.GetPackageFile());
					break;
				}
				case ColumnID::Signature:
				{
					found = KAux::CheckSearchMask(m_SearchMask, mod.GetSignature());
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

	void DisplayModel::SelectMod(const IGameMod* mod)
	{
		if (mod)
		{
			if (DisplayModelModNode* node = ModToNode(*mod))
			{
				node->EnsureVisible();
				node->Select();
			}
		}
	}
	IGameMod* DisplayModel::GetSelectedMod() const
	{
		KxDataView2::Node* node = GetView()->GetSelection();
		DisplayModelModNode* modNode = nullptr;
		if (node && node->QueryInterface(modNode))
		{
			return &modNode->GetMod();
		}
		return nullptr;
	}
}
