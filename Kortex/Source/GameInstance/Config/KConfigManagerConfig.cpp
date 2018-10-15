#include "stdafx.h"
#include "ConfigManager/KConfigManager.h"
#include "KConfigManagerConfig.h"
#include "GameInstance/KGameInstance.h"
#include "KApp.h"
#include <KxFramework/KxXML.h>

//////////////////////////////////////////////////////////////////////////
KPGCFormat KConfigManagerConfigEntry::NameToFormatID(const wxString& format)
{
	if (format == "INI")
	{
		return KPGC_FORMAT_INI;
	}
	return KPGC_FORMAT_INVALID;
}

KConfigManagerConfigEntry::KConfigManagerConfigEntry(KxXMLNode& node)
	:m_TypeDetectorID(KCM_DETECTOR_INVALID)
{
	m_ID = (KPGCFileID)node.GetAttributeInt("ID", KPGC_ID_INVALID);
	if (m_ID != KPGC_ID_INVALID)
	{
		m_Format = NameToFormatID(node.GetAttribute("Format"));
		m_TypeDetectorID = KConfigManager::GetTypeDetectorID(node.GetAttribute("TypeDetectionMethod"));
		m_FilePath = KApp::Get().ExpandVariables(node.GetValue());
	}
	else
	{
		m_ID = KPGC_ID_INVALID;
	}
}
KConfigManagerConfigEntry::KConfigManagerConfigEntry(KPGCFileID id, KPGCFormat format, KCMTypeDetector typeDetectorID, const wxString& filePath)
	:m_ID(id), m_Format(format), m_TypeDetectorID(typeDetectorID), m_FilePath(filePath)
{
}

//////////////////////////////////////////////////////////////////////////
KConfigManagerConfig::KConfigManagerConfig(KGameInstance& profile, const KxXMLNode& node)
	:m_EnableENB(node.GetAttributeBool("EnableENB"))
{
	for (KxXMLNode entryNode = node.GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
	{
		m_Entries.push_back(KConfigManagerConfigEntry(entryNode));
	}

	if (m_EnableENB)
	{
		m_Entries.push_back(KConfigManagerConfigEntry(KPGC_ID_ENB_LOCAL, KPGC_FORMAT_INI, KCM_DETECTOR_DATA_ANALYSIS, "ENBLocal.ini"));
		m_Entries.push_back(KConfigManagerConfigEntry(KPGC_ID_ENB_SERIES, KPGC_FORMAT_INI, KCM_DETECTOR_DATA_ANALYSIS, "ENBSeries.ini"));
	}
	m_Entries.push_back(KConfigManagerConfigEntry(KPGC_ID_APP, KPGC_FORMAT_INI, KCM_DETECTOR_HUNGARIAN_NOTATION, KApp::Get().GetUserSettingsFile()));
	m_Entries.push_back(KConfigManagerConfigEntry(KPGC_ID_INSTANCE, KPGC_FORMAT_INI, KCM_DETECTOR_HUNGARIAN_NOTATION, profile.GetConfigFile()));
}

const KConfigManagerConfigEntry* KConfigManagerConfig::GetEntry(KPGCFileID id) const
{
	if (id != KPGC_ID_INVALID)
	{
		auto it = std::find_if(m_Entries.cbegin(), m_Entries.cend(), [id](const KConfigManagerConfigEntry& v)
		{
			return id == v.GetID();
		});
		if (it != m_Entries.cend())
		{
			return &(*it);
		}
	}
	return NULL;
}
