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

			GameID TranslateGameIDToNetwork(const wxString& name);
			void LoadOptions();
			wxString GetDataFolderName() const;
			wxString GetProfileDirectory() const;

			void ReadExecutables(Utility::OperationWithProgressDialogBase* context);
			void CopySaves(Utility::OperationWithProgressDialogBase* context);
			void CopyMods(Utility::OperationWithProgressDialogBase* context);
			void ReadPlugins(Utility::OperationWithProgressDialogBase* context);
			void CopyGameConfig(Utility::OperationWithProgressDialogBase* context);
			void CopyDownloads(Utility::OperationWithProgressDialogBase* context);

		public:
			void SetDirectory(const wxString& path) override;
			void Import(Utility::OperationWithProgressDialogBase* context) override;
			
			bool CanImport() const override;
			GameID GetTargetGameID() const override
			{
				return m_TargetGameID;
			}
			wxString GetModManagerName() const override
			{
				return m_ModManagerName;
			}
			wxString GetAdditionalInfo() const override;
			wxString GetCurrentProfile() const override
			{
				return m_CurrentProfile;
			}
			KxStringVector GetAvailableProfiles() const override;
	};
}
