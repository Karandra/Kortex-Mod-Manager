#pragma once
#include "stdafx.h"
#include "PackageCreator/VectorModel.h"
#include "PackageProject/KPackageProjectComponents.h"
#include "PackageProject/KPackageProjectFileData.h"
#include <KxFramework/KxStdDialog.h>

namespace Kortex::PackageDesigner::PageComponentsNS
{
	class ConditionalStepsModel: public VectorModel<PackageProject::KPPCConditionalStep::Vector>
	{
		private:
			void OnInitControl() override;
	
			void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
	
			void OnActivateItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);
	
			void OnAddStep();
			void OnRemoveStep(const KxDataViewItem& item);
			void OnClearList();
			bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
			{
				OnInsertItemHelperUniquePtr(*GetDataVector(), currentItem, droppedItem);
				return true;
			}
	
		public:
			ConditionalStepsModel(WorkspaceDocument* controller)
			{
				m_Controller = controller;
			}
	
		public:
			PackageProject::KPPCConditionalStep* GetDataEntry(size_t index) const
			{
				if (index < GetItemCount())
				{
					return GetDataVector()->at(index).get();
				}
				return nullptr;
			}
	
			void SetDataVector();
			void SetDataVector(VectorType& data);
	};
}

namespace Kortex::PackageDesigner::PageComponentsNS
{
	class ConditionalStepsDialog: public KxStdDialog, public ConditionalStepsModel
	{
		private:
			wxWindow* m_ViewPane = nullptr;
			//KProgramOptionAI m_WindowOptions;
			//KProgramOptionAI m_ViewOptions;
	
		private:
			wxWindow* GetDialogMainCtrl() const override
			{
				return m_ViewPane;
			}
			wxWindow* GetDialogFocusCtrl() const override
			{
				return GetView();
			}
	
		public:
			ConditionalStepsDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller, bool isAutomatic);
			~ConditionalStepsDialog();
	};
}
