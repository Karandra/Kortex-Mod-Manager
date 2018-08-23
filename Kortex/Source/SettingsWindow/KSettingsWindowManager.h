#pragma once
#include "stdafx.h"
#include "ConfigManager/KConfigManager.h"
#include "ConfigManager/KCMDataProviderINI.h"
#include "Profile/KProgramManagerConfig.h"
class KApp;
class KWorkspace;
class KxXMLNode;

class KSettingsWindowManager: public KConfigManager
{
	friend class KApp;

	private:
		KCMDataProviderINI m_AppConfig;
		KCMDataProviderINI m_CurrentProfileConfig;

	private:
		KCMDataProviderINI* GetProvider(KPGCFileID id);

	public:
		KImageEnum GetImageID() const override
		{
			return KIMG_APPLICATION_DETAIL;
		}
		wxString GetID() const override;
		wxString GetName() const override;

	public:
		KSettingsWindowManager(KWorkspace* workspace);
		virtual ~KSettingsWindowManager();

	public:
		void InitAppConfig();
		void InitCurrentProfileConfig();
		void InitControllerData();

		void Save();

	private:
		static KCMSampleValueArray FF_GetProgramsIndexes(KCMConfigEntryStd* configEntry, KxXMLNode& node, KProgramManagerConfig::ProgramType index);

		static KCMSampleValueArray FF_GetLanguagesList(KCMConfigEntryStd* configEntry, KxXMLNode& node);
		static KCMSampleValueArray FF_GetWorkspacesList(KCMConfigEntryStd* configEntry, KxXMLNode& node);
		static KCMSampleValueArray FF_GetMainProgramsIndexes(KCMConfigEntryStd* configEntry, KxXMLNode& node);
		static KCMSampleValueArray FF_GetPreMainProgramsIndexes(KCMConfigEntryStd* configEntry, KxXMLNode& node);

	public:
		virtual FillFunnctionType OnQueryFillFunction(const wxString& name) override;
		virtual KCMIDataProvider* OnQueryDataProvider(const KCMFileEntry* fileEntry) override;
};
