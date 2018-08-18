#pragma once
#include "stdafx.h"
#include "KModEntry.h"
enum KImageEnum;

class KFixedModEntry: public KModEntry
{
	private:
		int m_Priority = -1;

	public:
		KFixedModEntry(int priority = -1)
			:m_Priority(priority)
		{
		}

	public:
		virtual const KFixedModEntry* ToFixedEntry() const override
		{
			return this;
		}
		virtual KFixedModEntry* ToFixedEntry() override
		{
			return this;
		}

		virtual bool IsEnabled() const override;
		virtual bool IsInstalled() const override;
		virtual bool IsLinkedMod() const override;

		virtual KImageEnum GetIcon() const override;
		virtual int GetPriority() const override
		{
			return m_Priority;
		}
		virtual int GetOrderIndex() const override;
};

