#include "stdafx.h"
#include "BethesdaPluginReaderSkyrim.h"
#include "IGamePlugin.h"
#include <KxFramework/KxFileStream.h>

namespace Kortex::PluginManager
{
	void BethesdaPluginReaderSkyrim::OnRead(const IGamePlugin& plugin)
	{
		KxFileStream stream(plugin.GetFullPath(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
		if (stream.IsOk() && stream.ReadStringASCII(4) == "TES4")
		{
			// Read flags and seek to CNAM
			stream.Seek(4);
			m_Data.m_HeaderFlags = (HeaderFlags)stream.ReadObject<uint32_t>();

			// Skip (FormID + Version control info)
			stream.Seek(8);
			m_Data.m_FormVersion = stream.ReadObject<uint32_t>();

			// Skip HEDR struct
			stream.Seek(18);

			wxString recordName = stream.ReadStringASCII(4);
			if (recordName == "CNAM")
			{
				m_Data.m_Author = stream.ReadStringACP(stream.ReadObject<uint16_t>());
				recordName = stream.ReadStringASCII(4);
			}
			if (recordName == "SNAM")
			{
				m_Data.m_Description = stream.ReadStringACP(stream.ReadObject<uint16_t>());
				recordName = stream.ReadStringASCII(4);
			}
			if (recordName == "MAST")
			{
				do
				{
					m_Data.m_RequiredPlugins.emplace_back(stream.ReadStringACP(stream.ReadObject<uint16_t>()));

					// Skip DATA 
					stream.Seek(14);
					recordName = stream.ReadStringASCII(4);
				}
				while (recordName == "MAST");
			}

			m_IsOK = true;
		}
	}
}
