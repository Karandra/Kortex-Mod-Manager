#include "stdafx.h"
#include "Item.h"
#include <Kortex/NetworkManager.hpp>

namespace Kortex
{
	bool ModProviderItem::IsOK() const
	{
		// Items with known provider valid if both provider and mod ID is valid.
		// Unknown items valid if at least name is present.
		if (HasProvider())
		{
			return HasModInfo();
		}
		else
		{
			return HasName();
		}
	}
	bool ModProviderItem::IsEmptyValue() const
	{
		return !HasModInfo() || !HasURL();
	}

	void ModProviderItem::Load(const KxXMLNode& node)
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
	void ModProviderItem::Save(KxXMLNode& node) const
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

	INetworkProvider* ModProviderItem::GetProvider() const
	{
		if (auto provider = std::get_if<INetworkProvider*>(&m_ID))
		{
			return *provider;
		}
		else if (const wxString* name = std::get_if<wxString>(&m_ID))
		{
			return INetworkManager::GetInstance()->FindProvider(*name);
		}
		return nullptr;
	}
	
	wxString ModProviderItem::GetName() const
	{
		if (auto provider = std::get_if<INetworkProvider*>(&m_ID); provider && *provider)
		{
			return (*provider)->GetName();
		}
		else if (const wxString* name = std::get_if<wxString>(&m_ID))
		{
			return *name;
		}
		return wxEmptyString;
	}
	void ModProviderItem::SetName(const wxString& name)
	{
		if (INetworkProvider* provider = INetworkManager::GetInstance()->FindProvider(name))
		{
			m_ID = provider;
		}
		else
		{
			m_ID = name;
		}
	}

	wxString ModProviderItem::GetURL(const GameID& gameID) const
	{
		INetworkProvider* provider = nullptr;
		if (const wxString* url = std::get_if<wxString>(&m_Data))
		{
			return *url;
		}
		else if (NetworkModInfo modInfo; TryGetModInfo(modInfo) && TryGetProvider(provider))
		{
			return provider->GetModURL(modInfo);
		}
		return wxEmptyString;
	}
	NetworkModInfo ModProviderItem::GetModInfo() const
	{
		if (const NetworkModInfo* modInfo = std::get_if<NetworkModInfo>(&m_Data))
		{
			return *modInfo;
		}
		return ModID();
	}
}
