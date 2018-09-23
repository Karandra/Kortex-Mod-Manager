#include "stdafx.h"
#include "KSMSaveFileBethesdaMorrowind.h"
#include "KSMSaveFile.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFileStream.h>

bool KSMSaveFileBethesdaMorrowind::DoReadData()
{
	KxFileStream file(GetFilePath(), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	if (file.IsOk())
	{
		if (file.ReadStringASCII(4) == "TES3")
		{
			// Seek to beginning of HEDR data
			file.Seek(16);

			uint32_t nHeaderSize = file.ReadObject<uint32_t>();
			
			// Seek to save file name (fixed length)
			file.Seek(64, KxFS_SEEK_BEGIN);
			m_BasicInfo.push_back(KLabeledValue(file.ReadStringCurrentLocale(256), T("SaveManager.Info.SaveName")));

			// Seek unknown  bytes
			file.Seek(4);

			// Read plugins list
			wxString nextRecordName = file.ReadStringASCII(4);
			while (nextRecordName == "MAST")
			{
				m_PluginsList.push_back(file.ReadStringCurrentLocale(file.ReadObject<uint32_t>()));

				// Skip entire 'DATA' record
				file.Seek(16);
				nextRecordName = file.ReadStringASCII(4);
			}

			// Skip unknown 28 bytes of GMDT record
			file.Seek(28);

			// 64 bytes of location name
			m_BasicInfo.push_back(KLabeledValue(file.ReadStringCurrentLocale(64), T("SaveManager.Info.Location")));

			// Unknown float
			file.Seek(4);

			// 32 bytes of character name
			m_BasicInfo.push_back(KLabeledValue(file.ReadStringCurrentLocale(32), T("SaveManager.Info.Name")));

			// Skip entire SCRD record and SCRS record name
			file.Seek(28 + 4);

			// Read image (best guess is 171x127)
			int width = 171;
			int height = 127;
			auto data = file.ReadData<std::vector<unsigned char>>(width * height * 3);
			m_Bitmap = wxBitmap(wxImage(width, height, data.data(), true), 32);

			return true;
		}
	}
	return false;
}
