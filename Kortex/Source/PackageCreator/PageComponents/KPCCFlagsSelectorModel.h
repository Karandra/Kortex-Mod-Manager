#pragma once
#include "stdafx.h"
#include "PackageCreator/KPackageCreatorVectorModel.h"
#include "PackageProject/KPackageProjectComponents.h"
#include "PackageProject/KPackageProjectRequirements.h"
#include "PackageCreator/KPackageCreatorIDTracker.h"
#include "KProgramOptions.h"
#include <KxFramework/KxStdDialog.h>
class KxComboBox;

class KPCCFlagsSelectorModel: public KPackageCreatorVectorModel<KPPCFlagEntryArray>, public KPackageCreatorIDTracker
{
	protected:
		KxDataViewComboBoxEditor* m_LabelEditor = NULL;
		KxDataViewComboBoxEditor* m_ValueEditor = NULL;
		KxDataViewComboBoxEditor* m_OperatorEditor = NULL;
		bool m_IsAssign = true;

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
		KPCCFlagsSelectorModel(KPackageCreatorController* controller, bool isAssign)
			:m_IsAssign(isAssign)
		{
			m_Controller = controller;
		}

	public:
		KPPCFlagEntry* GetDataEntry(size_t index)
		{
			if (index < GetItemCount())
			{
				return &GetDataVector()->at(index);
			}
			return NULL;
		}
		const KPPCFlagEntry* GetDataEntry(size_t index) const
		{
			if (index < GetItemCount())
			{
				return &GetDataVector()->at(index);
			}
			return NULL;
		}

		bool IsAssignMode() const
		{
			return m_IsAssign;
		}

		void SetDataVector();
		void SetDataVector(VectorType& data);
};

//////////////////////////////////////////////////////////////////////////
class KPCCFlagsSelectorModelDialog: public KxStdDialog, public KPCCFlagsSelectorModel
{
	protected:
		wxWindow* m_ViewPane = NULL;
		wxBoxSizer* m_Sizer = NULL;

		KProgramOptionUI m_WindowOptions;
		KProgramOptionUI m_ViewOptions;

	private:
		wxWindow* GetDialogMainCtrl() const override
		{
			return m_ViewPane;
		}

	public:
		KPCCFlagsSelectorModelDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller, bool isAssign);
		virtual ~KPCCFlagsSelectorModelDialog();
};

//////////////////////////////////////////////////////////////////////////
class KPCCFlagsTDSelectorModelDialog: public KPCCFlagsSelectorModelDialog
{
	private:
		KxComboBox* m_ComboBoxNewTD = NULL;
		KPPCEntry* m_Entry = NULL;

		KProgramOptionUI m_WindowOptions;
		KProgramOptionUI m_ViewOptions;

	private:
		void OnSelectNewTD(wxCommandEvent& event);
		wxWindow* GetDialogFocusCtrl() const override
		{
			return GetView();
		}

	public:
		KPCCFlagsTDSelectorModelDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller, KPPCEntry* entry);
		virtual ~KPCCFlagsTDSelectorModelDialog();
};
