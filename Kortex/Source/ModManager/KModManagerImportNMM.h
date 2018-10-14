#pragma once
#include "stdafx.h"
#include "KModManagerImport.h"
#include <KxFramework/KxXML.h>

class KModManagerImportNMM: public KModManagerImport
{
	private:
		wxString m_InstanceDirectory;
		KxXMLDocument m_ProfileManagerXML;

		bool m_CanImport = false;
		KGameID m_TargetProfile;
		const KGameInstance* m_TargetProfileTemplate = NULL;
		std::vector<std::pair<wxString, wxString>> m_ProfilesList;

	private:
		wxString ProcessDescription(const wxString& path) const;

		KGameID GetGameID(const wxString& name);
		void LoadOptions();
		wxString GetDataFolderName() const;
		wxString GetProfileDirectory() const;

		void CopySavesAndConfig(KOperationWithProgressDialogBase* context);
		void CopyMods(KOperationWithProgressDialogBase* context);
		void ReadPlugins(KOperationWithProgressDialogBase* context);
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
			return "Nexus Mod Manager";
		}
		virtual wxString GetAdditionalInfo() const;
		virtual wxString GetCurrentModList() const override
		{
			return wxEmptyString;
		}
		virtual KxStringVector GetProfilesList() const override;
};
