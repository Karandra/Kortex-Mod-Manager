#pragma once
#include "stdafx.h"
#include "KModManagerImport.h"
#include <KxFramework/KxINI.h>

class KModManagerImportMO: public KModManagerImport
{
	private:
		wxString m_InstanceDirectory;
		wxString m_ModsDirectory;
		wxString m_ProfilesDirectory;
		wxString m_DownloadsDirectory;
		KxINI m_Options;

		KGameID m_TargetProfile;
		const KGameInstance* m_TargetProfileTemplate = NULL;
		wxString m_CurrentModList;
		wxString m_ModManagerName;
		bool m_CanImport = false;

	private:
		wxString& DecodeUTF8(wxString& path) const;
		wxString& ProcessFilePath(wxString& path) const;
		wxString& ProcessDescription(wxString& path) const;

		KGameID GetGameID(const wxString& name);
		void LoadOptions();
		wxString GetDataFolderName() const;
		wxString GetProfileDirectory() const;

		void ReadExecutables(KOperationWithProgressDialogBase* context);
		void CopySaves(KOperationWithProgressDialogBase* context);
		void CopyMods(KOperationWithProgressDialogBase* context);
		void ReadPlugins(KOperationWithProgressDialogBase* context);
		void CopyGameConfig(KOperationWithProgressDialogBase* context);
		void CopyDownloads(KOperationWithProgressDialogBase* context);

	public:
		virtual void SetDirectory(const wxString& path) override;
		virtual void Import(KOperationWithProgressDialogBase* context) override;
		
		virtual bool CanImport() const override;
		virtual KGameID GetTargetProfileID() const override
		{
			return m_TargetProfile;
		}
		virtual wxString GetModManagerName() const override
		{
			return m_ModManagerName;
		}
		virtual wxString GetAdditionalInfo() const;
		virtual wxString GetCurrentModList() const override
		{
			return m_CurrentModList;
		}
		virtual KxStringVector GetProfilesList() const override;
};
