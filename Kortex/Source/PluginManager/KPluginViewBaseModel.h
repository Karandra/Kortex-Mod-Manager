#pragma once
#include "stdafx.h"
#include "KDataViewListModel.h"
#include "KPluginManager.h"
#include "KPluginManagerBethesdaGeneric.h"
#include "KProgramOptions.h"
class KPluginManagerWorkspace;
class KPluginManagerConfigStandardContentEntry;
class KPluginManagerListModelDataObject;

class KPluginManagerListModel:
	public KDataViewVectorListModel<KPMPluginEntryVector, KDataViewListModel>,
	public KDataViewModelDragDropEnabled<KPluginManagerListModelDataObject>
{
	public:
		static const int ms_LightPluginIndex = 0xFE;

	private:
		KPluginManagerWorkspace* m_Workspace = NULL;
		wxString m_SearchMask;

	private:
		virtual void OnInitControl() override;

		virtual void GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const override;
		virtual void GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;
		virtual bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
		virtual bool GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const override;
		virtual bool HasDefaultCompare() const override
		{
			return true;
		}
		virtual bool CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const override;

		bool GetPluginIndex(const KPMPluginEntry* entry, int& index) const;
		int CountLightActiveBefore(size_t index) const;
		int CountInactiveBefore(size_t index) const;
		const wxString& GetPartOfName(const KPMPluginEntry* entry) const;
		wxString GetPluginAuthor(const KPMPluginEntry* entry) const;

		void OnSelectItem(KxDataViewEvent& event);
		void OnActivateItem(KxDataViewEvent& event);
		void OnContextMenu(KxDataViewEvent& event);

		virtual KxDataViewCtrl* GetViewCtrl() const override
		{
			return GetView();
		}
		virtual bool OnDragItems(KxDataViewEventDND& event) override;
		virtual bool OnDropItems(KxDataViewEventDND& event) override;
		bool CanDragDropNow() const;

	public:
		KPluginManagerListModel(KPluginManagerWorkspace* workspace);

	public:
		KPluginManagerWorkspace* GetWorkspace() const
		{
			return m_Workspace;
		}
		void ChangeNotify();

		void SetDataVector();
		void SetDataVector(KPMPluginEntryVector& array);

		const KPMPluginEntry* GetDataEntry(size_t index) const
		{
			const KPMPluginEntryVector& items = *GetDataVector();
			return index < items.size() ? items[index].get() : NULL;
		}
		KPMPluginEntry* GetDataEntry(size_t index)
		{
			const KPMPluginEntryVector& items = *GetDataVector();
			return index < items.size() ? items[index].get() : NULL;
		}
		KxDataViewItem GetItemByEntry(const KPMPluginEntry* entry) const
		{
			const KPMPluginEntryVector& items = *GetDataVector();
			auto it = std::find_if(items.begin(), items.end(), [entry](const ValueType& v)
			{
				return v.get() == entry;
			});
			if (it != items.end())
			{
				return GetItem(std::distance(items.begin(), it));
			}
			return KxDataViewItem();
		}

		void SetAllEnabled(bool value);
		KPMPluginEntry* GetSelectedMod()
		{
			return GetDataEntry(GetSelectedModIndex());
		}
		int GetSelectedModIndex() const
		{
			size_t index = GetRow(GetView()->GetSelection());
			return index != (size_t)-1 ? index : (int)-1;
		}
		void UpdateUI();
		bool SetSearchMask(const wxString& mask)
		{
			return KAux::SetSearchMask(m_SearchMask, mask);
		}
};

class KPluginManagerListModelDataObject: public KDataViewModelDragDropData
{
	private:
		KPMPluginEntryRefVector m_Entries;

	public:
		KPluginManagerListModelDataObject(size_t count = 0)
		{
			m_Entries.reserve(count);
		}

	public:
		const KPMPluginEntryRefVector& GetEntries() const
		{
			return m_Entries;
		}
		void AddEntry(KPMPluginEntry* entry)
		{
			m_Entries.push_back(entry);
		}
};
