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

			bool m_CanImport = false;
			GameID m_TargetGameID;
			const IGameInstance* m_TargetGame = nullptr;
			std::vector<std::pair<wxString, wxString>> m_ProfilesList;

		private:
			wxString ProcessDescription(const wxString& path) const;

			GameID GetGameID(const wxString& name);
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
			virtual GameID GetTargetGameID() const override
			{
				return m_TargetGameID;
			}
			virtual wxString GetModManagerName() const override
			{
				return "Nexus Mod Manager";
			}
			virtual wxString GetAdditionalInfo() const;
			virtual wxString GetCurrentProfile() const override
			{
				return wxEmptyString;
			}
			virtual KxStringVector GetAvailableProfiles() const override;
	};
}
