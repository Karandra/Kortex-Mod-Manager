#pragma once
#include "stdafx.h"
#include "Application/BroadcastProcessor.h"

namespace Kortex
{
	class IProgramItem;
}

namespace Kortex
{
	class ProgramEvent: public BroadcastEvent
	{
		public:
			using RefVector = std::vector<IProgramItem*>;

		public:
			KxEVENT_MEMBER(ProgramEvent, Added);
			KxEVENT_MEMBER(ProgramEvent, Removed);
			KxEVENT_MEMBER(ProgramEvent, Changed);
			KxEVENT_MEMBER(ProgramEvent, Refreshed);

		private:
			RefVector m_Items;
			IProgramItem* m_Item = nullptr;

		public:
			ProgramEvent() = default;
			ProgramEvent(IProgramItem& item)
				:m_Item(&item)
			{
			}
			ProgramEvent(RefVector items)
				:m_Items(std::move(items))
			{
			}

			ProgramEvent* Clone() const override
			{
				return new ProgramEvent(*this);
			}

		public:
			bool HasSave() const
			{
				return m_Item != nullptr;
			}
			IProgramItem* GetItem() const
			{
				return m_Item;
			}

			bool HasItemsArray() const
			{
				return !m_Items.empty();
			}
			const RefVector& GetItems() const
			{
				return m_Items;
			}
	};
}
