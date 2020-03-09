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
	class ConditionFlagsModel: public VectorModel<PackageProject::FlagItem::Vector>, public IDTracker
	{
		protected:
			PackageProject::Condition* m_Condition = nullptr;
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
			ConditionFlagsModel(WorkspaceDocument* controller)
			{
				m_Controller = controller;
			}
			
		public:
			PackageProject::FlagItem* GetDataEntry(size_t index)
			{
				if (index < GetItemCount())
				{
					return &(*GetDataVector())[index];
				}
				return nullptr;
			}
			const PackageProject::FlagItem* GetDataEntry(size_t index) const
			{
				if (index < GetItemCount())
				{
					return &(*GetDataVector())[index];
				}
				return nullptr;
			}
	
			void SetDataVector();
			void SetDataVector(PackageProject::Condition& data);
	};
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	class ConditionFlagsDialog: public KxStdDialog, public ConditionFlagsModel
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
			ConditionFlagsDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller);
			~ConditionFlagsDialog();
	};
}
