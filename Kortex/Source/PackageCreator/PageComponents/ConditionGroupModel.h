#pragma once
#include "stdafx.h"
#include "PackageCreator/VectorModel.h"
#include "PackageProject/KPackageProjectComponents.h"
#include "PackageProject/KPackageProjectRequirements.h"
#include "PackageCreator/IDTracker.h"
#include <KxFramework/KxStdDialog.h>
class KxComboBox;

namespace Kortex::PackageDesigner::PageComponentsNS
{
	class ConditionGroupModel:
		public KxDataViewModelExBase<KxDataViewModel>,
		//public KxDataViewModelExDragDropEnabled<ListModelDataObject>,
		public IDTracker
	{
		protected:
			WorkspaceDocument* m_Controller = nullptr;
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
			virtual IDTracker* GetTracker()
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
			ConditionGroupModel(WorkspaceDocument* controller, KPPCConditionGroup& conditionGroup);
	
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

namespace Kortex::PackageDesigner::PageComponentsNS
{
	class ConditionGroupDialog: public KxStdDialog, public ConditionGroupModel
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
			ConditionGroupDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller, KPPCConditionGroup& conditionGroup);
			virtual ~ConditionGroupDialog();
	
		public:
			virtual int ShowModal() override;
	};
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	class ConditionGroupDialogWithTypeDescriptor: public ConditionGroupDialog
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
			ConditionGroupDialogWithTypeDescriptor(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller, KPPCConditionGroup& conditionGroup, KPPCEntry& entry);
			virtual ~ConditionGroupDialogWithTypeDescriptor();
	};
}
