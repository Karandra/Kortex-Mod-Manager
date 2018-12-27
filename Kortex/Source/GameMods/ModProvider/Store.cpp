#include "stdafx.h"
#include "Store.h"

namespace Kortex
{
	wxString ModProviderStore::GetModURL(const wxString& name, const GameID& gameID) const
	{
		if (const ModProviderItem* item = GetItem(name))
		{
			return item->GetURL(gameID);
		}
		return wxEmptyString;
	}
	wxString ModProviderStore::GetModURL(INetworkProvider* provider, const GameID& gameID) const
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
