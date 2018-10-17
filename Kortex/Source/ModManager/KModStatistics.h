#pragma once
#include "stdafx.h"
#include "KDataViewListModel.h"
#include "KModEntry.h"
#include "KLabeledValue.h"
#include <KxFramework/KxStdDialog.h>
class KxButton;

enum KModStatisticsType
{
	KMM_STAT_INVALID = -1,
	KMM_STAT_MIN = 0,

	KMM_STAT_MOD_COUNT_TOTAL,
	KMM_STAT_MOD_COUNT_ACTIVE,
	KMM_STAT_MOD_COUNT_INACTIVE,
	KMM_STAT_MOD_COUNT_FILES,
	KMM_STAT_MOD_COUNT_FOLDERS,
	KMM_STAT_MODS_SIZE,

	KMM_STAT_MAX,
	KMM_STAT_COUNT = KMM_STAT_MAX - 1
};

class KModStatisticsModel: public KxDataViewVectorListModelEx<KxStringVector, KxDataViewListModelEx>
{
	enum CountMode
	{
		All,
		Active,
		Inactive,
	};

	private:
		KxStringVector m_DataVector;

	protected:
		virtual void OnInitControl() override;

		virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;

	private:
		wxString GetStatName(KModStatisticsType index) const;
		const wxString& GetStatValue(KModStatisticsType index) const;
		wxString CalcStatValue(KModStatisticsType index) const;

		int64_t CountMods(CountMode mode) const;
		int64_t CalcModStoreSize() const;
		int64_t CountFilesAndFolders(KxFileSearchType type, CountMode mode) const;

	public:
		KModStatisticsModel()
		{
			m_DataVector.resize(KMM_STAT_COUNT);
			SetDataVector(&m_DataVector);
		}

	public:
		virtual size_t GetItemCount() const override
		{
			return KMM_STAT_COUNT;
		}
		virtual void RefreshItems() override;

		KModStatisticsType GetIndex(size_t row) const
		{
			if (row < KMM_STAT_COUNT)
			{
				return static_cast<KModStatisticsType>(row + 1);
			}
			return KMM_STAT_INVALID;
		}
		KxDataViewItem MakeItem(KModStatisticsType index) const
		{
			return KxDataViewItem((intptr_t)index);
		}
};

//////////////////////////////////////////////////////////////////////////
class KModStatisticsDialog: public KxStdDialog, public KModStatisticsModel
{
	private:
		wxWindow* m_ViewPane = NULL;
		KxButton* m_AddButton = NULL;
		KxButton* m_RemoveButton = NULL;

	private:
		virtual wxWindow* GetDialogMainCtrl() const override
		{
			return m_ViewPane;
		}

	public:
		KModStatisticsDialog(wxWindow* parent);
		virtual ~KModStatisticsDialog();
};
