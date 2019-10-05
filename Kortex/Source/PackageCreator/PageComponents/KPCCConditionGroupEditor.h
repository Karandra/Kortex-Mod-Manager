#pragma once
#include "stdafx.h"
#include "PackageCreator/KPackageCreatorVectorModel.h"
#include "PackageProject/KPackageProjectComponents.h"
#include "PackageProject/KPackageProjectRequirements.h"
#include "PackageCreator/KPackageCreatorIDTracker.h"
#include <KxFramework/KxStdDialog.h>
class KxComboBox;

namespace Kortex::PackageDesigner
{
	class KPCCConditionGroupEditor:
		public KxDataViewModelExBase<KxDataViewModel>,
		//public KxDataViewModelExDragDropEnabled<KPackageCreatorListModelDataObject>,
		public KPackageCreatorIDTracker
	{
		protected:
			KPackageCreatorController* m_Controller = nullptr;
			KPackageProject& m_Project;
			KPPCConditionGroup& m_ConditionGroup;
	
			KxDataViewComboBoxEditor* m_LabelEditor = nullptr;
			KxDataViewComboBoxEditor* m_ValueEditor = nullptr;
			KxColor m_ConditionColor;
	
		private:
			virtual void OnInitControl() override;
	
			virtual bool IsContainer(const KxDataViewItem& item) const override;
			virtual void GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const override;
			virtual KxDataViewItem GetParent(const KxDataViewItem& item) const override;
	
			virtual bool IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			virtual bool IsEditorEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			virtual void GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			virtual void GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			virtual bool SetValue(const wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) override;
			virtual bool GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const override;
	
			void OnActivateItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);
	
			void OnAddFlag(KPPCCondition& condition);
			void OnAddCondition();
			void OnRemoveFlag(KPPCCondition& condition, KPPCFlagEntry& flag);
			void OnRemoveCondition(KPPCCondition& condition);
			void OnRemoveAllConditions();
			
			int AskRemoveOption() const;
			bool DoTrackID(wxString trackedID, const wxString& newID, bool remove) const;
			void DoRemoveFlag(KPPCCondition& condition, KPPCFlagEntry& flag);
			void DoRemoveCondition(KPPCCondition& condition);
	
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
	
			void ChangeNotify();
			void RemoveEmptyConditions();
	
		public:
			KPCCConditionGroupEditor(KPackageCreatorController* controller, KPPCConditionGroup& conditionGroup);
	
		public:
			KxDataViewItem MakeItem(const KPPCFlagEntry& flag) const
			{
				return KxDataViewItem(static_cast<const wxObject*>(&flag));
			}
			KxDataViewItem MakeItem(const KPPCCondition& condition) const
			{
				return KxDataViewItem(static_cast<const wxObject*>(&condition));
			}
			KPPCFlagEntry* GetFlagEntry(const KxDataViewItem& item) const;
			KPPCCondition* GetConditionEntry(const KxDataViewItem& item) const;
	
			virtual void RefreshItems() override;
	};
}

namespace Kortex::PackageDesigner
{
	class KPCCConditionGroupEditorDialog: public KxStdDialog, public KPCCConditionGroupEditor
	{
		protected:
			wxWindow* m_ViewPane = nullptr;
			wxBoxSizer* m_Sizer = nullptr;
			KxComboBox* m_GlobalOperatorCB = nullptr;
	
			//KProgramOptionAI m_WindowOptions;
			//KProgramOptionAI m_ViewOptions;
	
		private:
			wxWindow* GetDialogMainCtrl() const override
			{
				return m_ViewPane;
			}
			void OnSelectGlobalOperator(wxCommandEvent& event);
	
		protected:
			void LoadWindowSizes();
	
		public:
			KPCCConditionGroupEditorDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller, KPPCConditionGroup& conditionGroup);
			virtual ~KPCCConditionGroupEditorDialog();
	
		public:
			virtual int ShowModal() override;
	};
}

namespace Kortex::PackageDesigner
{
	class KPCCConditionGroupEditorDialogTD: public KPCCConditionGroupEditorDialog
	{
		private:
			KxComboBox* m_NewTypeDescriptorCB = nullptr;
			KPPCEntry& m_Entry;
	
		private:
			wxWindow* GetDialogFocusCtrl() const override
			{
				return GetView();
			}
			void OnSelectNewTypeDescriptor(wxCommandEvent& event);
	
		public:
			KPCCConditionGroupEditorDialogTD(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller, KPPCConditionGroup& conditionGroup, KPPCEntry& entry);
			virtual ~KPCCConditionGroupEditorDialogTD();
	};
}
