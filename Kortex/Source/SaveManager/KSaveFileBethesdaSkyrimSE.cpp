#include "stdafx.h"
#include "KSaveFileBethesdaSkyrimSE.h"
#include "KSaveFileBethesdaFallout4.h"
#include "KSaveFile.h"
#include "KApp.h"
#include <KxFramework/KxFileStream.h>

bool KSaveFileBethesdaSkyrimSE::DoReadData()
{
	KxFileStream file(GetFilePath(), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	if (file.IsOk())
	{
		if (file.ReadStringASCII(13) == "TESV_SAVEGAME")
		{
			// Skip 'headerSize' and 'version' fields
			file.Seek(4 + 4);

			m_BasicInfo.push_back(KLabeledValue(std::to_string(file.ReadObject<uint32_t>()), T("SaveManager.Info.SaveIndex")));
			m_BasicInfo.push_back(KLabeledValue(file.ReadStringUTF8(file.ReadObject<uint16_t>()), T("SaveManager.Info.Name")));
			m_BasicInfo.push_back(KLabeledValue(std::to_string(file.ReadObject<uint32_t>()), T("SaveManager.Info.Level")));
			m_BasicInfo.push_back(KLabeledValue(file.ReadStringUTF8(file.ReadObject<uint16_t>()), T("SaveManager.Info.Location")));
			m_BasicInfo.push_back(KLabeledValue(file.ReadStringUTF8(file.ReadObject<uint16_t>()), T("SaveManager.Info.TimeInGame")));
			m_BasicInfo.push_back(KLabeledValue(file.ReadStringUTF8(file.ReadObject<uint16_t>()), T("SaveManager.Info.Race")));

			// Player sex
			auto nSex = file.ReadObject<uint16_t>();
			m_BasicInfo.push_back(KLabeledValue(nSex == 0 ? T("SaveManager.Info.SexMale") : T("SaveManager.Info.SexFemale"), T("SaveManager.Info.Sex")));

			// Skip 'playerCurExp', 'playerLvlUpExp', 'filetime' fields
			file.Seek(4 + 4 + 8);

			// Read image
			int width = file.ReadObject<uint32_t>();
			int height = file.ReadObject<uint32_t>();

			// Skip unknown 2 bytes
			file.Seek(2);
			m_Bitmap = wxBitmap(KSaveFileBethesdaFallout4::ReadImageRGBA(file.ReadData<std::vector<unsigned char>>(width * height * 4), width, height), 32);

			// Skip 'formVersion' field, unknown 10 bytes and 'pluginInfoSize' field
			file.Seek(1 + 10 + 4);

			// Read plugins list
			size_t count = file.ReadObject<uint8_t>();
			for (size_t i = 0; i < count; i++)
			{
				m_PluginsList.push_back(file.ReadStringUTF8(file.ReadObject<uint16_t>()));
			}
			return true;
		}
	}
	return false;
}
