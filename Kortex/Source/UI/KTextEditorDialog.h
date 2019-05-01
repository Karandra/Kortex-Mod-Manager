#pragma once
#include "stdafx.h"
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxStyledTextBox.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxStdDialog.h>
class KxBitmapComboBox;

class KTextEditorDialog: public KxStdDialog
{
	private:
		KxPanel* m_View = nullptr;
		wxSimplebook* m_Tabs = nullptr;
		wxString m_Text;
		bool m_TextModified = false;

		bool m_EditMode = true;
		bool m_Editable = true;
		KxAuiToolBar* m_ToolBar = nullptr;
		KxAuiToolBarItem* m_ToolBar_SwitchMode = nullptr;
		KxAuiToolBarItem* m_ToolBar_Save = nullptr;
		KxAuiToolBarItem* m_ToolBar_Open = nullptr;
		KxBitmapComboBox* m_HeadingList = nullptr;
		KxStyledTextBox* m_Editor = nullptr;

		KxHTMLWindow* m_Preview = nullptr;

	private:
		bool Create(wxWindow* parent);

	private:
		virtual int GetViewSizerProportion() const override
		{
			return 1;
		}
		virtual wxOrientation GetViewSizerOrientation() const override
		{
			return wxVERTICAL;
		}
		virtual wxOrientation GetViewLabelSizerOrientation() const override
		{
			return wxHORIZONTAL;
		}
		virtual bool IsEnterAllowed(wxKeyEvent& event, wxWindowID* id = nullptr) const override
		{
			return true;
		}
		virtual wxWindow* GetDialogMainCtrl() const override
		{
			return m_View;
		}
		virtual void ResetState()
		{
			m_Editor->Clear();
			m_Preview->Clear();
		}

		void OnNewTextSet();
		void OnPrepareSaveText();
		void ClearUndoHistory();
		void OnSwitchMode(KxAuiToolBarEvent& event);
		void OnKey(wxKeyEvent& event);
		void OnOK(wxNotifyEvent& event);
		void OnSaveLoadFile(KxAuiToolBarEvent& event);

		void ToggleTag(const wxString& tagStart, const wxString& tagEnd);
		void ToggleTag(const wxString& tagName, const wxString& attributeName, const wxString& attributeValue)
		{
			ToggleTag(wxString::Format(" %s=\"%s\"", attributeName, attributeValue), tagName);
		}
		void ToggleTag(const wxString& tagName)
		{
			ToggleTag(tagName, tagName);
		}
		void LoadFromFile(const wxString& filePath);
		void SaveToFile(const wxString& filePath) const;
		void DoShowPreview(bool show);

	public:
		KTextEditorDialog(wxWindow* parent);
		virtual ~KTextEditorDialog();

	public:
		virtual int ShowModal() override;

	public:
		const wxString& GetText() const;
		void SetText(const wxString& text);
		bool IsModified() const
		{
			return m_TextModified;
		}
		void ShowPreview(bool show)
		{
			m_EditMode = !show;
		}
		bool IsEditable() const
		{
			return m_Editable;
		}
		void SetEditable(bool value)
		{
			m_Editable = value;
		}
};
