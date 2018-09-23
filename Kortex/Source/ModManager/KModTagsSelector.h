#pragma once
#include "stdafx.h"
#include "KImageProvider.h"
#include "KDataViewListModel.h"
#include "KLabeledValue.h"
#include <KxFramework/KxStdDialog.h>
class KModTag;
class KModEntry;
class KModController;
class KxDataViewComboBox;
class KxButton;

class KModTagsSelector: public KxDataViewListModelEx
{
	private:
		const bool m_FullFeatured = false;

	protected:
		bool m_IsModified = false;
		KModEntry* m_ModEntry = NULL;
		KxStringVector* m_Data = NULL;
		const KModTag* m_PriorityGroupTag = NULL;
		bool m_AllowSave = false;

	protected:
		virtual void OnInitControl() override;

		virtual void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
		virtual bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;

		void OnActivate(KxDataViewEvent& event);

	protected:
		const KModTag* FindStdTag(const wxString& tagID) const;
		KxStringVector::const_iterator FindTag(const KModTag* entry) const;
		bool HasTag(const KModTag* entry) const
		{
			return FindTag(entry) != m_Data->cend();
		}
		bool ToggleTag(const KModTag* entry, bool add);
		bool HasPriorityGroupTag() const
		{
			return m_PriorityGroupTag != NULL;
		}

	public:
		KModTagsSelector(bool bFullFeatured = false);

	public:
		virtual void SetDataVector(KxStringVector* data = NULL, KModEntry* modEntry = NULL);
		virtual size_t GetItemCount() const override;
		
		KModTag* GetDataEntry(size_t index) const;

		bool IsFullFeatured() const
		{
			return m_FullFeatured;
		}
		bool IsModified() const
		{
			return m_IsModified;
		}
		void ApplyChanges();
		void SetAllowSave(bool allow = true)
		{
			m_AllowSave = allow;
		}
		wxWindow* GetWindow()
		{
			return OnGetDataViewWindow();
		}
};

//////////////////////////////////////////////////////////////////////////
class KModTagsSelectorCB: public KModTagsSelector
{
	private:
		KxDataViewComboBox* m_ComboView = NULL;

	protected:
		virtual KxDataViewCtrl* OnCreateDataView(wxWindow* window) override;
		virtual wxWindow* OnGetDataViewWindow() override;
		virtual bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;
		virtual bool IsEditorEnabledByRow(size_t row, const KxDataViewColumn* column) const override;

	private:
		wxString DoGetStingValue() const;
		void SetStringValue(const wxString& value);
		void OnGetStringValue(KxDataViewEvent& event);

	public:
		virtual void SetDataVector(KxStringVector* data = NULL);
};

//////////////////////////////////////////////////////////////////////////
class KModTagsSelectorDialog: public KxStdDialog, public KModTagsSelector
{
	private:
		wxWindow* m_ViewPane = NULL;
		KxButton* m_AddButton = NULL;
		KxButton* m_RemoveButton = NULL;

	private:
		virtual wxWindow* GetDialogMainCtrl() const override
		{
			return m_ViewPane;
		}
		virtual bool IsEditorEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
		void OnSelectItem(KxDataViewEvent& event);

		void OnAddTag(wxCommandEvent& event);
		void OnRemoveTag(wxCommandEvent& event);

	public:
		KModTagsSelectorDialog(wxWindow* parent, const wxString& caption);
		virtual ~KModTagsSelectorDialog();
};
