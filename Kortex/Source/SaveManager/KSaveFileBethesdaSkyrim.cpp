#include "stdafx.h"
#include "KSaveFileBethesdaSkyrim.h"
#include "KSaveFile.h"
#include "KApp.h"
#include <KxFramework/KxFileStream.h>

bool KSaveFileBethesdaSkyrim::DoReadData()
{
	KxFileStream file(GetFilePath(), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	if (file.IsOk())
	{
		// http://en.uesp.net/wiki/Tes5Mod:Save_File_Format
		if (file.ReadStringASCII(13) == "TESV_SAVEGAME")
		{
			// Skip 'headerSize' and 'version' fields
			file.Seek(4 + 4);

			m_BasicInfo.push_back(KLabeledValue(std::to_string(file.ReadObject<uint32_t>()), T("SaveManager.Info.SaveIndex")));
			m_BasicInfo.push_back(KLabeledValue(file.ReadStringCurrentLocale(file.ReadObject<uint16_t>()), T("SaveManager.Info.Name")));
			m_BasicInfo.push_back(KLabeledValue(std::to_string(file.ReadObject<uint32_t>()), T("SaveManager.Info.Level")));
			m_BasicInfo.push_back(KLabeledValue(file.ReadStringCurrentLocale(file.ReadObject<uint16_t>()), T("SaveManager.Info.Location")));
			m_BasicInfo.push_back(KLabeledValue(file.ReadStringCurrentLocale(file.ReadObject<uint16_t>()), T("SaveManager.Info.TimeInGame")));
			m_BasicInfo.push_back(KLabeledValue(file.ReadStringCurrentLocale(file.ReadObject<uint16_t>()), T("SaveManager.Info.Race")));

			// Player sex
			auto sex = file.ReadObject<uint16_t>();
			m_BasicInfo.push_back(KLabeledValue(sex == 0 ? T("SaveManager.Info.SexMale") : T("SaveManager.Info.SexFemale"), T("SaveManager.Info.Sex")));

			// Skip 'playerCurExp', 'playerLvlUpExp', 'filetime' fields.
			file.Seek(4 + 4 + 8);

			// Read image
			int width = file.ReadObject<uint32_t>();
			int height = file.ReadObject<uint32_t>();
			m_Bitmap = wxBitmap(ReadImageRGB(file.ReadData<KxUInt8Vector>(width * height * 3), width, height, -1, true), 32);

			// Skip 'formVersion' and 'pluginInfoSize' fields.
			file.Seek(1 + 4);

			// Read plugins list
			size_t count = file.ReadObject<uint8_t>();
			for (size_t i = 0; i < count; i++)
			{
				m_PluginsList.push_back(file.ReadStringCurrentLocale(file.ReadObject<uint16_t>()));
			}
			return true;
		}
	}
	return false;
}
