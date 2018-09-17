#include "stdafx.h"
#include "KSettingsWindowManager.h"
#include "ConfigManager/KCMDataProviderINI.h"
#include "UI/KMainWindow.h"
#include "UI/KWorkspace.h"
#include "Profile/KProfile.h"
#include "Profile/KConfigManagerConfig.h"
#include "Profile/KProgramManagerConfig.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxTranslation.h>
#include <KxFramework/KxXML.h>

KCMSampleValueArray KSettingsWindowManager::FF_GetProgramsIndexes(KCMConfigEntryStd* configEntry, KxXMLNode& node, KProgramManagerConfig::ProgramType index)
{
	KCMSampleValueArray outList;
	KProgramManagerConfig* runConfig = KProgramManagerConfig::GetInstance();
	for (size_t i = 0; i < runConfig->GetEntriesCount(index); i++)
	{
		auto entry = runConfig->GetEntryAt(index, i);
		if (entry)
		{
			outList.push_back(KCMSampleValue(configEntry, std::to_string(i), entry->GetName()));
		}
	}
	return outList;
}

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
KCMSampleValueArray KSettingsWindowManager::FF_GetMainProgramsIndexes(KCMConfigEntryStd* configEntry, KxXMLNode& node)
{
	return FF_GetProgramsIndexes(configEntry, node, KProgramManagerConfig::ProgramType::Main);
}
KCMSampleValueArray KSettingsWindowManager::FF_GetPreMainProgramsIndexes(KCMConfigEntryStd* configEntry, KxXMLNode& node)
{
	return FF_GetProgramsIndexes(configEntry, node, KProgramManagerConfig::ProgramType::PreMain);
}

KCMDataProviderINI* KSettingsWindowManager::GetProvider(KPGCFileID id)
{
	switch (id)
	{
		case KPGC_ID_APP:
		{
			return &m_AppConfig;
		}
		case KPGC_ID_CURRENT_PROFILE:
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
	m_CurrentProfileConfig.Init(GetGameConfig()->GetEntry(KPGC_ID_CURRENT_PROFILE)->GetFilePath());
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
	else if (name == "GetMainProgramsIndexes")
	{
		return FF_GetMainProgramsIndexes;
	}
	else if (name == "GetPreMainProgramsIndexes")
	{
		return FF_GetPreMainProgramsIndexes;
	}
	return KConfigManager::OnQueryFillFunction(name);
}
KCMIDataProvider* KSettingsWindowManager::OnQueryDataProvider(const KCMFileEntry* fileEntry)
{
	return GetProvider(fileEntry->GetID());
}
