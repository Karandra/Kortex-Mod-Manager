#pragma once
#include "stdafx.h"
#include "KModManagerModelNode.h"
#include "KDataViewListModel.h"
#include "KProgramOptions.h"
#include "KImageProvider.h"
#include "KBitmapSize.h"
class KModController;
class KModWorkspace;
class KxDataViewBitmapTextToggleRenderer;

enum KModManagerModelType
{
	KMM_TYPE_CONNECTOR,
	KMM_TYPE_MANAGER,
};

class KModManagerModelDataObject;
class KModManagerModel:	public KDataViewModelBase, public KDataViewModelDragDropEnabled<KModManagerModelDataObject>
{
	private:
		KModManagerModelType m_DisplayMode = KMM_TYPE_CONNECTOR;
		bool m_ShowPriorityGroups = false;
		bool m_ShowPriorityGroupsSuppress = false;
		bool m_ShowNotInstalledMods = false;

		KBitmapSize m_BitmapSize;
		KxDataViewColumn* m_NameColumn = NULL;
		KxDataViewColumn* m_BitmapColumn = NULL;
		KxDataViewColumn* m_PriorityColumn = NULL;
		KxColor m_PriortyGroupColor;

		const KModTag m_NoneTag;
		KMMLogModelNodeVector m_DataVector;
		KModEntryArray* m_Entries = NULL;
		std::vector<KPriorityGroupEntry> m_PriortyGroups;
		
		wxString m_SearchMask;
		KxDataViewColumn::Vector m_SearchColumns;
		KProgramOptionUI m_SearchFilterOptions;

	private:
		bool IsTree() const
		{
			return m_DisplayMode == KMM_TYPE_MANAGER;
		}
		bool IsSpecialSiteColumn(int column) const;
		bool CanShowPriorityGroups() const
		{
			return m_ShowPriorityGroups && !m_ShowPriorityGroupsSuppress;
		}
		KNetworkProviderID ColumnToSpecialSite(int column) const;
		wxString FormatTagList(const KModEntry* entry) const;

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
		void GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column, const KModEntry* entry) const;
		void GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column, const KFixedModEntry* entry) const;
		void GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column, const KPriorityGroupEntry* entry) const;
		
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

		void AskOpenSites(const KModEntry* entry) const;
		wxBitmap CreateModThumbnail(const KModEntry* entry) const;

		virtual KxDataViewCtrl* GetViewCtrl() const override
		{
			return GetView();
		}
		virtual bool OnDragItems(KxDataViewEventDND& event) override;
		virtual bool OnDropItems(KxDataViewEventDND& event) override;
		bool CanDragDropNow() const;

	public:
		KModManagerModel();

	public:
		KModManagerModelType GetDisplayMode() const
		{
			return m_DisplayMode;
		}
		void SetDisplayMode(KModManagerModelType mode);

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

		void RefreshItem(KModEntry* entry)
		{
			ItemChanged(MakeItem(entry));
		}

		const KModTagArray& GetTags() const;
		void SetDataVector();
		void SetDataVector(KModEntryArray& array);
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
		bool FilterMod(const KModEntry* modEntry) const;

		KxDataViewItem MakeItem(const KMMModelNode* node) const;
		KxDataViewItem MakeItem(const KMMModelNode& node) const
		{
			return MakeItem(&node);
		}
		KMMModelNode* GetNode(const KxDataViewItem& item) const;
		
		void SelectMod(const KModEntry* entry)
		{
			SelectItem(GetItemByEntry(entry));
		}
		KModEntry* GetSelectedModEntry() const
		{
			return GetModEntry(GetView()->GetSelection());
		}
		KxDataViewItem GetItemByEntry(const KModEntry* entry) const;
		KModEntry* GetModEntry(const KxDataViewItem& item) const
		{
			KMMModelNode* node = GetNode(item);
			return node ? node->GetEntry() : NULL;
		}
		size_t CountItemsInGroup(const KModTag* group) const;
};

class KModManagerModelDataObject: public KDataViewModelDragDropData
{
	private:
		KModEntryArray m_Entries;

	public:
		KModManagerModelDataObject(size_t count = 0)
		{
			m_Entries.reserve(count);
		}

	public:
		const KModEntryArray& GetEntries() const
		{
			return m_Entries;
		}
		void AddEntry(KModEntry* entry)
		{
			m_Entries.push_back(entry);
		}
};
