#include "stdafx.h"
#include "KPluginManagerBethesda2.h"
#include "KPluginEntryBethesda2.h"
#include "Profile/KPluginManagerConfig.h"
#include <KxFramework/KxTextFile.h>

bool KPluginManagerBethesda2::CheckExtension(const wxString& name) const
{
	wxString ext = name.AfterLast('.');
	return ext == "esp" || ext == "esm" || ext == "esl";
}
void KPluginManagerBethesda2::LoadNativeActiveBG()
{
	// Load names from 'Plugins.txt' it they are not already added.
	// Activate all new added and existing items with same name.
	KxStringVector activeOrder = KxTextFile::ReadToArray(V(m_ActiveListFile));
	for (wxString& name: activeOrder)
	{
		if (!name.IsEmpty() && !name.StartsWith('#'))
		{
			// Remove asterix
			if (name.StartsWith('*'))
			{
				name = name.AfterFirst('*');
			}

			if (KPluginEntry* entry = FindPluginByName(name))
			{
				entry->SetEnabled(true);
			}
		}
	}
}

wxString KPluginManagerBethesda2::OnWriteToLoadOrder(const KPluginEntry& entry) const
{
	return entry.GetName();
}
wxString KPluginManagerBethesda2::OnWriteToActiveOrder(const KPluginEntry& entry) const
{
	if (!entry.IsStdContent())
	{
		return '*' + entry.GetName();
	}
	return entry.GetName();
}

intptr_t KPluginManagerBethesda2::CountLightActiveBefore(const KPluginEntry& modEntry) const
{
	intptr_t count = 0;
	for (const auto& entry: GetEntries())
	{
		if (entry.get() == &modEntry)
		{
			break;
		}

		const KPluginEntryBethesda* bethesdaPlugin = NULL;
		if (entry->IsEnabled() && entry->As(bethesdaPlugin) && bethesdaPlugin->IsLight())
		{
			count++;
		}
	}
	return count;
}

KPluginManagerBethesda2::KPluginManagerBethesda2(const wxString& interfaceName, const KxXMLNode& configNode)
	:KPluginManagerBethesda(interfaceName, configNode)
{
}
KPluginManagerBethesda2::~KPluginManagerBethesda2()
{
}

KPluginEntryBethesda2* KPluginManagerBethesda2::NewPluginEntry(const wxString& name, bool isActive) const
{
	return new KPluginEntryBethesda2(name, isActive);
}

intptr_t KPluginManagerBethesda2::GetPluginDisplayPriority(const KPluginEntry& modEntry) const
{
	const KPluginEntryBethesda* bethesdaPlugin = NULL;
	if (modEntry.As(bethesdaPlugin) && bethesdaPlugin->IsLight())
	{
		return 0xFE;
	}
	return GetPluginPriority(modEntry);
}
wxString KPluginManagerBethesda2::FormatPriority(const KPluginEntry& modEntry, intptr_t value) const
{
	if (modEntry.IsEnabled())
	{
		const KPluginEntryBethesda* bethesdaPlugin = NULL;
		if (modEntry.As(bethesdaPlugin) && bethesdaPlugin->IsLight())
		{
			return wxString::Format("0x%02X:%03X", (int)value, (int)CountLightActiveBefore(modEntry));
		}
		return KPluginManager::FormatPriority(modEntry, value);
	}
	return wxEmptyString;
}
