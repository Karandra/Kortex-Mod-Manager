#include "stdafx.h"
#include "KPluginReaderBethesdaMorrowind.h"
#include "KPluginEntry.h"
#include <KxFramework/KxFileStream.h>

void KPluginReaderBethesdaMorrowind::DoReadData(const KPluginEntry& pluginEntry)
{
	KxFileStream stream(pluginEntry.GetFullPath(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
	if (stream.IsOk())
	{
		if (stream.ReadStringASCII(4) == "TES3")
		{
			// Seek after to HEDR and skip its record name and following three 32-bit fields
			stream.Seek(16 + 12);

			// These are fixed length
			m_Author = stream.ReadStringACP(32);
			m_Description = stream.ReadStringACP(256);

			// Skip unknown 32-bit value
			stream.Seek(4);

			// Read master-files if any
			wxString recordName = stream.ReadStringASCII(4);
			while (recordName == "MAST")
			{
				m_RequiredPlugins.emplace_back(stream.ReadStringACP(stream.ReadObject<uint32_t>()));

				// Skip DATA 
				stream.Seek(16);
				recordName = stream.ReadStringASCII(4);
			}

			// Morrownd doesn't store anything inside file to help distinguish master from ordinary plugin,
			// so file extension is the only option.
			m_HeaderFlags = pluginEntry.GetName().AfterLast('.').IsSameAs("esm", false) ? HeaderFlags::Master : HeaderFlags::None;

			m_IsOK = true;
		}
	}
}
