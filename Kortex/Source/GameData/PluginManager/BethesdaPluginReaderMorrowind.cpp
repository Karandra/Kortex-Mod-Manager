#include "stdafx.h"
#include "BethesdaPluginReaderMorrowind.h"
#include "GameData/IGamePlugin.h"
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxComparator.h>

namespace Kortex::PluginManager
{
	void BethesdaPluginReaderMorrowind::OnRead(const IGamePlugin& plugin)
	{
		KxFileStream stream(plugin.GetFullPath(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
		if (stream.IsOk())
		{
			if (stream.ReadStringASCII(4) == "TES3")
			{
				// Seek after to HEDR and skip its record name and following three 32-bit fields
				stream.Seek(16 + 12);

				// These are fixed length
				m_Data.m_Author = stream.ReadStringACP(32);
				m_Data.m_Description = stream.ReadStringACP(256);

				// Skip unknown 32-bit value
				stream.Seek(4);

				// Read master-files if any
				wxString recordName = stream.ReadStringASCII(4);
				while (recordName == "MAST")
				{
					m_Data.m_RequiredPlugins.emplace_back(stream.ReadStringACP(stream.ReadObject<uint32_t>()));

					// Skip DATA 
					stream.Seek(16);
					recordName = stream.ReadStringASCII(4);
				}

				// Morrownd doesn't store anything inside file to help distinguish master from ordinary plugin,
				// so file extension is the only option.
				if (KxComparator::IsEqual(plugin.GetName().AfterLast(wxS('.')), wxS("esm")))
				{
					m_Data.m_HeaderFlags = BethesdaPluginData::HeaderFlags::Master;
				}

				m_IsOK = true;
			}
		}
	}
}
