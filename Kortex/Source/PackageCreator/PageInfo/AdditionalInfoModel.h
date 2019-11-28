#pragma once
#include "stdafx.h"
#include "PackageCreator/VectorModel.h"
#include "PackageProject/KPackageProjectInfo.h"
#include <KxFramework/KxStdDialog.h>

namespace Kortex::PackageDesigner::PageInfoNS
{
	class AdditionalInfoModel: public VectorModel<KLabeledValue::Vector>
	{
		private:
			KPackageProjectInfo* m_InfoData = nullptr;
			bool m_UseInlineEditor = false;
			
		private:
			void OnInitControl() override;
			
			void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
			
			void OnActivateItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);
			
			void OnAddEntry();
			void OnRemoveEntry(const KxDataViewItem& item);
			void OnClearList();
			bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
			{
				OnInsertItemHelperPrimitive(*GetDataVector(), currentItem, droppedItem);
				return true;
			}
			
		public:
			const KLabeledValue* GetDataEntry(size_t index) const
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
			KLabeledValue* GetDataEntry(size_t index)
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
	
			KLabeledValue* GetDataEntry(const KxDataViewItem& item)
			{
				return GetDataEntry(GetRow(item));
			}
			const KLabeledValue* GetDataEntry(const KxDataViewItem& item) const
			{
				return GetDataEntry(GetRow(item));
			}
	
			void SetDataVector();
			void SetDataVector(VectorType& data, KPackageProjectInfo* info);
	
			void UseInlineEditor(bool value)
			{
				m_UseInlineEditor = value;
			}
	};
}

namespace Kortex::PackageDesigner::PageInfoNS
{
	class AdditionalInfoDialog: public KxStdDialog, public AdditionalInfoModel
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
	
		public:
			AdditionalInfoDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller, bool useInloneEditor = false);
			virtual ~AdditionalInfoDialog();
	};
}
