#include "stdafx.h"
#include "Oblivion.h"
#include <KxFramework/KxFileStream.h>

namespace Kortex::SaveManager::BethesdaSave
{
	bool Oblivion::OnRead(const KxFileItem& fileItem)
	{
		KxFileStream stream(fileItem.GetFullPath(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
		if (stream.IsOk())
		{
			if (stream.ReadStringASCII(12) == wxS("TES4SAVEGAME"))
			{
				// Skip 'majorVersion', 'minorVersion' and exeTime.
				stream.Skip<uint8_t, uint8_t, SYSTEMTIME>();

				// Read 'headerVersion' field
				m_SaveVersion = stream.ReadObject<uint32_t>();

				// Skip 'saveHeaderSize' field
				stream.Skip<uint32_t>();

				m_BasicInfo.emplace_back(std::to_string(stream.ReadObject<uint32_t>()), KTr("SaveManager.Info.SaveIndex"));
				m_BasicInfo.emplace_back(stream.ReadStringACP(stream.ReadObject<uint8_t>()), KTr("SaveManager.Info.Name"));
				m_BasicInfo.emplace_back(std::to_string(stream.ReadObject<uint16_t>()), KTr("SaveManager.Info.Level"));
				m_BasicInfo.emplace_back(stream.ReadStringACP(stream.ReadObject<uint8_t>()), KTr("SaveManager.Info.Location"));
				m_BasicInfo.emplace_back(wxString::FromCDouble(stream.ReadObject<float32_t>(), 2), KTr("SaveManager.Info.TimeInGame"));

				// Skip 'gameTicks', 'gameTime' and screenshot struct size
				stream.Skip<uint32_t, SYSTEMTIME, uint32_t>();

				// Read image
				uint32_t width = stream.ReadObject<uint32_t>();
				uint32_t height = stream.ReadObject<uint32_t>();
				m_Bitmap = ReadBitmapRGB(stream.ReadVector<uint8_t>(width * height * 3), width, height);

				// Read plugins list
				size_t count = stream.ReadObject<uint8_t>();
				for (size_t i = 0; i < count; i++)
				{
					m_PluginsList.emplace_back(stream.ReadStringACP(stream.ReadObject<uint8_t>()));
				}
				return true;
			}
		}
		return false;
	}
}
