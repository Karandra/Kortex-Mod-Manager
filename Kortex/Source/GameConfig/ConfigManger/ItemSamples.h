#pragma once
#include "stdafx.h"
#include "Common.h"
#include "ItemValue.h"
class KxXMLNode;

namespace Kortex::GameConfig
{
	class Item;

	class ItemSamples
	{
		private:
			Item& m_Item;
			SortOrderValue m_SortOrder;
			SortOptionsValue m_SortOptions;
			std::vector<ItemValue> m_Values;

		private:
			void LoadSamples(const KxXMLNode& rootNode);

		public:
			ItemSamples(Item& item, const KxXMLNode& node);

		public:
			Item& GetItem() const
			{
				return m_Item;
			}
			SortOrderValue GetSortOrder() const
			{
				return m_SortOrder;
			}
			SortOptionsValue GetSortOptions() const
			{
				return m_SortOptions;
			}
	};
}
