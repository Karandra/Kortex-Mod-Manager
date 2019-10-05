#pragma once
#include "stdafx.h"
#include "PackageCreator/KPackageCreatorVectorModel.h"
#include "PackageProject/KPackageProjectInfo.h"
#include <KxFramework/KxStdDialog.h>

namespace Kortex::PackageDesigner
{
	class KPCInfoSitesModel: public KPackageCreatorVectorModel<KLabeledValue::Vector>
	{
		private:
			KPackageProjectInfo* m_InfoData = nullptr;
			bool m_UseInlineEditor = false;
	
		private:
			void OnInitControl() override;
	
			void GetEditorValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
			void GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
			bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;
	
			void OnActivateItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);
	
			void OnAddSite();
			void OnRemoveSite(const KxDataViewItem& item);
			void OnClearList();
			bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
			{
				OnInsertItemHelperPrimitive(*GetDataVector(), currentItem, droppedItem);
				return true;
			}
	
		public:
			KLabeledValue* GetDataEntry(size_t index)
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
			const KLabeledValue* GetDataEntry(size_t index) const
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
	
			void SetDataVector();
			void SetDataVector(VectorType& data, KPackageProjectInfo* info);
	
			void UseInlineEditor(bool value)
			{
				m_UseInlineEditor = value;
			}
	};
}

namespace Kortex::PackageDesigner
{
	class KPCInfoSitesModelDialog: public KxStdDialog, public KPCInfoSitesModel
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
			KPCInfoSitesModelDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller, bool useInloneEditor = false);
			~KPCInfoSitesModelDialog();
	};
}
