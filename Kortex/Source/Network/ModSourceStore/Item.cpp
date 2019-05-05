#include "stdafx.h"
#include "Item.h"
#include <Kortex/NetworkManager.hpp>

namespace Kortex
{
	bool ModSourceItem::IsOK() const
	{
		// Items with known mod network are valid if both mod network and mod ID are valid.
		// Unknown items are valid if at least name is present.
		if (HasModNetwork())
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
		return !HasModInfo() && !HasURL();
	}

	void ModSourceItem::Load(const KxXMLNode& node)
	{
		// Load name. This function will automatically set IModNetwork instance if it exist
		SetName(node.GetAttribute(wxS("Name")));

		// Load source data
		if (HasModNetwork())
		{
			NetworkModInfo modInfo(node.GetAttributeInt(wxS("ModID"), ModID().GetValue()), node.GetAttributeInt(wxS("FileID"), ModFileID().GetValue()));
			if (!modInfo.IsEmpty())
			{
				m_Data = std::move(modInfo);
			}
		}
		else
		{
			m_Data = node.GetAttribute(wxS("URL"));
		}
	}
	void ModSourceItem::Save(KxXMLNode& node) const
	{
		// Save name
		node.SetAttribute(wxS("Name"), GetName());

		// Save actual source info. As mod and file IDs for known sources (represented by IModNetwork instance)
		// and as web-address for everything else
		if (NetworkModInfo modInfo; TryGetModInfo(modInfo))
		{
			node.SetAttribute(wxS("ModID"), modInfo.GetModID().GetValue());
			if (modInfo.HasFileID())
			{
				node.SetAttribute(wxS("FileID"), modInfo.GetFileID().GetValue());
			}
		}
		else if (wxString url; TryGetURL(url))
		{
			node.SetAttribute(wxS("URL"), url);
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
		return {};
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
		return {};
	}
	NetworkModInfo ModSourceItem::GetModInfo() const
	{
		if (const NetworkModInfo* modInfo = std::get_if<NetworkModInfo>(&m_Data))
		{
			return *modInfo;
		}
		return {};
	}
}
