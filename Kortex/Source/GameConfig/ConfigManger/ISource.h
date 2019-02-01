#pragma once
#include "stdafx.h"
#include "Common.h"

namespace Kortex::GameConfig
{
	class Item;
	class ItemValue;
	class ItemGroup;

	class ISource
	{
		public:
			virtual ~ISource() = default;

		public:
			virtual SourceTypeValue GetType() const = 0;
			virtual SourceFormatValue GetFormat() const = 0;

			virtual bool IsOpened() const = 0;
			virtual bool Open() = 0;
			virtual bool Save() = 0;
			virtual void Close() = 0;

			virtual bool WriteValue(const Item& item, const ItemValue& value) = 0;
			virtual bool ReadValue(Item& item, ItemValue& value) const = 0;
			virtual void LoadUnknownItems(ItemGroup& group) = 0;
	};
}
