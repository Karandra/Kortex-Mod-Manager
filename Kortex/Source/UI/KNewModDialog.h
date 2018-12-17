#pragma once
#include "stdafx.h"
#include <KxFramework/KxTextBoxDialog.h>

class KModController;
class KNewModDialog: public KxTextBoxDialog
{
	private:
		wxString m_Name;

	private:
		void OnOK(wxNotifyEvent& event);

	public:
		KNewModDialog(wxWindow* parent);
		virtual ~KNewModDialog();

	public:
		const wxString& GetFolderName() const
		{
			return m_Name;
		}
};

//////////////////////////////////////////////////////////////////////////
class KNewModDialogEx: public KNewModDialog
{
	private:
		wxCheckBox* m_LinkedModCheckBox = nullptr;

		bool m_IsLinkedMod = false;
		wxString m_FolderPath;

	private:
		void OnSelectFolder(wxNotifyEvent& event);
		void OnChangeMethod(wxCommandEvent& event);

		wxString GetMethodString(bool bLink) const;
		wxString GetMethodString() const
		{
			return GetMethodString(m_LinkedModCheckBox->GetValue());
		}
		void UpdateLabelText();

	public:
		KNewModDialogEx(wxWindow* parent);
		virtual ~KNewModDialogEx();

	public:
		const wxString& GetFolderPath() const
		{
			return m_FolderPath;
		}
		bool IsCreateAsLinkedMod() const
		{
			return m_IsLinkedMod;
		}
};
