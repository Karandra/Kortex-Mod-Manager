#include "stdafx.h"
#include "KSaveFileBethesdaFallout3.h"
#include "KSaveFile.h"
#include "KApp.h"
#include <KxFramework/KxFileStream.h>

bool KSaveFileBethesdaFallout3::DoInitializeSaveData()
{
	KxFileStream file(GetFilePath(), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	if (file.IsOk())
	{
		if (file.ReadStringASCII(11) == "FO3SAVEGAME")
		{
			// Skip 'headerSize' field, unknown field and separator
			file.Seek(4 + 5);

			// Read screenshot dimensions
			int width = file.ReadObject<uint32_t>();
			file.Seek(1);
			int height = file.ReadObject<uint32_t>();

			// Read save index
			file.Seek(1);
			m_BasicInfo.push_back(KLabeledValue(std::to_string(file.ReadObject<uint32_t>()), T("SaveManager.Info.SaveIndex")));

			auto ReadWZString = [this, &file](const wxString& fieldName)
			{
				file.Seek(1);
				auto length = file.ReadObject<uint16_t>();
				file.Seek(1);
				m_BasicInfo.push_back(KLabeledValue(file.ReadStringCurrentLocale(length), T(fieldName)));
			};

			// Read name
			ReadWZString("SaveManager.Info.Name");
			ReadWZString("SaveManager.Info.Karma");
			
			// Read level
			file.Seek(1);
			m_BasicInfo.push_back(KLabeledValue(std::to_string(file.ReadObject<uint32_t>()), T("SaveManager.Info.Level")));

			// Read location
			ReadWZString("SaveManager.Info.Location");

			// Read game time
			ReadWZString("SaveManager.Info.TimeInGame");

			// Skip separator
			file.Seek(1);

			// Read image
			auto data = file.ReadData<std::vector<unsigned char>>(width * height * 3);
			m_Bitmap = wxBitmap(wxImage(width, height, data.data(), true), 32);

			// Skip 'formVersion' and 'pluginInfoSize' fields
			file.Seek(1 + 4);

			// Read plugins list
			size_t count = file.ReadObject<uint8_t>();

			for (size_t i = 0; i < count; i++)
			{
				file.Seek(1);
				auto length = file.ReadObject<uint16_t>();
				file.Seek(1);
				m_PluginsList.push_back(file.ReadStringCurrentLocale(length));
			}
			return true;
		}
	}
	return false;
}
