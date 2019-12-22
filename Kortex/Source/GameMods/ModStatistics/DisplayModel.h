#pragma once
#include "stdafx.h"
#include "KxFramework/KxDataViewListModelEx.h"
#include "GameMods/IModStatistics.h"

namespace Kortex::ModStatistics
{
	class DisplayModel: public KxDataViewVectorListModelEx<KxStringVector, KxDataViewListModelEx>
	{
		private:
			IModStatistics* m_Stats = nullptr;
			KxStringVector m_DataVector;

		protected:
			void OnInitControl() override;

			void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;

		private:
			wxString GetStatName(size_t index) const;
			const wxString& GetStatValue(size_t index) const;

		public:
			DisplayModel();

		public:
			size_t GetItemCount() const override
			{
				return m_Stats->GetStatCount();
			}
			void RefreshItems() override;
	};
}
