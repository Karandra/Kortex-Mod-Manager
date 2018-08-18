#include "stdafx.h"
#include "KPluginManagerBethesdaGeneric2.h"
#include "Profile/KPluginManagerConfig.h"
#include <KxFramework/KxTextFile.h>

KPMPluginEntryType KPluginManagerBethesdaGeneric2::GetPluginTypeFromPath(const wxString& name) const
{
	if (name.AfterLast('.').IsSameAs("esl", false))
	{
		return KPMPE_TYPE_LIGHT;
	}
	return KPluginManagerBethesdaGeneric::GetPluginTypeFromPath(name);
}

void KPluginManagerBethesdaGeneric2::LoadNativeActiveBG()
{
	// Load names from 'Plugins.txt' it they are not already added.
	// Activate all new added and existing items with same name.
	KxStringVector activeOrder = KxTextFile::ReadToArray(m_ActiveListFile);
	for (wxString& name: activeOrder)
	{
		if (!name.IsEmpty() && !name.StartsWith('#'))
		{
			// Remove asterix
			if (name.StartsWith('*'))
			{
				name = name.AfterFirst('*');
			}

			if (KPMPluginEntry* entry = FindPluginByName(name))
			{
				entry->SetEnabled(true);
			}
		}
	}
}

wxString KPluginManagerBethesdaGeneric2::OnWriteToLoadOrder(const KPMPluginEntry* entry) const
{
	return entry->GetName();
}
wxString KPluginManagerBethesdaGeneric2::OnWriteToActiveOrder(const KPMPluginEntry* entry) const
{
	if (!entry->GetStdContentEntry())
	{
		return '*' + entry->GetName();
	}
	return entry->GetName();
}

KPluginManagerBethesdaGeneric2::KPluginManagerBethesdaGeneric2(const wxString& interfaceName, const KxXMLNode& configNode, const KPluginManagerConfig* profilePluginConfig)
	:KPluginManagerBethesdaGeneric(interfaceName, configNode, profilePluginConfig)
{
}
KPluginManagerBethesdaGeneric2::~KPluginManagerBethesdaGeneric2()
{
}
