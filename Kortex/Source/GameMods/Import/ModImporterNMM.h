#pragma once
#include "stdafx.h"
#include "GameMods/IModImporter.h"
#include <KxFramework/KxXML.h>

namespace Kortex::ModManager
{
	class ModImporterNMM: public IModImporter
	{
		private:
			wxString m_InstanceDirectory;
			KxXMLDocument m_ProfileManagerXML;

			std::vector<std::pair<wxString, wxString>> m_ProfilesList;
			GameID m_TargetGameID;
			const IGameInstance* m_TargetGame = nullptr;
			bool m_CanImport = false;

		private:
			wxString ProcessDescription(const wxString& path) const;

			GameID GetGameID(const wxString& name);
			void LoadOptions();
			wxString GetDataFolderName() const;
			wxString GetProfileDirectory() const;

			void CopySavesAndConfig(Utility::OperationWithProgressDialogBase* context);
			void CopyMods(Utility::OperationWithProgressDialogBase* context);
			void ReadPlugins(Utility::OperationWithProgressDialogBase* context);
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
				return "Nexus Mod Manager";
			}
			wxString GetAdditionalInfo() const override;
			wxString GetCurrentProfile() const override
			{
				return wxEmptyString;
			}
			KxStringVector GetAvailableProfiles() const override;
	};
}
