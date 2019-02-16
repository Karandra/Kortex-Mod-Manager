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
		KxFileStream stream(DispatchFSLocation(KVarExp(m_FilePath)), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
		return m_INI.Load(stream);
	}
	bool INISource::Save()
	{
		KxFileStream stream(DispatchFSLocation(KVarExp(m_FilePath)), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
		if (stream.IsOk())
		{
			return m_INI.Save(stream);
		}
		return false;
	}
	void INISource::Close()
	{
	}

	bool INISource::WriteValue(const Item& item, const ItemValue& value)
	{
		return m_INI.SetValue(item.GetPath(), item.GetName(), value.Serialize(item));
	}
	bool INISource::ReadValue(Item& item, ItemValue& value) const
	{
		return value.Deserialize(m_INI.GetValue(item.GetPath(), item.GetName()), item);
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
