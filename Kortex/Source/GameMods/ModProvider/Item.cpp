#include "stdafx.h"
#include "Item.h"
#include <Kortex/NetworkManager.hpp>

namespace Kortex::ModProvider
{
	void Item::Load(const KxXMLNode& node)
	{
		SetName(node.GetAttribute("Name"));

		Network::ModID id = node.GetAttributeInt("ModID", Network::InvalidModID);
		if (id != Network::InvalidModID)
		{
			m_Data = id;
		}
		else
		{
			m_Data = node.GetAttributeInt("URL");
		}
	}
	void Item::Save(KxXMLNode& node) const
	{
		node.SetAttribute("Name", GetName());

		Network::ModID id = Network::InvalidModID;
		wxString url;
		if (TryGetModID(id))
		{
			node.SetAttribute("ModID", id);
		}
		else if (TryGetURL(url))
		{
			node.SetAttribute("URL", url);
		}
	}

	INetworkProvider* Item::GetProvider() const
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
	
	wxString Item::GetName() const
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
	void Item::SetName(const wxString& name)
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

	wxString Item::GetURL(const GameID& gameID) const
	{
		Network::ModID modID = Network::InvalidModID;
		INetworkProvider* provider = nullptr;

		if (const wxString* url = std::get_if<wxString>(&m_Data))
		{
			return *url;
		}
		else if (TryGetModID(modID) && TryGetProvider(provider))
		{
			return provider->GetModURL(modID, wxEmptyString, gameID);
		}
		return wxEmptyString;
	}
	Network::ModID Item::GetModID() const
	{
		if (const Network::ModID* id = std::get_if<Network::ModID>(&m_Data))
		{
			return *id;
		}
		return Network::InvalidModID;
	}
}
