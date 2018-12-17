#pragma once
#include "stdafx.h"
#include "KDataViewListModel.h"
#include "GameData/IGameSave.h"
#include "KBitmapSize.h"

namespace Kortex
{
	class ISaveManager;
}

namespace Kortex::SaveManager
{
	class Workspace;

	class DisplayModel: public KxDataViewVectorListModelEx<IGameSave::Vector, KxDataViewListModelEx>
	{
		private:
			ISaveManager* m_Manager = nullptr;
			Workspace* m_Workspace = nullptr;
			IGameSave::Vector m_DataVector;

			KxDataViewColumn* m_BitmapColumn = nullptr;
			KBitmapSize m_BitmapSize;

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
			DisplayModel(ISaveManager* manager, Workspace* workspace);

		public:
			void SetDataVector();
			void SetDataVector(const wxString& folder, const KxStringVector& filtersList);

			IGameSave* GetDataEntry(size_t row) const
			{
				if (row < GetItemCount())
				{
					return m_DataVector[row].get();
				}
				return nullptr;
			}

			void UpdateRowHeight();
	};
}
