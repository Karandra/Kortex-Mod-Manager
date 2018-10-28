#include "stdafx.h"
#include "KSaveFileBethesdaMorrowind.h"
#include "KSaveFileBethesdaFallout4.h"
#include "KSaveFile.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFileStream.h>

bool KSaveFileBethesdaMorrowind::DoInitializeSaveData()
{
	KxFileStream stream(GetFilePath(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
	if (stream.IsOk())
	{
		if (stream.ReadStringASCII(4) == wxS("TES3"))
		{
			// Seek to beginning of HEDR data
			stream.Skip(16);

			uint32_t headerSize = stream.ReadObject<uint32_t>();
			
			// Seek to save file name (fixed length)
			stream.SeekFromStart(64);
			m_BasicInfo.emplace_back(stream.ReadStringACP(256), KTr("SaveManager.Info.SaveName"));

			// Seek unknown bytes
			stream.Skip<uint32_t>();

			// Read plugins list
			wxString nextRecordName = stream.ReadStringASCII(4);
			while (nextRecordName == wxS("MAST"))
			{
				m_PluginsList.push_back(stream.ReadStringACP(stream.ReadObject<uint32_t>()));

				// Skip entire 'DATA' record
				stream.Skip(16);
				nextRecordName = stream.ReadStringASCII(4);
			}

			// Skip unknown 28 bytes of GMDT record
			stream.Skip(28);

			// 64 bytes of location name
			m_BasicInfo.emplace_back(stream.ReadStringACP(64), KTr("SaveManager.Info.Location"));

			// Unknown float
			stream.Skip<float>();

			// 32 bytes of character name
			m_BasicInfo.emplace_back(stream.ReadStringACP(32), KTr("SaveManager.Info.Name"));

			// Skip entire SCRD record and SCRS record name
			stream.Skip(28 + 4);

			// Read image
			const int width = 128;
			const int height = 128;
			m_Bitmap = ReadBitmapRGBA(stream.ReadVector<uint8_t>(width * height * 4), width, height, wxALPHA_OPAQUE);
			return true;
		}
	}
	return false;
}
