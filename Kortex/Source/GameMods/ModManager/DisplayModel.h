#pragma once
#include "stdafx.h"
#include "DisplayModelNode.h"
#include "KDataViewListModel.h"
#include "KImageProvider.h"
#include "KBitmapSize.h"
class KxDataViewBitmapTextToggleRenderer;

namespace Kortex
{
	class IGameMod;
	class IModTag;
}

namespace Kortex::ModManager
{
	enum class DisplayModelType
	{
		Connector,
		Manager,
	};

	class IFixedGameMod;
	class IPriorityGroup;
	class Workspace;
	class DisplayModelDNDObject;

	class DisplayModel:	public KxDataViewModelExBase<KxDataViewModel>, public KxDataViewModelExDragDropEnabled<DisplayModelDNDObject>
	{
		public:
			enum class PriorityGroupLabelAlignment
			{
				Center,
				Left,
				Right,
			};

		private:
			bool m_ShowPriorityGroups = false;
			bool m_ShowPriorityGroupsSuppress = false;
		
			DisplayModelType m_DisplayMode = DisplayModelType::Connector;
			bool m_ShowNotInstalledMods = false;
			bool m_BoldPriorityGroupLabels = false;

			PriorityGroupLabelAlignment m_PriorityGroupLabelAlignmentType = PriorityGroupLabelAlignment::Center;
			int m_PriorityGroupLabelAlignment = wxALIGN_INVALID;

			KBitmapSize m_BitmapSize;
			int m_PriorityGroupRowHeight = 0;
			KxColor m_PriortyGroupColor;

			KxDataViewColumn* m_NameColumn = nullptr;
			KxDataViewColumn* m_BitmapColumn = nullptr;
			KxDataViewColumn* m_PriorityColumn = nullptr;

			std::unique_ptr<IModTag> m_NoneTag;
			DisplayModelNode::Vector m_DataVector;
			BasicGameMod::Vector* m_Entries = nullptr;
			std::vector<PriorityGroup> m_PriortyGroups;
		
			wxString m_SearchMask;
			KxDataViewColumn::Vector m_SearchColumns;

		private:
			bool IsTree() const
			{
				return m_DisplayMode == DisplayModelType::Manager;
			}
			bool IsSpecialSiteColumn(int column) const;
			bool CanShowPriorityGroups() const
			{
				return m_ShowPriorityGroups && !m_ShowPriorityGroupsSuppress;
			}
			Kortex::Network::ProviderID ColumnToSpecialSite(int column) const;
			wxString FormatTagList(const IGameMod& entry) const;

			virtual void OnInitControl() override;
		
			virtual bool IsListModel() const override;
			virtual bool HasContainerColumns(const KxDataViewItem& item) const override;
			virtual bool HasDefaultCompare() const override
			{
				return true;
			}
			virtual bool IsContainer(const KxDataViewItem& item) const override;
			virtual KxDataViewItem GetParent(const KxDataViewItem& item) const override;
			virtual void GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const override;
		
			virtual void GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			virtual void GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			void GetValueMod(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column, const IGameMod* mod) const;
			void GetValueFixedMod(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column, const IGameMod* mod) const;
			void GetValuePriorityGroup(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column, const IGameMod* mod, const IPriorityGroup* group) const;
		
			virtual bool SetValue(const wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) override;
			virtual bool IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			virtual bool IsEditorEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			virtual bool GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const override;
			virtual bool GetCellHeight(const KxDataViewItem& item, int& height) const override;
			virtual bool Compare(const KxDataViewItem& item1, const KxDataViewItem& item2, const KxDataViewColumn* column) const override;

			void OnSelectItem(KxDataViewEvent& event);
			void OnActivateItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);
			void OnHeaderContextMenu(KxDataViewEvent& event);
			void OnColumnSorted(KxDataViewEvent& event);
			void OnCacheHint(KxDataViewEvent& event);

			wxBitmap CreateModThumbnail(const IGameMod& entry) const;
			virtual KxDataViewCtrl* GetViewCtrl() const override
			{
				return GetView();
			}
			virtual bool OnDragItems(KxDataViewEventDND& event) override;
			virtual bool OnDropItems(KxDataViewEventDND& event) override;
			bool CanDragDropNow() const;

		public:
			DisplayModel();

		public:
			DisplayModelType GetDisplayMode() const
			{
				return m_DisplayMode;
			}
			void SetDisplayMode(DisplayModelType mode);

			bool ShouldShowPriorityGroups() const
			{
				return m_ShowPriorityGroups;
			}
			void ShowPriorityGroups(bool value)
			{
				m_ShowPriorityGroups = value;
			}
		
			bool ShouldShowNotInstalledMods() const
			{
				return m_ShowNotInstalledMods;
			}
			void ShowNotInstalledMods(bool value)
			{
				m_ShowNotInstalledMods = value;
			}

			bool IsBoldPriorityGroupLabels() const
			{
				return m_BoldPriorityGroupLabels;
			}
			void SetBoldPriorityGroupLabels(bool value)
			{
				m_BoldPriorityGroupLabels = value;
			}

			PriorityGroupLabelAlignment GetPriorityGroupLabelAlignment() const;
			void SetPriorityGroupLabelAlignment(PriorityGroupLabelAlignment value);

			void RefreshItem(IGameMod& entry)
			{
				ItemChanged(MakeItem(entry));
			}

			void SetDataVector();
			void SetDataVector(BasicGameMod::Vector& array);
			virtual void RefreshItems() override;
			void UpdateUI();
			void UpdateRowHeight();
		
			void CreateSearchColumnsMenu(KxMenu& menu);
			bool SetSearchMask(const wxString& mask)
			{
				KAux::SetSearchMask(m_SearchMask, mask);
				return true;
			}
			void SetSearchColumns(const KxDataViewColumn::Vector& columns);
			bool FilterMod(const IGameMod& modEntry) const;

			KxDataViewItem MakeItem(const DisplayModelNode* node) const;
			KxDataViewItem MakeItem(const DisplayModelNode& node) const
			{
				return MakeItem(&node);
			}
			DisplayModelNode* GetNode(const KxDataViewItem& item) const;
		
			void SelectMod(const IGameMod* entry)
			{
				SelectItem(GetItemByEntry(entry));
			}
			IGameMod* GetSelectedModEntry() const
			{
				return GetModEntry(GetView()->GetSelection());
			}
			KxDataViewItem GetItemByEntry(const IGameMod* entry) const;
			IGameMod* GetModEntry(const KxDataViewItem& item) const
			{
				DisplayModelNode* node = GetNode(item);
				return node ? node->GetEntry() : nullptr;
			}
			size_t CountItemsInGroup(const IModTag* group) const;
	};
}

namespace Kortex::ModManager
{
	class DisplayModelDNDObject: public KxDataViewModelExDragDropData
	{
		private:
			IGameMod::RefVector m_Entries;

		public:
			DisplayModelDNDObject(size_t count = 0)
			{
				m_Entries.reserve(count);
			}

		public:
			void AddEntry(IGameMod* entry)
			{
				m_Entries.push_back(entry);
			}
			const IGameMod::RefVector& GetEntries() const
			{
				return m_Entries;
			}
	};
}
