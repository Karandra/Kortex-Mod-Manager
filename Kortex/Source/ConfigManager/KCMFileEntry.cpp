#include "stdafx.h"
#include "KCMFileEntry.h"
#include "KCMConfigEntry.h"
#include "KCMIDataProvider.h"
#include "KCMDataProviderINI.h"
#include "KConfigManager.h"
#include "Profile/KProfile.h"

KPGCFileID KCMFileEntry::LoadID(KxXMLNode& node)
{
	return (KPGCFileID)node.GetAttributeInt("ID", KPGC_ID_INVALID);
}
void KCMFileEntry::LoadEntries(KxXMLNode& node)
{
	for (KxXMLNode entryNode = node.GetFirstChildElement("Values").GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
	{
		KCMConfigEntryStd* configEntry = NULL;
		switch (KCMConfigEntryStd::GetSubType(entryNode))
		{
			case KCMDST_DOUBLE_VALUE:
			{
				configEntry = new KCMConfigEntryDV(this, m_Formatter);
				break;
			}
			case KCMDST_VIRTUAL_KEY:
			{
				configEntry = new KCMConfigEntryVK(this, m_Formatter);
				break;
			}
			case KCMDST_ARRAY:
			{
				configEntry = new KCMConfigEntryArray(this, m_Formatter);
				break;
			}
			case KCMDST_FILE_BROWSE:
			{
				configEntry = new KCMConfigEntryFileBrowse(this, m_Formatter);
				break;
			}
			default:
			{
				configEntry = new KCMConfigEntryStd(this, m_Formatter);
			}
		};

		if (configEntry)
		{
			configEntry->Create(entryNode);
			AddEntry(configEntry);
		}
	}
}
void KCMFileEntry::LoadUnknownEntries()
{
	if (m_DataProvider)
	{
		m_DataProvider->Load();
		m_DataProvider->OnLoadUnknownEntries(this);
	}
}
void KCMFileEntry::InitDataProvider()
{
	m_DataProvider = GetConfigManager()->OnQueryDataProvider(this);
}

KCMFileEntry::KCMFileEntry(KConfigManager* pConfigManager, KxXMLNode& node, const KCMOptionsFormatter& defaultOptions)
	:m_ConfigManager(pConfigManager), m_Formatter(KCMOptionsFormatter::LoadFormatOptions(node, defaultOptions))
{
	m_ProfileEntry = KConfigManager::GetGameConfig()->GetEntry(LoadID(node));
	InitDataProvider();
	LoadEntries(node);
	LoadUnknownEntries();
}
KCMFileEntry::KCMFileEntry(KConfigManager* pConfigManager, const KConfigManagerConfigEntry* profileEntry, const KCMOptionsFormatter& defaultOptions)
	:m_ConfigManager(pConfigManager), m_ProfileEntry(profileEntry), m_Formatter(defaultOptions)
{
	InitDataProvider();
	LoadUnknownEntries();
}
void KCMFileEntry::ClearEntries()
{
	for (KCMConfigEntryBase* configEntry: m_Entries)
	{
		delete configEntry;
	}
	m_Entries.clear();
}
KCMFileEntry::~KCMFileEntry()
{
	ClearEntries();
}

bool KCMFileEntry::IsOK() const
{
	return m_ConfigManager != NULL && m_ProfileEntry != NULL && m_DataProvider != NULL && m_DataProvider->IsOK();
}
KPGCFileID KCMFileEntry::GetID() const
{
	if (m_ProfileEntry)
	{
		return m_ProfileEntry->GetID();
	}
	return KPGC_ID_INVALID;
}
bool KCMFileEntry::IsPartOfENB() const
{
	KPGCFileID id = GetID();
	return id == KPGC_ID_ENB_LOCAL || id == KPGC_ID_ENB_SERIES;
}

void KCMFileEntry::Load()
{
	if (m_DataProvider)
	{
		m_DataProvider->Load();
		for (size_t i = 0; i < GetEntriesCount(); i++)
		{
			KCMConfigEntryStd* configEntry = GetEntryAt(i)->ToStdEntry();
			if (configEntry)
			{
				m_DataProvider->ProcessLoadEntry(configEntry);
			}
		}
	}
}
