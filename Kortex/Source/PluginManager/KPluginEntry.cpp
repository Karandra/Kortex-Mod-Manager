#include "stdafx.h"
#include "KPluginEntry.h"
#include "KPluginReader.h"
#include "KPluginManager.h"
#include "GameInstance/Config/KPluginManagerConfig.h"

KPluginEntry::~KPluginEntry()
{
}

const KPluginManagerConfigStdContentEntry* KPluginEntry::GetStdContentEntry() const
{
	if (m_StdContent == NULL)
	{
		m_StdContent = KPluginManagerConfig::GetInstance()->GetStandardContent(GetName());
	}
	return m_StdContent;
}

bool KPluginEntry::ReadPluginData()
{
	if (!m_Reader)
	{
		m_Reader = KPluginManager::QueryPluginReader(KPluginManagerConfig::GetInstance()->GetPluginFileFormat());
		if (m_Reader)
		{
			m_Reader->DoReadData(*this);
			if (m_Reader->IsOK())
			{
				OnPluginRead(*m_Reader);
				return true;
			}
		}

		m_Reader.reset();
		return false;
	}
	return true;
}
