#pragma once
#include "stdafx.h"
#include "PackageCreator/VectorModel.h"
#include "PackageProject/ComponentsSection.h"
#include "PackageProject/RequirementsSection.h"
#include "PackageCreator/IDTracker.h"
#include <KxFramework/KxStdDialog.h>
class KxComboBox;

namespace Kortex::PackageDesigner::PageComponentsNS
{
	class ConditionGroupModel:
		public KxDataViewModelExBase<KxDataViewModel>,
		public IDTracker
	{
		protected:
			WorkspaceDocument* m_Controller = nullptr;
			ModPackageProject& m_Project;
			PackageProject::ConditionGroup& m_ConditionGroup;
	
			KxDataViewComboBoxEditor* m_LabelEditor = nullptr;
			KxDataViewComboBoxEditor* m_ValueEditor = nullptr;
			KxColor m_ConditionColor;
	
		private:
			void OnInitControl() override;
	
			bool IsContainer(const KxDataViewItem& item) const override;
			void GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const override;
			KxDataViewItem GetParent(const KxDataViewItem& item) const override;
	
			bool IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			bool IsEditorEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			void GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			void GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			bool SetValue(const wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) override;
			bool GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const override;
	
			void OnActivateItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);
	
			void OnAddFlag(PackageProject::Condition& condition);
			void OnAddCondition();
			void OnRemoveFlag(PackageProject::Condition& condition, PackageProject::FlagItem& flag);
			void OnRemoveCondition(PackageProject::Condition& condition);
			void OnRemoveAllConditions();
			
			int AskRemoveOption() const;
			bool DoTrackID(wxString trackedID, const wxString& newID, bool remove) const;
			void DoRemoveFlag(PackageProject::Condition& condition, PackageProject::FlagItem& flag);
			void DoRemoveCondition(PackageProject::Condition& condition);
	
		protected:
			IDTracker* GetTracker()
			{
				return this;
			}
			bool TrackChangeID(const wxString& trackedID, const wxString& newID) override
			{
				return DoTrackID(trackedID, newID, false);
			}
			bool TrackRemoveID(const wxString& trackedID) override
			{
				return DoTrackID(trackedID, wxEmptyString, true);
			}
	
			void ChangeNotify();
			void RemoveEmptyConditions();
	
		public:
			ConditionGroupModel(WorkspaceDocument* controller, PackageProject::ConditionGroup& conditionGroup);
	
		public:
			KxDataViewItem MakeItem(const PackageProject::FlagItem& flag) const
			{
				return KxDataViewItem(static_cast<const wxObject*>(&flag));
			}
			KxDataViewItem MakeItem(const PackageProject::Condition& condition) const
			{
				return KxDataViewItem(static_cast<const wxObject*>(&condition));
			}
			PackageProject::FlagItem* GetFlagEntry(const KxDataViewItem& item) const;
			PackageProject::Condition* GetConditionEntry(const KxDataViewItem& item) const;
	
			void RefreshItems() override;
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
			ConditionGroupDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller, PackageProject::ConditionGroup& conditionGroup);
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
			PackageProject::ComponentItem& m_Entry;
	
		private:
			wxWindow* GetDialogFocusCtrl() const override
			{
				return GetView();
			}
			void OnSelectNewTypeDescriptor(wxCommandEvent& event);
	
		public:
			ConditionGroupDialogWithTypeDescriptor(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller, PackageProject::ConditionGroup& conditionGroup, PackageProject::ComponentItem& entry);
			virtual ~ConditionGroupDialogWithTypeDescriptor();
	};
}
