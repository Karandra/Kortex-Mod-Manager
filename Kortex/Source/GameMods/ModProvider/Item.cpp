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
			return HasModID();
		}
		else
		{
			return HasName();
		}
	}
	bool ModProviderItem::IsEmptyValue() const
	{
		return !HasModID() || !HasURL();
	}

	void ModProviderItem::Load(const KxXMLNode& node)
	{
		SetName(node.GetAttribute("Name"));

		ModID id = node.GetAttributeInt("ModID", ModID::GetInvalidValue());
		if (id.HasValue())
		{
			m_Data = std::move(id);
		}
		else
		{
			m_Data = node.GetAttributeInt("URL");
		}
	}
	void ModProviderItem::Save(KxXMLNode& node) const
	{
		node.SetAttribute("Name", GetName());

		ModID id;
		wxString url;
		if (TryGetModID(id))
		{
			node.SetAttribute("ModID", id.GetValue());
		}
		else if (TryGetURL(url))
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
		ModID modID;

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
	ModID ModProviderItem::GetModID() const
	{
		if (const ModID* id = std::get_if<ModID>(&m_Data))
		{
			return *id;
		}
		return ModID();
	}
}
