#include "stdafx.h"
#include "GetVirtualKeys.h"
#include "GameConfig/IConfigManager.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxXML.h>
#include <KxFramework/KxFileStream.h>

namespace Kortex::GameConfig::SamplingFunction
{
	void GetVirtualKeys::LoadVirtualKeys()
	{
		static VirtualKeyInfo::Map virtualKeys;
		m_KeyMap = &virtualKeys;

		KxFileStream xmlStream(IApplication::GetInstance()->GetDataFolder() + wxS("VirtualKeys.xml"), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting);
		KxXMLDocument xml(xmlStream);

		const ITranslator& translator = m_Manager.GetTranslator();
		KxXMLNode node = xml.QueryElement("VirtualKeys");
		for (node = node.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			unsigned long keyCode = WXK_NONE;
			wxString value = node.GetValue();
			if (value.Mid(2).ToCULong(&keyCode, 16) || value.ToCULong(&keyCode, 16))
			{
				wxString vkid = node.GetAttribute("VKID");
				wxString name = node.GetAttribute("Name");
				if (name.IsEmpty())
				{
					name = KxString::Format("%1", keyCode);
				}
				auto label = translator.TryGetString(wxS("ConfigManager.VirtualKey.") + vkid);
				if (label)
				{
					name = *label;
				}

				virtualKeys.insert_or_assign(keyCode, VirtualKeyInfo {vkid, name, keyCode});
			}
		}

		if (!virtualKeys.count(WXK_NONE))
		{
			virtualKeys.insert_or_assign(WXK_NONE, VirtualKeyInfo {"VK_NONE", translator.GetString("ConfigManager.VirtualKey.VK_NONE"), WXK_NONE});
		}
	}
	void GetVirtualKeys::OnCall(const ItemValue::Vector& arguments)
	{
		if (m_KeyMap == nullptr)
		{
			LoadVirtualKeys();
		}
		for (const auto&[keyCode, info]: *m_KeyMap)
		{
			m_Values.emplace_back(keyCode).SetLabel(info.Name);
		}
	}
}
