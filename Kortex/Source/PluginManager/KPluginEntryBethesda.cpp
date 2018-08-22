#include "stdafx.h"
#include "KPluginEntryBethesda.h"
#include "KPluginReaderBethesda.h"
#include "ModManager/KModEntry.h"

void KPluginEntryBethesda::OnPluginRead(const KPluginReader& reader)
{
	const KPluginReaderBethesda& bethesdaReader = static_cast<const KPluginReaderBethesda&>(reader);
	m_IsMaster = bethesdaReader.IsMaster();
	m_IsLight = bethesdaReader.IsLight();
}

bool KPluginEntryBethesda::CanToggleEnabled() const
{
	if (const KModEntry* modEntry = GetParentMod())
	{
		return modEntry->IsEnabledUnchecked();
	}
	return true;
}
