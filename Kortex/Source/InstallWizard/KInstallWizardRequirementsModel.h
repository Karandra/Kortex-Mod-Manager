#pragma once
#include "stdafx.h"
#include "Utility/KDataViewListModel.h"
#include "PackageProject/KPackageProjectRequirements.h"

class KInstallWizardRequirementsModel: public KDataViewVectorListModel<KPPRRequirementEntry::RefVector, KDataViewListModel>
{
	private:
		const KPackageProjectRequirements* m_RequirementsInfo = nullptr;
		KPPRRequirementEntry::RefVector m_DataVector;

	private:
		virtual void OnInitControl() override;

		virtual void GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;
		virtual bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;

		wxIcon GetIconByState(KPPReqState state) const;
		wxIcon GetIconByState(bool state) const
		{
			return GetIconByState(state ? KPPReqState::True : KPPReqState::False);
		}
		void OnActivateItem(KxDataViewEvent& event);

	public:
		void SetDataVector();
		void SetDataVector(const KPackageProjectRequirements* reqsInfo, const KxStringVector& reqIDs);

		KPPRRequirementEntry* GetDataEntry(size_t index) const
		{
			if (index < GetItemCount())
			{
				return m_DataVector.at(index);
			}
			return nullptr;
		}
		KPPRRequirementEntry* GetDataEntry(const KxDataViewItem& item) const
		{
			return GetDataEntry(GetRow(item));
		}
};
