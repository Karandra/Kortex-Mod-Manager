#pragma once
#include "stdafx.h"
#include "Common.h"

namespace Kortex::InstallWizard
{
	class StepStackItem
	{
		private:
			PackageDesigner::KPPCStep* m_Step = nullptr;
			PackageDesigner::KPPCEntry::RefVector m_CheckedEntries;

		public:
			StepStackItem(PackageDesigner::KPPCStep* step, const PackageDesigner::KPPCEntry::RefVector& checked = {})
				:m_Step(step), m_CheckedEntries(checked)
			{
			}
			
		public:
			PackageDesigner::KPPCStep* GetStep() const
			{
				return m_Step;
			}
			const PackageDesigner::KPPCEntry::RefVector& GetChecked() const
			{
				return m_CheckedEntries;
			}
			PackageDesigner::KPPCEntry::RefVector& GetChecked()
			{
				return m_CheckedEntries;
			}
	};
}

namespace Kortex::InstallWizard
{
	class StepStack: private std::stack<StepStackItem>
	{
		private:
			using TItem = StepStackItem;
			using iterator = container_type::iterator;
			using const_iterator = container_type::const_iterator;

		public:
			StepStack() = default;
			~StepStack() = default;

		public:
			void PushStep(PackageDesigner::KPPCStep* step, const PackageDesigner::KPPCEntry::RefVector& checked = {})
			{
				push({step, checked});
			}
			void PopItem()
			{
				pop();
			}

			StepStackItem* GetTopItem()
			{
				return !empty() ? &top() : nullptr;
			}
			const StepStackItem* GetTopItem() const
			{
				return !empty() ? &top() : nullptr;
			}
			PackageDesigner::KPPCStep* GetTopStep() const
			{
				return !empty() ? top().GetStep() : nullptr;
			}
			void Clear()
			{
				while (!empty())
				{
					pop();
				}
			}

			bool IsEmpty() const
			{
				return empty();
			}
			size_t GetSize() const
			{
				return size();
			}
			
			const container_type& GetContainer() const
			{
				return c;
			}
			container_type& GetContainer()
			{
				return c;
			}

			iterator begin()
			{
				return GetContainer().begin();
			}
			iterator end()
			{
				return GetContainer().end();
			}
			const_iterator begin() const
			{
				return GetContainer().cbegin();
			}
			const_iterator end() const
			{
				return GetContainer().cend();
			}
	};
}
