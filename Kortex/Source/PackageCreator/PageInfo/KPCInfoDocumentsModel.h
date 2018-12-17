#pragma once
#include "stdafx.h"
#include "PackageCreator/KPackageCreatorVectorModel.h"
#include "PackageProject/KPackageProjectInfo.h"
#include <KxFramework/KxStdDialog.h>

class KPCInfoDocumentsModel: public KPackageCreatorVectorModel<KLabeledValue::Vector>
{
	private:
		KPackageProjectInfo* m_InfoData = nullptr;

	private:
		virtual void OnInitControl() override;

		virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;

		void OnActivateItem(KxDataViewEvent& event);
		void OnContextMenu(KxDataViewEvent& event);

		void OnAddEntry();
		void OnRemoveEntry(const KxDataViewItem& item);
		void OnClearList();
		KxStringVector OpenFileDialog(bool isMultiple = true) const;
		virtual bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
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

		void SetDataVector();
		void SetDataVector(VectorType& data, KPackageProjectInfo* info);
};

//////////////////////////////////////////////////////////////////////////
class KPCInfoDocumentsModelDialog: public KxStdDialog, public KPCInfoDocumentsModel
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
		KPCInfoDocumentsModelDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller);
		virtual ~KPCInfoDocumentsModelDialog();
};
