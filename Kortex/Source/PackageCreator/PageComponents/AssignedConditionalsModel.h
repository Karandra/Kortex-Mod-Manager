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
	class AssignedConditionalsModel: public VectorModel<KPPCFlagEntry::Vector>, public IDTracker
	{
		protected:
			KPPCCondition* m_Condition = nullptr;
			KxDataViewComboBoxEditor* m_LabelEditor = nullptr;
			KxDataViewComboBoxEditor* m_ValueEditor = nullptr;
			
		private:
			void OnInitControl() override;
	
			void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
	
			void OnActivateItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);
	
			void OnAddFlag();
			void OnRemoveFlag(const KxDataViewItem& item);
			void OnClearList();
			bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
			{
				OnInsertItemHelperPrimitive(*GetDataVector(), currentItem, droppedItem);
				return true;
			}
			
			bool DoTrackID(wxString trackedID, const wxString& newID, bool remove) const;
	
		protected:
			IDTracker* GetTracker() override
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
	
		public:
			AssignedConditionalsModel(WorkspaceDocument* controller)
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
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	class AssignedConditionalsDialog: public KxStdDialog, public AssignedConditionalsModel
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
			AssignedConditionalsDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller);
			~AssignedConditionalsDialog();
	};
}
