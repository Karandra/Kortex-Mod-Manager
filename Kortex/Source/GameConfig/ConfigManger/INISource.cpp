#include "stdafx.h"
#include "INISource.h"
#include "Item.h"
#include "ItemValue.h"
#include "ItemGroup.h"
#include "Definition.h"
#include "Items/SimpleItem.h"
#include <KxFramework/KxFileStream.h>

namespace Kortex::GameConfig
{
	bool INISource::Open()
	{
		if (!m_IsOpened)
		{
			KxFileStream stream(DispatchFSLocation(KVarExp(m_FilePath)), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
			if (stream.IsOk() && m_INI.Load(stream))
			{
				m_IsOpened = true;
				return true;
			}
		}
		return false;
	}
	bool INISource::Save()
	{
		if (m_IsOpened)
		{
			KxFileStream stream(DispatchFSLocation(KVarExp(m_FilePath)), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
			if (stream.IsOk())
			{
				return m_INI.Save(stream);
			}
		}
		return false;
	}
	void INISource::Close()
	{
		m_IsOpened = false;
	}

	bool INISource::WriteValue(const Item& item, const ItemValue& value)
	{
		if (value.IsNull())
		{
			return m_INI.RemoveValue(item.GetPath(), item.GetName());
		}
		else
		{
			return m_INI.SetValue(item.GetPath(), item.GetName(), value.Serialize(item));
		}
	}
	bool INISource::ReadValue(Item& item, ItemValue& value) const
	{
		wxString valueData = m_INI.GetValue(item.GetPath(), item.GetName());
		if (!valueData.IsEmpty())
		{
			return value.Deserialize(valueData, item);
		}
		return false;
	}
	void INISource::LoadUnknownItems(ItemGroup& group)
	{
		for (const wxString& sectionName: m_INI.GetSectionNames())
		{
			for (const wxString& keyName: m_INI.GetKeyNames(sectionName))
			{
				// Skip C++ style comments
				if (keyName.StartsWith(wxS("//")))
				{
					continue;
				}

				auto item = group.NewItem<SimpleItem>(group, true);
				item->SetName(keyName);
				item->SetPath(sectionName);

				if (!group.HasItem(*item))
				{
					// Run type detectors
					TypeID type = InvokeTypeDetectors(group, [this, &type, &keyName, &sectionName](const ITypeDetector& detector)
					{
						return std::tuple{keyName, detector.RequiresValueData() ? m_INI.GetValue(sectionName, keyName) : wxString()};
					});
					item->SetTypeID(type);

					// Category is never a part of item's hash calculation, so set it here
					item->SetCategory(wxS('/') + m_FileName + wxS('/') + sectionName);

					// Move the item to group
					group.AddItem(std::move(item));
				}
			}
		}
	}
}
