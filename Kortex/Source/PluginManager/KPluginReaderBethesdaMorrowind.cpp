#include "stdafx.h"
#include "KPMPluginReaderBethesdaMorrowind.h"
#include "KPMPluginEntry.h"
#include <KxFramework/KxFileStream.h>

void KPMPluginReaderBethesdaMorrowind::DoReadData()
{
	m_Format = KPMPE_TYPE_INVALID;

	KxFileStream file(GetFullPath(), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	if (file.IsOk())
	{
		if (file.ReadStringASCII(4) == "TES3")
		{
			// Seek after to HEDR and skip its record name and following three 32-bit fields
			file.Seek(16 + 12);

			// These are fixed length
			m_Author = file.ReadStringCurrentLocale(32);
			m_Description = file.ReadStringCurrentLocale(256);

			// Skip unknown 32-bit value
			file.Seek(4);

			// Read master-files if any
			wxString recordName = file.ReadStringASCII(4);
			while (recordName == "MAST")
			{
				m_Dependencies.emplace_back(file.ReadStringCurrentLocale(file.ReadObject<uint32_t>()));

				// Skip DATA 
				file.Seek(16);
				recordName = file.ReadStringASCII(4);
			}

			// Morrownd doesn't store anything inside file to help distinguish master from ordinary plugin,
			// so file extension is the only option.
			m_Format = GetFullPath().AfterLast('.').IsSameAs("esm", false) ? KPMPE_TYPE_MASTER : KPMPE_TYPE_NORMAL;
		}
	}
}
