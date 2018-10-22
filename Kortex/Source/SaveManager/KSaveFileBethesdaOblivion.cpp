#include "stdafx.h"
#include "KSaveFileBethesdaOblivion.h"
#include "KSaveFile.h"
#include "KApp.h"
#include <KxFramework/KxFileStream.h>

bool KSaveFileBethesdaOblivion::DoInitializeSaveData()
{
	KxFileStream file(GetFilePath(), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	if (file.IsOk())
	{
		if (file.ReadStringASCII(12) == "TES4SAVEGAME")
		{
			file.Seek(2 + 16 + 4 + 4);

			m_BasicInfo.push_back(KLabeledValue(std::to_string(file.ReadObject<uint32_t>()), T("SaveManager.Info.SaveIndex")));
			m_BasicInfo.push_back(KLabeledValue(file.ReadStringCurrentLocale(file.ReadObject<uint8_t>()), T("SaveManager.Info.Name")));
			m_BasicInfo.push_back(KLabeledValue(std::to_string(file.ReadObject<uint16_t>()), T("SaveManager.Info.Level")));
			m_BasicInfo.push_back(KLabeledValue(file.ReadStringCurrentLocale(file.ReadObject<uint8_t>()), T("SaveManager.Info.Location")));
			m_BasicInfo.push_back(KLabeledValue(wxString::FromCDouble(file.ReadObject<float>(), 2), T("SaveManager.Info.TimeInGame")));

			file.Seek(4 + 16 + 4);

			// Read image
			int width = file.ReadObject<uint32_t>();
			int height = file.ReadObject<uint32_t>();
			auto data = file.ReadData<std::vector<unsigned char>>(width * height * 3);
			m_Bitmap = wxBitmap(wxImage(width, height, data.data(), true), 32);

			// Read plugins list
			size_t count = file.ReadObject<uint8_t>();
			for (size_t i = 0; i < count; i++)
			{
				m_PluginsList.push_back(file.ReadStringCurrentLocale(file.ReadObject<uint8_t>()));
			}
			return true;
		}
	}
	return false;
}
