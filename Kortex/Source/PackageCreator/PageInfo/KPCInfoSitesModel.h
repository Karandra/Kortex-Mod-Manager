#pragma once
#include "stdafx.h"
#include "PackageCreator/KPackageCreatorVectorModel.h"
#include "PackageProject/KPackageProjectInfo.h"
#include "KProgramOptions.h"
#include <KxFramework/KxStdDialog.h>

class KPCInfoSitesModel: public KPackageCreatorVectorModel<KLabeledValueArray>
{
	private:
		KPackageProjectInfo* m_InfoData = NULL;
		bool m_UseInlineEditor = false;

	private:
		virtual void OnInitControl() override;

		virtual void GetEditorValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
		virtual void GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;

		void OnActivateItem(KxDataViewEvent& event);
		void OnContextMenu(KxDataViewEvent& event);

		void OnAddSite();
		void OnRemoveSite(const KxDataViewItem& item);
		void OnClearList();
		virtual bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
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
			return NULL;
		}
		const KLabeledValue* GetDataEntry(size_t index) const
		{
			if (index < GetItemCount())
			{
				return &GetDataVector()->at(index);
			}
			return NULL;
		}

		void SetDataVector();
		void SetDataVector(VectorType& data, KPackageProjectInfo* info);

		void UseInlineEditor(bool value)
		{
			m_UseInlineEditor = value;
		}
};

//////////////////////////////////////////////////////////////////////////
class KPCInfoSitesModelDialog: public KxStdDialog, public KPCInfoSitesModel
{
	private:
		wxWindow* m_ViewPane = NULL;
		KProgramOptionUI m_WindowOptions;
		KProgramOptionUI m_ViewOptions;

	private:
		wxWindow* GetDialogMainCtrl() const override
		{
			return m_ViewPane;
		}

	public:
		KPCInfoSitesModelDialog(wxWindow* parent, const wxString& caption, KPackageCreatorController* controller, bool useInloneEditor = false);
		virtual ~KPCInfoSitesModelDialog();
};
