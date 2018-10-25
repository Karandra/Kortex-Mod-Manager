#include "stdafx.h"
#include "KSaveFileBethesdaSkyrimSE.h"
#include "KSaveFileBethesdaFallout4.h"
#include "KSaveFile.h"
#include "KApp.h"
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxLZ4Stream.h>
#include <KxFramework/KxMemoryStream.h>

namespace
{
	template<class CounterType, class StreamType> void ReadPluginList(StreamType& stream, KxStringVector& plugins)
	{
		size_t pluginCount = stream.ReadObject<CounterType>();
		for (size_t i = 0; i < pluginCount; i++)
		{
			uint16_t length = stream.ReadObject<uint16_t>();
			plugins.emplace_back(stream.ReadStringUTF8(length));
		}
	}
}

bool KSaveFileBethesdaSkyrimSE::DoInitializeSaveData()
{
	KxFileStream stream(GetFilePath(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
	if (stream.IsOk())
	{
		if (stream.ReadStringASCII(13) == wxS("TESV_SAVEGAME"))
		{
			// Skip 'headerSize'
			stream.Skip<uint32_t>();

			// Read version
			m_SaveVersion = stream.ReadObject<uint32_t>();

			// Read basic info
			m_BasicInfo.emplace_back(std::to_string(stream.ReadObject<uint32_t>()), T("SaveManager.Info.SaveIndex"));
			m_BasicInfo.emplace_back(stream.ReadStringUTF8(stream.ReadObject<uint16_t>()), T("SaveManager.Info.Name"));
			m_BasicInfo.emplace_back(std::to_string(stream.ReadObject<uint32_t>()), T("SaveManager.Info.Level"));
			m_BasicInfo.emplace_back(stream.ReadStringUTF8(stream.ReadObject<uint16_t>()), T("SaveManager.Info.Location"));
			m_BasicInfo.emplace_back(stream.ReadStringUTF8(stream.ReadObject<uint16_t>()), T("SaveManager.Info.TimeInGame"));
			m_BasicInfo.emplace_back(stream.ReadStringUTF8(stream.ReadObject<uint16_t>()), T("SaveManager.Info.Race"));

			// Player sex
			uint16_t playerSex = stream.ReadObject<uint16_t>();
			m_BasicInfo.emplace_back(playerSex == 0 ? T("SaveManager.Info.SexMale") : T("SaveManager.Info.SexFemale"), T("SaveManager.Info.Sex"));

			// Skip 'playerCurExp', 'playerLvlUpExp' and 'filetime' fields
			stream.Skip<float32_t, float32_t, FILETIME>();

			// Read image
			uint32_t width = stream.ReadObject<uint32_t>();
			uint32_t height = stream.ReadObject<uint32_t>();

			// Skip unknown 2 bytes
			stream.Skip<uint16_t>();
			m_Bitmap = ReadBitmapRGBA(stream.ReadVector<uint8_t>(width * height * 4), width, height);

			if (m_SaveVersion >= 12)
			{
				uint32_t uncompressedSize = stream.ReadObject<uint32_t>();
				uint32_t compressedSize = stream.ReadObject<uint32_t>();

				// Read and decompress.
				// Not very efficient, but LZ4 docs is too much awful to write decompressing stream.
				KxUInt8Vector compressedData = stream.ReadVector<uint8_t>(compressedSize);
				KxUInt8Vector uncompressedData(uncompressedSize, 0);
				KxLZ4::Decompress(compressedData.data(), compressedData.size(), uncompressedData.data(), uncompressedData.size());

				KxIOStreamWrapper<KxMemoryInputStream> memoryStream(uncompressedData.data(), uncompressedData.size());
				
				// Skip unknown 5 bytes
				memoryStream.Skip(5);

				// ESM + ESP
				ReadPluginList<uint8_t>(memoryStream, m_PluginsList);

				// ESL
				ReadPluginList<uint16_t>(memoryStream, m_PluginsList);
			}
			else
			{
				// Skip 'formVersion' field, unknown 10 bytes and 'pluginInfoSize' field
				stream.Skip(1 + 10 + 4);

				// Read plugins list
				ReadPluginList<uint8_t>(stream, m_PluginsList);
			}
			return true;
		}
	}
	return false;
}
