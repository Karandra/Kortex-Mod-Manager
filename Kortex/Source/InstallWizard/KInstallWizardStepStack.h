#pragma once
#include "stdafx.h"
#include "KInstallWizardDefs.h"

class KIWStepStackItem
{
	private:
		KPPCStep* Step = NULL;
		KPPCEntryRefArray CheckedEntries;

	public:
		KIWStepStackItem(KPPCStep* step)
			:Step(step)
		{
		}
		KIWStepStackItem(KPPCStep* step, const KPPCEntryRefArray& checked)
			:Step(step), CheckedEntries(checked)
		{
		}
		
	public:
		KPPCStep* GetStep() const
		{
			return Step;
		}
		const KPPCEntryRefArray& GetChecked() const
		{
			return CheckedEntries;
		}
		KPPCEntryRefArray& GetChecked()
		{
			return CheckedEntries;
		}
};

using KIWStackType = std::stack<KIWStepStackItem>;
class KIWStepStack: private KIWStackType
{
	private:
		using ContainerType = KIWStackType::container_type;
		using iterator = ContainerType::iterator;
		using const_iterator = ContainerType::const_iterator;

	public:
		KIWStepStack();
		~KIWStepStack();

	public:
		void PushStep(KPPCStep* step);
		void PushStep(KPPCStep* step, const KPPCEntryRefArray& checked);
		
		void PopItem()
		{
			pop();
		}
		KIWStepStackItem* GetTopItem()
		{
			return !empty() ? &top() : NULL;
		}
		const KIWStepStackItem* GetTopItem() const
		{
			return !empty() ? &top() : NULL;
		}
		KPPCStep* GetTopStep() const
		{
			return !empty() ? top().GetStep() : NULL;
		}
		void Clear();

		bool IsEmpty() const
		{
			return empty();
		}
		size_t GetSize() const
		{
			return size();
		}
		
		const ContainerType& GetContainer() const
		{
			return c;
		}
		ContainerType& GetContainer()
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
