#pragma once
#include "stdafx.h"
#include <KxFramework/KxStdDialog.h>
class KxTextBox;

class KPackageCreatorFOModIEDialog: public KxStdDialog
{
	private:
		enum Type
		{
			InfoXML,
			ModuleConfigXML,
			ProjectFolder,
		};

	private:
		wxWindow* m_ViewPane = NULL;
		wxWindow* m_OKButton = NULL;
		KxTextBox* m_InfoInput = NULL;
		KxTextBox* m_ModuleConfigInput = NULL;
		KxTextBox* m_ProjectFolderInput = NULL;
		bool m_IsExport = false;

		wxString m_InfoFile;
		wxString m_ModuleConfigFile;
		wxString m_ProjectFolder;

	private:
		wxWindow* GetDialogMainCtrl() const override
		{
			return m_ViewPane;
		}
		void OnText(wxCommandEvent& event);
		void OnBrowseFile(wxCommandEvent& event);
		void OnOK(wxNotifyEvent& event);

	public:
		KPackageCreatorFOModIEDialog(wxWindow* parent, bool isExport);
		virtual ~KPackageCreatorFOModIEDialog();

	public:
		const wxString& GetInfoFile() const
		{
			return m_InfoFile;
		}
		const wxString& GetModuleConfigFile() const
		{
			return m_ModuleConfigFile;
		}
		const wxString& GetProjectFolder() const
		{
			return m_ProjectFolder;
		}
};
