#pragma once
#include "stdafx.h"
#include "PackageCreator/VectorModel.h"
#include "PackageProject/InfoSection.h"
#include <KxFramework/KxStdDialog.h>

namespace Kortex::PackageDesigner::PageInfoNS
{
	class SitesModel: public VectorModel<Utility::LabeledValue::Vector>
	{
		private:
			PackageProject::InfoSection* m_InfoData = nullptr;
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
			Utility::LabeledValue* GetDataEntry(size_t index)
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
			const Utility::LabeledValue* GetDataEntry(size_t index) const
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
	
			void SetDataVector();
			void SetDataVector(VectorType& data, PackageProject::InfoSection* info);
	
			void UseInlineEditor(bool value)
			{
				m_UseInlineEditor = value;
			}
	};
}

namespace Kortex::PackageDesigner::PageInfoNS
{
	class SitesDialog: public KxStdDialog, public SitesModel
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
			SitesDialog(wxWindow* parent, const wxString& caption, WorkspaceDocument* controller, bool useInloneEditor = false);
			~SitesDialog();
	};
}
