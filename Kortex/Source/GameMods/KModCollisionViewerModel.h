#pragma once
#include "stdafx.h"
#include <Kortex/ModManager.hpp>
#include "Utility/KDataViewListModel.h"
#include <KxFramework/KxStdDialog.h>
#include <KxFramework/KxFileFinder.h>
class KOperationWithProgressBase;

namespace Kortex
{
	class IGameMod;
}

namespace KModCollisionViewerModelNS
{
	class ModelEntry
	{
		public:
			using Vector = std::vector<ModelEntry>;
			using CollisionVector = Kortex::ModManager::KDispatcherCollision::Vector;

		private:
			KxFileItem m_Item;
			wxString m_RelativePath;
			CollisionVector m_Collisions;

		public:
			ModelEntry(const KxFileItem& item)
				:m_Item(item)
			{
			}

		public:
			bool FindCollisions(const Kortex::IGameMod& modEntry);
			const CollisionVector& GetCollisions() const
			{
				return m_Collisions;
			}
			bool HasCollisions() const
			{
				return !m_Collisions.empty();
			}

			const KxFileItem& GetFileItem() const
			{
				return m_Item;
			}
			KxFileItem& GetFileItem()
			{
				return m_Item;
			}
			const wxString& GetRelativePath() const
			{
				return m_RelativePath;
			}
	};
}

//////////////////////////////////////////////////////////////////////////
class KModCollisionViewerModel: public KxDataViewVectorListModelEx<KModCollisionViewerModelNS::ModelEntry::Vector, KxDataViewListModelEx>
{
	public:
		using ModelEntry = KModCollisionViewerModelNS::ModelEntry;
		using CollisionVector = Kortex::ModManager::KDispatcherCollision::Vector;

		static wxString FormatSingleCollision(const Kortex::ModManager::KDispatcherCollision& collision);
		static wxString FormatCollisionsCount(const CollisionVector& collisions);
		static wxString FormatCollisionsView(const CollisionVector& collisions);

	private:
		ModelEntry::Vector m_DataVector;
		const Kortex::IGameMod* m_ModEntry = nullptr;

	private:
		virtual void OnInitControl() override;

		virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
		virtual bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;

		void OnSelectItem(KxDataViewEvent& event);
		void OnActivateItem(KxDataViewEvent& event);
		void OnContextMenu(KxDataViewEvent& event);

	protected:
		void RunCollisionsSearch(KOperationWithProgressBase* context);

	public:
		KModCollisionViewerModel(const Kortex::IGameMod* modEntry);

	public:
		const ValueType* GetDataEntry(size_t i) const
		{
			if (i < GetItemCount())
			{
				return &m_DataVector[i];
			}
			return nullptr;
		}
};

//////////////////////////////////////////////////////////////////////////
class KModCollisionViewerModelDialog: public KxStdDialog, public KModCollisionViewerModel
{
	private:
		wxWindow* m_ViewPane = nullptr;
		//KProgramOptionAI m_ViewOptions;
		//KProgramOptionAI m_WindowOptions;

	private:
		wxWindow* GetDialogMainCtrl() const override
		{
			return m_ViewPane;
		}
		wxWindow* GetDialogFocusCtrl() const override
		{
			return GetView();
		}

		void RunThread();

	public:
		KModCollisionViewerModelDialog(wxWindow* window, const  Kortex::IGameMod* modEntry);
		virtual ~KModCollisionViewerModelDialog();
};
