#include "stdafx.h"
#include "Store.h"

namespace Kortex
{
	const ModProviderItem* ModProviderStore::GetItem(const wxString& name) const
	{
		return FindItemPtr(m_Items, name);
	}
	ModProviderItem* ModProviderStore::GetItem(const wxString& name)
	{
		return FindItemPtr(m_Items, name);
	}

	const ModProviderItem* ModProviderStore::GetItem(const INetworkProvider& provider) const
	{
		return FindItemPtr(m_Items, &provider);
	}
	ModProviderItem* ModProviderStore::GetItem(const INetworkProvider& provider)
	{
		return FindItemPtr(m_Items, &provider);
	}

	ModProviderItem& ModProviderStore::AssignWith(const wxString& name, const wxString& url)
	{
		auto it = FindItem(m_Items, name);
		if (it == m_Items.end())
		{
			return m_Items.emplace_back(name, url);
		}
		else
		{
			it->SetName(name);
			it->SetURL(url);
			return *it;
		}
	}
	ModProviderItem& ModProviderStore::AssignWith(const wxString& name, NetworkModInfo modInfo)
	{
		auto it = FindItem(m_Items, name);
		if (it == m_Items.end())
		{
			return m_Items.emplace_back(name, modInfo);
		}
		else
		{
			it->SetName(name);
			it->SetModInfo(modInfo);
			return *it;
		}
	}

	ModProviderItem& ModProviderStore::AssignWith(INetworkProvider& provider, const wxString& url)
	{
		auto it = FindItem(m_Items, &provider);
		if (it == m_Items.end())
		{
			return m_Items.emplace_back(&provider, url);
		}
		else
		{
			it->SetProvider(provider);
			it->SetURL(url);
			return *it;
		}
	}
	ModProviderItem& ModProviderStore::AssignWith(INetworkProvider& provider, NetworkModInfo modInfo)
	{
		auto it = FindItem(m_Items, &provider);
		if (it == m_Items.end())
		{
			return m_Items.emplace_back(&provider, modInfo);
		}
		else
		{
			it->SetProvider(provider);
			it->SetModInfo(modInfo);
			return *it;
		}
	}

	bool ModProviderStore::RemoveItem(const wxString& name)
	{
		auto it = FindItem(m_Items, name);
		if (it != m_Items.end())
		{
			m_Items.erase(it);
			return true;
		}
		return false;
	}
	bool ModProviderStore::RemoveItem(const INetworkProvider& provider)
	{
		auto it = FindItem(m_Items, &provider);
		if (it != m_Items.end())
		{
			m_Items.erase(it);
			return true;
		}
		return false;
	}

	wxString ModProviderStore::GetModURL(const wxString& name, const GameID& gameID) const
	{
		if (const ModProviderItem* item = GetItem(name))
		{
			return item->GetURL(gameID);
		}
		return wxEmptyString;
	}
	wxString ModProviderStore::GetModURL(const INetworkProvider& provider, const GameID& gameID) const
	{
		if (const ModProviderItem* item = GetItem(provider))
		{
			return item->GetURL(gameID);
		}
		return wxEmptyString;
	}
	
	KxStringVector ModProviderStore::GetModURLs(const GameID& gameID) const
	{
		KxStringVector items;
		items.reserve(m_Items.size());

		for (const ModProviderItem& item: m_Items)
		{
			items.push_back(item.GetURL(gameID));
		}
		return items;
	}
	KLabeledValue::Vector ModProviderStore::GetModNamedURLs(const GameID& gameID) const
	{
		KLabeledValue::Vector items;
		items.reserve(m_Items.size());

		for (const ModProviderItem& item: m_Items)
		{
			items.emplace_back(item.GetURL(gameID), item.GetName());
		}
		return items;
	}

	void ModProviderStore::LoadTryAdd(const KxXMLNode& arrayNode)
	{
		LoadHelper<LoadMode::TryAdd>(*this, arrayNode);
	}
	void ModProviderStore::LoadAssign(const KxXMLNode& arrayNode)
	{
		LoadHelper<LoadMode::Assign>(*this, arrayNode);
	}
	void ModProviderStore::Save(KxXMLNode& arrayNode) const
	{
		for (const ModProviderItem& item: m_Items)
		{
			if (item.IsOK() && !item.IsEmptyValue())
			{
				KxXMLNode node = arrayNode.NewElement(wxS("Entry"));
				item.Save(node);
			}
		}
	}
}
