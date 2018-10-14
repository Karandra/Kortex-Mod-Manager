#pragma once
#include "stdafx.h"
#include "KDataViewListModel.h"
#include "KProgramManager.h"

class KProgramManagerModel: public KDataViewVectorListModel<KProgramManagerEntry::Vector, KDataViewListModel>
{
	private:
		virtual void OnInitControl() override;
		
		virtual void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
		virtual bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;

		void OnActivateItem(KxDataViewEvent& event);
		void OnSelectItem(KxDataViewEvent& event);
		void OnContextMenu(KxDataViewEvent& event);

		KProgramManagerEntry::Vector& GetProgramsList() const
		{
			return KProgramManager::GetInstance()->GetProgramList();
		}
		KProgramManagerEntry* GetDataEntry(size_t i)
		{
			if (i < GetItemCount())
			{
				return &GetDataVector()->at(i);
			}
			return NULL;
		}
		
		wxString AskSelectExecutablePath(const KProgramManagerEntry* entry = NULL) const;
		bool AddProgram();
		void RemoveProgram(KProgramManagerEntry* entry);

	public:
		KProgramManagerModel()
		{
			SetDataVector(&GetProgramsList());
		}

	public:
		virtual size_t GetItemCount() const override
		{
			return GetProgramsList().size();
		}
		virtual void RefreshItems() override;

		const KProgramManagerEntry* GetDataEntry(size_t i) const
		{
			if (i < GetItemCount())
			{
				return &GetDataVector()->at(i);
			}
			return NULL;
		}
		const KProgramManagerEntry* GetDataEntry(const KxDataViewItem& item) const
		{
			return GetDataEntry(GetRow(item));
		}
};
