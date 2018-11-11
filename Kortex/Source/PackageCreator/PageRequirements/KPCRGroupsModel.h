#pragma once
#include "stdafx.h"
#include "PackageCreator/KPackageCreatorVectorModel.h"
#include "PackageCreator/KPackageCreatorIDTracker.h"
#include "PackageProject/KPackageProjectRequirements.h"
class KPCREntriesListModel;
class KxDataViewComboBox;
class KxButton;

class KPCRGroupsModel: public KPackageCreatorVectorModel<KPPRRequirementsGroup::Vector>, public KPackageCreatorIDTracker
{
	private:
		KPackageProjectRequirements* m_Requirements = NULL;
		KxDataViewComboBox* m_ComboView = NULL;
		KPCREntriesListModel* m_EntriesModel = NULL;
		KxButton* m_AddButton = NULL;
		KxButton* m_RemoveButton = NULL;

	private:
		virtual KxDataViewCtrl* OnCreateDataView(wxWindow* window) override;
		virtual wxWindow* OnGetDataViewWindow() override;
		virtual void OnInitControl() override;

		virtual void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;

		void OnSelectItem(KxDataViewEvent& event);
		void OnActivateItem(KxDataViewEvent& event);
		void OnContextMenu(KxDataViewEvent& event);
		void OnGetStringValue(KxDataViewEvent& event);

		void OnAddGroup(bool useDialog = false);
		void OnRemoveGroup(const KxDataViewItem& item);
		void OnClearList();
		virtual bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
		{
			OnInsertItemHelperUniquePtr(*GetDataVector(), currentItem, droppedItem);
			return true;
		}

		void RefreshComboControl();
		bool DoTrackID(const wxString& trackedID, const wxString& newID, bool remove) const;

	protected:
		virtual KPackageCreatorIDTracker* GetTracker() override
		{
			return this;
		}
		virtual bool TrackChangeID(const wxString& trackedID, const wxString& newID) override
		{
			return DoTrackID(trackedID, newID, false);
		}
		virtual bool TrackRemoveID(const wxString& trackedID) override
		{
			return DoTrackID(trackedID, wxEmptyString, true);
		}

	public:
		virtual void Create(KPackageCreatorController* controller, wxWindow* window, wxSizer* sizer) override;
		virtual void ChangeNotify() override;

		KPPRRequirementsGroup* GetDataEntry(size_t index) const
		{
			if (index < GetItemCount())
			{
				return (*GetDataVector())[index].get();
			}
			return NULL;
		}
		KPPRRequirementsGroup* GetDataEntry(const KxDataViewItem& item) const
		{
			return GetDataEntry(GetRow(item));
		}
		
		void SetProject(KPackageProject& projectData);
		void SetEntriesModel(KPCREntriesListModel* model)
		{
			m_EntriesModel = model;
		}
};
