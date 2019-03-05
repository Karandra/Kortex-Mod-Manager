#pragma once
#include "stdafx.h"
#include "GameMods/IModImporter.h"
#include <KxFramework/KxINI.h>

namespace Kortex::ModManager
{
	class ModImporterMO: public IModImporter
	{
		private:
			wxString m_InstanceDirectory;
			wxString m_ModsDirectory;
			wxString m_ProfilesDirectory;
			wxString m_DownloadsDirectory;
			KxINI m_Options;

			wxString m_CurrentProfile;
			wxString m_ModManagerName;
			GameID m_TargetGameID;
			const IGameInstance* m_TargetInstance = nullptr;
			bool m_CanImport = false;

		private:
			wxString& DecodeUTF8(wxString& path) const;
			wxString& ProcessFilePath(wxString& path) const;
			wxString& ProcessDescription(wxString& path) const;

			GameID TranslateGameID(const wxString& name);
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
			virtual GameID GetTargetGameID() const override
			{
				return m_TargetGameID;
			}
			virtual wxString GetModManagerName() const override
			{
				return m_ModManagerName;
			}
			virtual wxString GetAdditionalInfo() const;
			virtual wxString GetCurrentProfile() const override
			{
				return m_CurrentProfile;
			}
			virtual KxStringVector GetAvailableProfiles() const override;
	};
}
