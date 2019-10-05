#pragma once
#include "stdafx.h"
#include <KxFramework/KxStdDialog.h>
class KxTextBox;

namespace Kortex::PackageDesigner
{
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
			wxWindow* m_ViewPane = nullptr;
			wxWindow* m_OKButton = nullptr;
			KxTextBox* m_InfoInput = nullptr;
			KxTextBox* m_ModuleConfigInput = nullptr;
			KxTextBox* m_ProjectFolderInput = nullptr;
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
			~KPackageCreatorFOModIEDialog();

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
}
