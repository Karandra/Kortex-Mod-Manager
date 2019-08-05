#pragma once
#include "stdafx.h"
#include "Utility/KDataViewListModel.h"

namespace Kortex
{
	class IModTag;
	class ModTagStore;
	class IGameMod;
}

namespace Kortex::ModTagManager
{
	class SelectorDisplayModel: public KxDataViewListModelEx
	{
		protected:
			enum ColumnID
			{
				PriorityTag,
				Expanded,
				Name,
				NexusID,
				Color,
			};

		private:
			const bool m_FullFeatured = false;

		protected:
			ModTagStore* m_Data = nullptr;
			IGameMod* m_GameMod = nullptr;
			const IModTag* m_PromaryTag = nullptr;
			bool m_IsModified = false;

		protected:
			void OnInitControl() override;

			void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
			bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
			bool GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attribute, KxDataViewCellState cellState) const override;

			void OnActivate(KxDataViewEvent& event);
			void OnKeyDown(wxKeyEvent& event);

		public:
			SelectorDisplayModel(bool isFullFeatured = false);

		public:
			void SetDataVector(ModTagStore* tagStore = nullptr, IGameMod* mod = nullptr);
			size_t GetItemCount() const override;
			IModTag* GetDataEntry(size_t index) const;

			bool IsFullFeatured() const
			{
				return m_FullFeatured;
			}
			bool IsModified() const
			{
				return m_IsModified;
			}
			
			bool HasPriorityGroupTag() const
			{
				return m_PromaryTag != nullptr;
			}
			void ApplyChangesToMod();
	};
}
