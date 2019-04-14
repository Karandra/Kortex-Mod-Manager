#include "stdafx.h"
#include "Store.h"

namespace Kortex
{
	const ModSourceItem* ModSourceStore::GetItem(const wxString& name) const
	{
		return FindItemPtr(m_Items, name);
	}
	ModSourceItem* ModSourceStore::GetItem(const wxString& name)
	{
		return FindItemPtr(m_Items, name);
	}

	const ModSourceItem* ModSourceStore::GetItem(const IModNetwork& modNetwork) const
	{
		return FindItemPtr(m_Items, &modNetwork);
	}
	ModSourceItem* ModSourceStore::GetItem(const IModNetwork& modNetwork)
	{
		return FindItemPtr(m_Items, &modNetwork);
	}

	ModSourceItem& ModSourceStore::AssignWith(const wxString& name, const wxString& url)
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
	ModSourceItem& ModSourceStore::AssignWith(const wxString& name, NetworkModInfo modInfo)
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

	ModSourceItem& ModSourceStore::AssignWith(IModNetwork& modNetwork, const wxString& url)
	{
		auto it = FindItem(m_Items, &modNetwork);
		if (it == m_Items.end())
		{
			return m_Items.emplace_back(&modNetwork, url);
		}
		else
		{
			it->SetModSource(modNetwork);
			it->SetURL(url);
			return *it;
		}
	}
	ModSourceItem& ModSourceStore::AssignWith(IModNetwork& modNetwork, NetworkModInfo modInfo)
	{
		auto it = FindItem(m_Items, &modNetwork);
		if (it == m_Items.end())
		{
			return m_Items.emplace_back(&modNetwork, modInfo);
		}
		else
		{
			it->SetModSource(modNetwork);
			it->SetModInfo(modInfo);
			return *it;
		}
	}

	bool ModSourceStore::RemoveItem(const wxString& name)
	{
		auto it = FindItem(m_Items, name);
		if (it != m_Items.end())
		{
			m_Items.erase(it);
			return true;
		}
		return false;
	}
	bool ModSourceStore::RemoveItem(const IModNetwork& modNetwork)
	{
		auto it = FindItem(m_Items, &modNetwork);
		if (it != m_Items.end())
		{
			m_Items.erase(it);
			return true;
		}
		return false;
	}

	wxString ModSourceStore::GetModPageURL(const wxString& name, const GameID& gameID) const
	{
		if (const ModSourceItem* item = GetItem(name))
		{
			return item->GetURL(gameID);
		}
		return {};
	}
	wxString ModSourceStore::GetModPageURL(const IModNetwork& modNetwork, const GameID& gameID) const
	{
		if (const ModSourceItem* item = GetItem(modNetwork))
		{
			return item->GetURL(gameID);
		}
		return {};
	}
	
	KxStringVector ModSourceStore::GetModURLs(const GameID& gameID) const
	{
		KxStringVector items;
		items.reserve(m_Items.size());

		for (const ModSourceItem& item: m_Items)
		{
			items.push_back(item.GetURL(gameID));
		}
		return items;
	}
	KLabeledValue::Vector ModSourceStore::GetLabeledModURLs(const GameID& gameID) const
	{
		KLabeledValue::Vector items;
		items.reserve(m_Items.size());

		for (const ModSourceItem& item: m_Items)
		{
			items.emplace_back(item.GetURL(gameID), item.GetName());
		}
		return items;
	}

	void ModSourceStore::LoadTryAdd(const KxXMLNode& arrayNode)
	{
		LoadHelper<LoadMode::TryAdd>(*this, arrayNode);
	}
	void ModSourceStore::LoadAssign(const KxXMLNode& arrayNode)
	{
		LoadHelper<LoadMode::Assign>(*this, arrayNode);
	}
	void ModSourceStore::Save(KxXMLNode& arrayNode) const
	{
		for (const ModSourceItem& item: m_Items)
		{
			if (item.IsOK() && !item.IsEmptyValue())
			{
				KxXMLNode node = arrayNode.NewElement(wxS("Entry"));
				item.Save(node);
			}
		}
	}
}
