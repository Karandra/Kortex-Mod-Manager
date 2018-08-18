#pragma once
#include "stdafx.h"
#include "KDataViewListModel.h"
#include "KRunManager.h"

class KRunManagerWorkspaceView: public KDataViewVectorListModel<KRMProgramEntryArray, KDataViewListModel>
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

		KRMProgramEntryArray& GetProgramsList() const
		{
			return KRunManager::Get().GetProgramList();
		}
		KRunManagerProgram* GetDataEntry(size_t i)
		{
			if (i < GetItemCount())
			{
				return &GetDataVector()->at(i);
			}
			return NULL;
		}
		
		wxString AskSelectExecutablePath(const KRunManagerProgram* entry = NULL) const;
		bool AddProgram();
		void RemoveProgram(KRunManagerProgram* entry);

	public:
		KRunManagerWorkspaceView()
		{
			SetDataVector(&GetProgramsList());
		}

	public:
		virtual size_t GetItemCount() const override
		{
			return GetProgramsList().size();
		}
		virtual void RefreshItems() override;

		const KRunManagerProgram* GetDataEntry(size_t i) const
		{
			if (i < GetItemCount())
			{
				return &GetDataVector()->at(i);
			}
			return NULL;
		}
		const KRunManagerProgram* GetDataEntry(const KxDataViewItem& item) const
		{
			return GetDataEntry(GetRow(item));
		}
};
