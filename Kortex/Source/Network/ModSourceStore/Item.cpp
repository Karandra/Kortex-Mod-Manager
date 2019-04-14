#include "stdafx.h"
#include "Item.h"
#include <Kortex/NetworkManager.hpp>

namespace Kortex
{
	bool ModSourceItem::IsOK() const
	{
		// Items with known modNetwork valid if both modNetwork and mod ID is valid.
		// Unknown items valid if at least name is present.
		if (HasModModNetwork())
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

	IModNetwork* ModSourceItem::GetModNetwork() const
	{
		if (auto modNetwork = std::get_if<IModNetwork*>(&m_ID))
		{
			return *modNetwork;
		}
		else if (const wxString* name = std::get_if<wxString>(&m_ID))
		{
			return INetworkManager::GetInstance()->GetModNetworkByName(*name);
		}
		return nullptr;
	}
	
	wxString ModSourceItem::GetName() const
	{
		if (auto modNetwork = std::get_if<IModNetwork*>(&m_ID); modNetwork && *modNetwork)
		{
			return (*modNetwork)->GetName();
		}
		else if (const wxString* name = std::get_if<wxString>(&m_ID))
		{
			return *name;
		}
		return wxEmptyString;
	}
	void ModSourceItem::SetName(const wxString& name)
	{
		if (IModNetwork* modNetwork = INetworkManager::GetInstance()->GetModNetworkByName(name))
		{
			m_ID = modNetwork;
		}
		else
		{
			m_ID = name;
		}
	}

	wxString ModSourceItem::GetURL(const GameID& gameID) const
	{
		IModNetwork* modNetwork = nullptr;
		if (const wxString* url = std::get_if<wxString>(&m_Data))
		{
			return *url;
		}
		else if (NetworkModInfo modInfo; TryGetModInfo(modInfo) && TryGetModNetwork(modNetwork))
		{
			return modNetwork->GetModPageURL(modInfo);
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
