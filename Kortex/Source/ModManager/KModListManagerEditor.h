#pragma once
#include "stdafx.h"
#include "KImageProvider.h"
#include "KDataViewListModel.h"
#include "KModEntry.h"
#include "KModListManager.h"
#include "KLabeledValue.h"
#include <KxFramework/KxStdDialog.h>
class KxButton;

class KModListManagerEditor: public KxDataViewVectorListModelEx<KModListManager::ListVector, KxDataViewListModelEx>
{
	private:
		bool m_IsModified = false;
		wxString m_CurrentList;

	protected:
		virtual void OnInitControl() override;

		virtual void GetEditorValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
		virtual void GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;
		virtual bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;

		void OnActivate(KxDataViewEvent& event);
		void MarkModified()
		{
			m_IsModified = true;
		}
		void SetCurrentList(const wxString& id)
		{
			m_CurrentList = id;
		}

	public:
		KModListManagerEditor();

	public:
		bool IsModified() const
		{
			return m_IsModified;
		}
		const wxString& GetCurrentList() const
		{
			return m_CurrentList;
		}

		const KModList* GetDataEntry(size_t i) const
		{
			if (i < GetItemCount())
			{
				return &GetDataVector()->at(i);
			}
			return NULL;
		}
		KModList* GetDataEntry(size_t i)
		{
			if (i < GetItemCount())
			{
				return &GetDataVector()->at(i);
			}
			return NULL;
		}
};

//////////////////////////////////////////////////////////////////////////
class KModListManagerEditorDialog: public KxStdDialog, public KModListManagerEditor
{
	private:
		wxWindow* m_ViewPane = NULL;
		KxButton* m_AddButton = NULL;
		KxButton* m_CopyButton = NULL;
		KxButton* m_RemoveButton = NULL;

	private:
		virtual wxWindow* GetDialogMainCtrl() const override
		{
			return m_ViewPane;
		}
		
		void OnSelectItem(KxDataViewEvent& event);
		void OnAddList(wxCommandEvent& event);
		void OnCopyList(wxCommandEvent& event);
		void OnRemoveList(wxCommandEvent& event);

	public:
		KModListManagerEditorDialog(wxWindow* parent);
		virtual ~KModListManagerEditorDialog();
};
