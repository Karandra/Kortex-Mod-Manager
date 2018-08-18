#include "stdafx.h"
#include "KPMPluginReader.h"
#include "KPMPluginEntry.h"

void KPMPluginReader::Create(const wxString& fullPath)
{
	m_FullPath = fullPath;
	DoReadData();
	m_IsRead = true;
}

bool KPMPluginReader::IsOK() const
{
	return DoGetFormat() != KPMPE_TYPE_INVALID;
}
bool KPMPluginReader::IsMaster() const
{
	return DoGetFormat() == KPMPE_TYPE_MASTER;
}
