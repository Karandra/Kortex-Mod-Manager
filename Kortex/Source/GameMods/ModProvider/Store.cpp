#include "stdafx.h"
#include "Store.h"

namespace Kortex::ModProvider
{
	wxString Store::GetModURL(const wxString& name, const GameID& gameID) const
	{
		if (const Item* item = GetItem(name))
		{
			return item->GetURL(gameID);
		}
		return wxEmptyString;
	}
	wxString Store::GetModURL(INetworkProvider* provider, const GameID& gameID) const
	{
		if (const Item* item = GetItem(provider))
		{
			return item->GetURL(gameID);
		}
		return wxEmptyString;
	}
	
	KxStringVector Store::GetModURLs(const GameID& gameID) const
	{
		KxStringVector items;
		items.reserve(m_Items.size());

		for (const Item& item: m_Items)
		{
			items.push_back(item.GetURL(gameID));
		}
		return items;
	}
	KLabeledValue::Vector Store::GetModNamedURLs(const GameID& gameID) const
	{
		KLabeledValue::Vector items;
		items.reserve(m_Items.size());

		for (const Item& item: m_Items)
		{
			items.emplace_back(item.GetURL(gameID), item.GetName());
		}
		return items;
	}

	void Store::LoadTryAdd(const KxXMLNode& arrayNode)
	{
		LoadHelper<LoadMode::TryAdd>(*this, arrayNode);
	}
	void Store::LoadAssign(const KxXMLNode& arrayNode)
	{
		LoadHelper<LoadMode::Assign>(*this, arrayNode);
	}
	void Store::Save(KxXMLNode& arrayNode) const
	{
		for (const Item& item: m_Items)
		{
			KxXMLNode node = arrayNode.NewElement(wxS("Entry"));
			item.Save(node);
		}
	}
}
