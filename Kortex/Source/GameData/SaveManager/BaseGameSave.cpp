#include "stdafx.h"
#include "BaseGameSave.h"

namespace Kortex::SaveManager
{
	bool BaseGameSave::IsOK() const
	{
		return m_IsOK;
	}
	bool BaseGameSave::Create(const wxString& filePath)
	{
		m_Item = KxFileItem(filePath);
		m_Item.UpdateInfo();
		return OnCreate(m_Item);
	}
	bool BaseGameSave::ReadFile()
	{
		if (!m_FileRead)
		{
			m_IsOK = OnRead(m_Item);
			m_FileRead = true;
		}
		return m_IsOK;
	}
}
