#include "stdafx.h"
#include "Item.h"
#include <Kortex/NetworkManager.hpp>

namespace Kortex
{
	bool ModSourceItem::IsOK() const
	{
		// Items with known modSource valid if both modSource and mod ID is valid.
		// Unknown items valid if at least name is present.
		if (HasModSource())
		{
			return HasModInfo();
		}
		else
		{
			return HasName();
		}
	}
	bool ModSourceItem::IsEmptyValue() const
	{
		return !HasModInfo() || !HasURL();
	}

	void ModSourceItem::Load(const KxXMLNode& node)
	{
		SetName(node.GetAttribute("Name"));

		NetworkModInfo modInfo(node.GetAttributeInt("ModID", ModID().GetValue()), node.GetAttributeInt("FileID", ModFileID().GetValue()));
		if (!modInfo.IsEmpty())
		{
			m_Data = std::move(modInfo);
		}
		else
		{
			m_Data = node.GetAttribute("URL");
		}
	}
	void ModSourceItem::Save(KxXMLNode& node) const
	{
		node.SetAttribute("Name", GetName());
		if (NetworkModInfo modInfo; TryGetModInfo(modInfo))
		{
			node.SetAttribute("ModID", modInfo.GetModID().GetValue());
			if (modInfo.HasFileID())
			{
				node.SetAttribute("FileID", modInfo.GetFileID().GetValue());
			}
		}
		else if (wxString url; TryGetURL(url))
		{
			node.SetAttribute("URL", url);
		}
	}

	IModSource* ModSourceItem::GetModSource() const
	{
		if (auto modSource = std::get_if<IModSource*>(&m_ID))
		{
			return *modSource;
		}
		else if (const wxString* name = std::get_if<wxString>(&m_ID))
		{
			return INetworkManager::GetInstance()->FindModSource(*name);
		}
		return nullptr;
	}
	
	wxString ModSourceItem::GetName() const
	{
		if (auto modSource = std::get_if<IModSource*>(&m_ID); modSource && *modSource)
		{
			return (*modSource)->GetName();
		}
		else if (const wxString* name = std::get_if<wxString>(&m_ID))
		{
			return *name;
		}
		return wxEmptyString;
	}
	void ModSourceItem::SetName(const wxString& name)
	{
		if (IModSource* modSource = INetworkManager::GetInstance()->FindModSource(name))
		{
			m_ID = modSource;
		}
		else
		{
			m_ID = name;
		}
	}

	wxString ModSourceItem::GetURL(const GameID& gameID) const
	{
		IModSource* modSource = nullptr;
		if (const wxString* url = std::get_if<wxString>(&m_Data))
		{
			return *url;
		}
		else if (NetworkModInfo modInfo; TryGetModInfo(modInfo) && TryGetModSource(modSource))
		{
			return modSource->GetModURL(modInfo);
		}
		return wxEmptyString;
	}
	NetworkModInfo ModSourceItem::GetModInfo() const
	{
		if (const NetworkModInfo* modInfo = std::get_if<NetworkModInfo>(&m_Data))
		{
			return *modInfo;
		}
		return ModID();
	}
}
