#pragma once
#include "stdafx.h"
#include "PackageCreator/KPackageCreatorVectorModel.h"
#include "PackageProject/KPackageProjectComponents.h"
#include "PackageProject/KPackageProjectRequirements.h"
#include "PackageCreator/KPackageCreatorIDTracker.h"
#include <KxFramework/KxStdDialog.h>
class KxComboBox;

class KPCCAssignedConditionalsEditor: public KPackageCreatorVectorModel<KPPCFlagEntry::Vector>, public KPackageCreatorIDTracker
{
	protected:
		KPPCCondition* m_Condition = nullptr;
		KxDataViewComboBoxEditor* m_LabelEditor = nullptr;
		KxDataViewComboBoxEditor* m_ValueEditor = nullptr;

	private:
		virtual void OnInitControl() override;

		virtual void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;

		void OnActivateItem(KxDataViewEvent& event);
		void OnContextMenu(KxDataViewEvent& event);

		void OnAddFlag();
		void OnRemoveFlag(const KxDataViewItem& item);
		void OnClearList();
		virtual bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
		{
			OnInsertItemHelperPrimitive(*GetDataVector(), currentItem, droppedItem);
			return true;
		}
		
		bool DoTrackID(wxString trackedID, const wxString& newID, bool remove) const;

	protected:
		virtual KPackageCreatorIDTracker* GetTracker()
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
		KPCCAssignedConditionalsEditor(KPackageCreatorController* controller)
		{
			m_Controller = controller;
		}

	public:
		KPPCFlagEntry* GetDataEntry(size_t index)
		{
			if (index < GetItemCount())
			{
				return &(*GetDataVector())[index];
			}
			return nullptr;
		}
		const KPPCFlagEntry* GetDataEntry(size_t index) const
		{
			if (index < GetItemCount())
			{
				return &(*GetDataVector())[index];
			}
			return nullptr;
		}

		void SetDataVector();
		void SetDataVector(KPPCCondition& data);
};

//////////////////////////////////////////////////////////////////////////
class KPCCAssignedConditionalsEditorDialog: public KxStdDialog, public KPCCAssignedConditionalsEditor
{
	protected:
		wxWindow* m_ViewPane = nullptr;
		wxBoxSizer* m_Sizer = nullptr;

		//KProgramOptionAI m_WindowOptions;
		//KProgramOptionAI m_ViewOptions;

	private:
		wxWindow* GetDialogMainCtrl() const override
		{
			return m_ViewPane;
		}

	public:
		KPCCAssignedConditionalsEditorDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller);
		virtual ~KPCCAssignedConditionalsEditorDialog();
};
