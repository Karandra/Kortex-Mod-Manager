#include "stdafx.h"
#include "KPMPluginReaderBethesdaSkyrim.h"
#include "KPMPluginEntry.h"
#include <KxFramework/KxFileStream.h>

void KPMPluginReaderFileBethesdaSkyrim::DoReadData()
{
	m_Format = KPMPE_TYPE_INVALID;

	KxFileStream file(GetFullPath(), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	if (file.IsOk())
	{
		if (file.ReadStringASCII(4) == "TES4")
		{
			// Read flags and seek to CNAM
			file.Seek(4);
			uint32_t flags = file.ReadObject<uint32_t>();

			if (flags & KPMPF_FLAG_MASTER)
			{
				m_Format = KPMPE_TYPE_MASTER;
			}
			else
			{
				m_Format = KPMPE_TYPE_NORMAL;
			}
			if (flags & KPMPF_FLAG_LIGHT)
			{
				m_Format |= KPMPE_TYPE_LIGHT;
			}

			// Skip (FormID + Version + Data) + (HEDR struct)
			file.Seek(12 + 18);

			wxString recordName = file.ReadStringASCII(4);
			if (recordName == "CNAM")
			{
				m_Author = file.ReadStringCurrentLocale(file.ReadObject<uint16_t>());
				recordName = file.ReadStringASCII(4);
			}
			if (recordName == "SNAM")
			{
				m_Description = file.ReadStringCurrentLocale(file.ReadObject<uint16_t>());
				recordName = file.ReadStringASCII(4);
			}
			if (recordName == "MAST")
			{
				do
				{
					m_Dependencies.emplace_back(file.ReadStringCurrentLocale(file.ReadObject<uint16_t>()));
					
					// Skip DATA 
					file.Seek(14);
					recordName = file.ReadStringASCII(4);
				}
				while (recordName == "MAST");
			}
		}
	}
}
