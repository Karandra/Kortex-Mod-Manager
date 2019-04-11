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

	const ModSourceItem* ModSourceStore::GetItem(const INetworkModSource& modSource) const
	{
		return FindItemPtr(m_Items, &modSource);
	}
	ModSourceItem* ModSourceStore::GetItem(const INetworkModSource& modSource)
	{
		return FindItemPtr(m_Items, &modSource);
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

	ModSourceItem& ModSourceStore::AssignWith(INetworkModSource& modSource, const wxString& url)
	{
		auto it = FindItem(m_Items, &modSource);
		if (it == m_Items.end())
		{
			return m_Items.emplace_back(&modSource, url);
		}
		else
		{
			it->SetModSource(modSource);
			it->SetURL(url);
			return *it;
		}
	}
	ModSourceItem& ModSourceStore::AssignWith(INetworkModSource& modSource, NetworkModInfo modInfo)
	{
		auto it = FindItem(m_Items, &modSource);
		if (it == m_Items.end())
		{
			return m_Items.emplace_back(&modSource, modInfo);
		}
		else
		{
			it->SetModSource(modSource);
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
	bool ModSourceStore::RemoveItem(const INetworkModSource& modSource)
	{
		auto it = FindItem(m_Items, &modSource);
		if (it != m_Items.end())
		{
			m_Items.erase(it);
			return true;
		}
		return false;
	}

	wxString ModSourceStore::GetModURL(const wxString& name, const GameID& gameID) const
	{
		if (const ModSourceItem* item = GetItem(name))
		{
			return item->GetURL(gameID);
		}
		return wxEmptyString;
	}
	wxString ModSourceStore::GetModURL(const INetworkModSource& modSource, const GameID& gameID) const
	{
		if (const ModSourceItem* item = GetItem(modSource))
		{
			return item->GetURL(gameID);
		}
		return wxEmptyString;
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
	KLabeledValue::Vector ModSourceStore::GetModNamedURLs(const GameID& gameID) const
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
