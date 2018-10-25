#include "stdafx.h"
#include "KPluginReaderBethesdaOblivion.h"
#include "KPluginEntry.h"
#include <KxFramework/KxFileStream.h>

void KPluginReaderBethesdaOblivion::DoReadData(const KPluginEntry& pluginEntry)
{
	KxFileStream stream(pluginEntry.GetFullPath(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
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
				m_Author = stream.ReadStringACP(stream.ReadObject<uint16_t>());
				recordName = stream.ReadStringASCII(4);
			}
			if (recordName == "SNAM")
			{
				m_Description = stream.ReadStringACP(stream.ReadObject<uint16_t>());
				recordName = stream.ReadStringASCII(4);
			}
			if (recordName == "MAST")
			{
				do
				{
					m_RequiredPlugins.emplace_back(stream.ReadStringACP(stream.ReadObject<uint16_t>()));
					
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
