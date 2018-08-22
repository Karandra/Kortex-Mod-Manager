#include "stdafx.h"
#include "KPluginReaderBethesdaOblivion.h"
#include "KPluginEntry.h"
#include <KxFramework/KxFileStream.h>

void KPluginReaderBethesdaOblivion::DoReadData(const KPluginEntry& pluginEntry)
{
	KxFileStream stream(pluginEntry.GetFullPath(), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	if (stream.IsOk())
	{
		if (stream.ReadStringASCII(4) == "TES4")
		{
			// Read flags and seek to CNAM
			stream.Seek(4);
			m_HeaderFlags = stream.ReadObject<uint32_t>();
			stream.Seek(8 + 18);

			wxString recordName = stream.ReadStringASCII(4);
			if (recordName == "CNAM")
			{
				m_Author = stream.ReadStringCurrentLocale(stream.ReadObject<uint16_t>());
				recordName = stream.ReadStringASCII(4);
			}
			if (recordName == "SNAM")
			{
				m_Description = stream.ReadStringCurrentLocale(stream.ReadObject<uint16_t>());
				recordName = stream.ReadStringASCII(4);
			}
			if (recordName == "MAST")
			{
				do
				{
					m_RequiredPlugins.emplace_back(stream.ReadStringCurrentLocale(stream.ReadObject<uint16_t>()));
					
					// Skip DATA 
					stream.Seek(14);
					recordName = stream.ReadStringASCII(4);
				}
				while (recordName == "MAST");
			}
		}

		m_IsOK = true;
	}
}
