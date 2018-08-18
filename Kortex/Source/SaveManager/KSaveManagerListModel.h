#pragma once
#include "stdafx.h"
#include "KDataViewListModel.h"
#include "KSMSaveFile.h"
class KSaveManager;
class KSaveManagerWorkspace;

class KSaveManagerListModel: public KDataViewVectorListModel<KSMSaveFileArray, KDataViewListModel>
{
	private:
		KSaveManager* m_Manager = NULL;
		KSaveManagerWorkspace* m_Workspace = NULL;
		KSMSaveFileArray m_DataVector;

		int ms_BitmapWidth;
		int m_RowHeight;

	private:
		virtual void OnInitControl() override;

		virtual void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual void GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
		virtual bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
		virtual bool HasDefaultCompare() const override
		{
			return true;
		}
		virtual bool CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const override;

		void OnSelectItem(KxDataViewEvent& event);
		void OnActivateItem(KxDataViewEvent& event);
		void OnContextMenu(KxDataViewEvent& event);
		void OnHeaderContextMenu(KxDataViewEvent& event);
		void OnCacheHint(KxDataViewEvent& event);

	public:
		KSaveManagerListModel(KSaveManager* manager, KSaveManagerWorkspace* workspace);

	public:
		void SetDataVector();
		void SetDataVector(const wxString& folder, const KxStringVector& filtersList);

		KSMSaveFile* GetDataEntry(size_t row) const
		{
			if (row < GetItemCount())
			{
				return m_DataVector[row].get();
			}
			return NULL;
		}
};
