#pragma once
#include "stdafx.h"
#include <KxFramework/KxTextBoxDialog.h>

namespace Kortex::ModManager
{
	class NewModDialog: public KxTextBoxDialog
	{
		private:
			wxString m_Name;

		private:
			void OnOK(wxNotifyEvent& event);

		public:
			NewModDialog(wxWindow* parent);
			virtual ~NewModDialog();

		public:
			wxString GetFolderName() const
			{
				return m_Name;
			}
	};
}

namespace Kortex::ModManager
{
	class NewModFromFolderDialog: public NewModDialog
	{
		private:
			wxCheckBox* m_AsLinkedModCB = nullptr;

			bool m_IsLinkedMod = false;
			wxString m_FolderPath;

		private:
			void OnSelectFolder(wxNotifyEvent& event);
			void OnChangeMethod(wxCommandEvent& event);

			wxString GetMethodString(bool bLink) const;
			wxString GetMethodString() const
			{
				return GetMethodString(m_AsLinkedModCB->GetValue());
			}
			void UpdateLabelText();

		public:
			NewModFromFolderDialog(wxWindow* parent);
			virtual ~NewModFromFolderDialog();

		public:
			wxString GetFolderPath() const
			{
				return m_FolderPath;
			}
			bool ShouldCreateAsLinkedMod() const
			{
				return m_IsLinkedMod;
			}
	};
}
