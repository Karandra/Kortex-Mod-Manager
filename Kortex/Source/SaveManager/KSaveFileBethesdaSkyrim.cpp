#include "stdafx.h"
#include "KSaveFileBethesdaSkyrim.h"
#include "KSaveFile.h"
#include "KApp.h"
#include <KxFramework/KxFileStream.h>

bool KSaveFileBethesdaSkyrim::DoInitializeSaveData()
{
	// http://en.uesp.net/wiki/Tes5Mod:Save_File_Format
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
			m_BasicInfo.emplace_back(std::to_string(stream.ReadObject<uint32_t>()), KTr("SaveManager.Info.SaveIndex"));
			m_BasicInfo.emplace_back(stream.ReadStringACP(stream.ReadObject<uint16_t>()), KTr("SaveManager.Info.Name"));
			m_BasicInfo.emplace_back(std::to_string(stream.ReadObject<uint32_t>()), KTr("SaveManager.Info.Level"));
			m_BasicInfo.emplace_back(stream.ReadStringACP(stream.ReadObject<uint16_t>()), KTr("SaveManager.Info.Location"));
			m_BasicInfo.emplace_back(stream.ReadStringACP(stream.ReadObject<uint16_t>()), KTr("SaveManager.Info.TimeInGame"));
			m_BasicInfo.emplace_back(stream.ReadStringACP(stream.ReadObject<uint16_t>()), KTr("SaveManager.Info.Race"));

			// Player sex
			uint16_t playerSex = stream.ReadObject<uint16_t>();
			m_BasicInfo.emplace_back(playerSex == 0 ? KTr("SaveManager.Info.SexMale") : KTr("SaveManager.Info.SexFemale"), KTr("SaveManager.Info.Sex"));

			// Skip 'playerCurExp', 'playerLvlUpExp' and 'filetime' fields.
			stream.Skip<float32_t, float32_t, FILETIME>();

			// Read image
			uint32_t width = stream.ReadObject<uint32_t>();
			uint32_t height = stream.ReadObject<uint32_t>();
			m_Bitmap = ReadBitmapRGB(stream.ReadVector<uint8_t>(width * height * 3), width, height);

			// Skip 'formVersion' and 'pluginInfoSize' fields.
			stream.Skip<uint8_t, uint32_t>();

			// Read plugins list
			size_t count = stream.ReadObject<uint8_t>();
			for (size_t i = 0; i < count; i++)
			{
				m_PluginsList.emplace_back(stream.ReadStringACP(stream.ReadObject<uint16_t>()));
			}
			return true;
		}
	}
	return false;
}
