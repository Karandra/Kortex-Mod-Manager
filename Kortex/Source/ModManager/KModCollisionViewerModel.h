#pragma once
#include "stdafx.h"
#include "KDataViewListModel.h"
#include "KImageProvider.h"
#include "KModManager.h"
#include "KModManagerDispatcher.h"
#include "KProgramOptions.h"
#include <KxFramework/KxStdDialog.h>
#include <KxFramework/KxFileFinder.h>
class KOperationWithProgressBase;

class KModCollisionViewerModelEntry
{
	public:
		using CollisionVector = KModManagerDispatcher::CollisionVector;

	private:
		KxFileFinderItem m_File;
		wxString m_RelativePath;
		CollisionVector m_Collisions;

	public:
		KModCollisionViewerModelEntry(const KxFileFinderItem& item)
			:m_File(item)
		{
		}

	public:
		bool FindCollisions(const KModEntry* modEntry);
		const CollisionVector& GetCollisions() const
		{
			return m_Collisions;
		}
		bool HasCollisions() const
		{
			return !m_Collisions.empty();
		}

		const KxFileFinderItem& GetFileItem() const
		{
			return m_File;
		}
		KxFileFinderItem& GetFileItem()
		{
			return m_File;
		}
		const wxString& GetRelativePath() const
		{
			return m_RelativePath;
		}
};
using KModCollisionViewerModelEntryVector = std::vector<KModCollisionViewerModelEntry>;

class KModCollisionViewerModel: public KDataViewVectorListModel<KModCollisionViewerModelEntryVector, KDataViewListModel>
{
	public:
		using CollisionVector = KModManagerDispatcher::CollisionVector;

		static wxString FormatSingleCollision(const KMMDispatcherCollision& collision);
		static wxString FormatCollisionsCount(const CollisionVector& collisions);
		static wxString FormatCollisionsView(const CollisionVector& collisions);

	private:
		KModCollisionViewerModelEntryVector m_DataVector;
		const KModEntry* m_ModEntry = NULL;

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
		KModCollisionViewerModel(const KModEntry* modEntry);

	public:
		const ValueType* GetDataEntry(size_t i) const
		{
			if (i < GetItemCount())
			{
				return &m_DataVector[i];
			}
			return NULL;
		}
};

//////////////////////////////////////////////////////////////////////////
class KModCollisionViewerModelDialog: public KxStdDialog, public KModCollisionViewerModel
{
	private:
		wxWindow* m_ViewPane = NULL;
		KProgramOptionUI m_ViewOptions;
		KProgramOptionUI m_WindowOptions;

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
		KModCollisionViewerModelDialog(wxWindow* window, const KModEntry* modEntry);
		virtual ~KModCollisionViewerModelDialog();
};
