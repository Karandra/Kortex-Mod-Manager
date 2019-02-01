#include "stdafx.h"
#include "INISource.h"
#include "Item.h"
#include "ItemValue.h"
#include <KxFramework/KxFileStream.h>

namespace Kortex::GameConfig
{
	bool INISource::Open()
	{
		KxFileStream stream(KVarExp(m_FilePath), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
		return m_INI.Load(stream);
	}
	bool INISource::Save()
	{
		KxFileStream stream(KVarExp(m_FilePath), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
		if (stream.IsOk())
		{
			return m_INI.Save(stream);
		}
		return false;
	}
	void INISource::Close()
	{
		m_INI.Load(wxEmptyString);
	}

	bool INISource::WriteValue(const Item& item, const ItemValue& value)
	{
		return m_INI.SetValue(item.GetPath(), item.GetName(), value.Serialize(item));
	}
	bool INISource::ReadValue(Item& item, ItemValue& value) const
	{
		return value.Deserialize(m_INI.GetValue(item.GetPath(), item.GetName()), item);
	}
}
