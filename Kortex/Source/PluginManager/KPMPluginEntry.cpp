#include "stdafx.h"
#include "KPMPluginEntry.h"
#include "KPMPluginReader.h"
#include "KPluginManager.h"
#include "Profile/KProfile.h"
#include "Profile/KPluginManagerConfig.h"
#include <KxFramework/KxString.h>

KPMPluginEntryType operator|(const KPMPluginEntryType& arg1, const KPMPluginEntryType& arg2)
{
	return static_cast<KPMPluginEntryType>((uint64_t)arg1 | (uint64_t)arg2);
}
KPMPluginEntryType& operator|=(KPMPluginEntryType& arg1, const KPMPluginEntryType& arg2)
{
	arg1 = arg1|arg2;
	return arg1;
}

//////////////////////////////////////////////////////////////////////////
KPMPluginEntry::KPMPluginEntry(const wxString& name, bool isActive, KPMPluginEntryType type)
	:m_Name(name), m_IsEnabled(isActive), m_Type(type)
{
}
KPMPluginEntry::~KPMPluginEntry()
{
	delete m_PluginReader;
}

bool KPMPluginEntry::CanToggleEnabled() const
{
	if (const KModEntry* modEntry = GetParentMod())
	{
		return modEntry->IsEnabledUnchecked();
	}
	return true;
}
bool KPMPluginEntry::IsEnabled() const
{
	if (const KModEntry* modEntry = GetParentMod())
	{
		return m_IsEnabled && modEntry->IsEnabledUnchecked();
	}
	return m_IsEnabled;
}
void KPMPluginEntry::SetEnabled(bool isActive)
{
	m_IsEnabled = isActive;
}

KPMPluginEntryType KPMPluginEntry::GetFormat() const
{
	if (m_PluginReader)
	{
		return m_PluginReader->GetFormat();
	}
	return m_Type;
}
void KPMPluginEntry::SetFormat(KPMPluginEntryType type)
{
	m_Type = type;
}

KPMPluginReader* KPMPluginEntry::GetPluginReader() const
{
	if (!m_PluginReader)
	{
		m_PluginReader = KPluginManagerConfig::GetInstance()->GetPluginReader();
	}
	return m_PluginReader;
}
const KPluginManagerConfigStandardContentEntry* KPMPluginEntry::GetStdContentEntry() const
{
	if (!m_StdContentEntryChecked)
	{
		m_StdContentEntryChecked = true;
		m_StdContentEntry = KPluginManagerConfig::GetInstance()->GetStandardContent(m_Name);
	}
	return m_StdContentEntry;
}
