#include "stdafx.h"
#include "INIRefSource.h"
#include "GameConfig/ConfigManger/Item.h"
#include "GameConfig/ConfigManger/ItemValue.h"
#include "GameConfig/ConfigManger/ItemGroup.h"
#include "GameConfig/ConfigManger/Definition.h"
#include "GameConfig/ConfigManger/Items/SimpleItem.h"

namespace Kortex::GameConfig
{
	bool INIRefSource::Open()
	{
		if (!m_IsOpened)
		{
			m_IsOpened = true;
			return true;
		}
		return false;
	}
	bool INIRefSource::Save()
	{
		return m_IsOpened;
	}
	void INIRefSource::Close()
	{
		m_IsOpened = false;
	}

	bool INIRefSource::WriteValue(const Item& item, const ItemValue& value)
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
	bool INIRefSource::ReadValue(Item& item, ItemValue& value) const
	{
		wxString valueData = m_INI.GetValue(item.GetPath(), item.GetName());
		if (!valueData.IsEmpty())
		{
			return value.Deserialize(valueData, item);
		}
		return false;
	}
	void INIRefSource::LoadUnknownItems(ItemGroup& group)
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
					wxString pathDescription = GetPathDescription();
					if (!pathDescription.IsEmpty())
					{
						item->SetCategory(wxS('/') + pathDescription + wxS('/') + sectionName);
					}
					else
					{
						item->SetCategory(wxS('/') + sectionName);
					}

					// Move the item to group
					if (item->Create())
					{
						group.AddItem(std::move(item));
					}
				}
			}
		}
	}
}
