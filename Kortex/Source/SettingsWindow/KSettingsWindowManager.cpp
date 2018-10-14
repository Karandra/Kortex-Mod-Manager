#include "stdafx.h"
#include "KSettingsWindowManager.h"
#include "ConfigManager/KCMDataProviderINI.h"
#include "UI/KMainWindow.h"
#include "UI/KWorkspace.h"
#include "GameInstance/KGameInstance.h"
#include "GameInstance/Config/KConfigManagerConfig.h"
#include "GameInstance/Config/KProgramManagerConfig.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxTranslation.h>
#include <KxFramework/KxXML.h>

KCMSampleValueArray KSettingsWindowManager::FF_GetLanguagesList(KCMConfigEntryStd* configEntry, KxXMLNode& node)
{
	KCMSampleValueArray outList;
	for (const auto& v: KApp::Get().GetAvailableTranslations())
	{
		outList.push_back(KCMSampleValue(configEntry, v.first, KxTranslation::GetLanguageFullName(v.first)));
	}
	return outList;
}
KCMSampleValueArray KSettingsWindowManager::FF_GetWorkspacesList(KCMConfigEntryStd* configEntry, KxXMLNode& node)
{
	KCMSampleValueArray outList;
	const auto& tWorkspaces = KMainWindow::GetInstance()->GetWorkspacesList();
	for (const auto& v: tWorkspaces)
	{
		if (v.second->CanBeStartPage())
		{
			outList.push_back(KCMSampleValue(configEntry, v.first, v.second->GetName()));
		}
	}
	return outList;
}
KCMSampleValueArray KSettingsWindowManager::FF_GetProgramIndexes(KCMConfigEntryStd* configEntry, KxXMLNode& node)
{
	KCMSampleValueArray outList;
	if (KProgramManagerConfig* programManager = KProgramManagerConfig::GetInstance())
	{
		size_t i = 0;
		for (const KProgramManagerEntry& entry: programManager->GetPrograms())
		{
			outList.push_back(KCMSampleValue(configEntry, KxFormat("%1").arg(i), entry.GetName()));
			i++;
		}
	}
	return outList;
}

KCMDataProviderINI* KSettingsWindowManager::GetProvider(KPGCFileID id)
{
	switch (id)
	{
		case KPGC_ID_APP:
		{
			return &m_AppConfig;
		}
		case KPGC_ID_CURRENT_INSTANCE:
		{
			return &m_CurrentProfileConfig;
		}
	};
	return NULL;
}

wxString KSettingsWindowManager::GetID() const
{
	return "KSettingsWindowManager";
}
wxString KSettingsWindowManager::GetName() const
{
	return T("Settings.Caption");
}

KSettingsWindowManager::KSettingsWindowManager(KWorkspace* workspace)
	:KConfigManager(workspace)
{
	InitAppConfig();
}
KSettingsWindowManager::~KSettingsWindowManager()
{
}

void KSettingsWindowManager::InitAppConfig()
{
	m_AppConfig.Init(KApp::Get().GetUserSettingsFile());
	m_AppConfig.Load();
}
void KSettingsWindowManager::InitCurrentProfileConfig()
{
	m_CurrentProfileConfig.Init(GetGameConfig()->GetEntry(KPGC_ID_CURRENT_INSTANCE)->GetFilePath());
	m_CurrentProfileConfig.Load();
}
void KSettingsWindowManager::InitControllerData()
{
	// Load config entries for controller
	m_FilePath = GetConfigFile("Settings");
	KxFileStream xmlStream(m_FilePath, KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING);
	m_XML.Load(xmlStream);

	LoadMainFile(m_XML);
}

void KSettingsWindowManager::Save() const
{
	m_AppConfig.Save();
	m_CurrentProfileConfig.Save();
}

KConfigManager::FillFunnctionType KSettingsWindowManager::OnQueryFillFunction(const wxString& name)
{
	if (name == "GetLanguagesList")
	{
		return FF_GetLanguagesList;
	}
	else if (name == "GetWorkspacesList")
	{
		return FF_GetWorkspacesList;
	}
	else if (name == "GetProgramsIndexes")
	{
		return FF_GetProgramIndexes;
	}
	return KConfigManager::OnQueryFillFunction(name);
}
KCMIDataProvider* KSettingsWindowManager::OnQueryDataProvider(const KCMFileEntry* fileEntry)
{
	return GetProvider(fileEntry->GetID());
}
