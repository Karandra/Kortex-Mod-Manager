#pragma once
#include "stdafx.h"
#include "ConfigManager/KConfigManager.h"
#include "ConfigManager/KCMDataProviderINI.h"
#include "GameInstance/Config/KProgramManagerConfig.h"
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

		virtual void Save() const override;

	private:
		static KCMSampleValueArray FF_GetLanguagesList(KCMConfigEntryStd* configEntry, KxXMLNode& node);
		static KCMSampleValueArray FF_GetWorkspacesList(KCMConfigEntryStd* configEntry, KxXMLNode& node);
		static KCMSampleValueArray FF_GetProgramIndexes(KCMConfigEntryStd* configEntry, KxXMLNode& node);

	public:
		virtual FillFunnctionType OnQueryFillFunction(const wxString& name) override;
		virtual KCMIDataProvider* OnQueryDataProvider(const KCMFileEntry* fileEntry) override;
};
