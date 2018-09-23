#pragma once
#include "stdafx.h"
#include "KFixedModEntry.h"
enum KImageEnum;

class KPriorityGroupEntry: public KFixedModEntry
{
	public:
		KModEntry* m_BaseMod = NULL;
		bool m_IsBegin = false;

	public:
		KPriorityGroupEntry(KModEntry* baseMod, bool isBegin)
			:m_BaseMod(baseMod), m_IsBegin(isBegin)
		{
		}

	public:
		virtual const KPriorityGroupEntry* ToPriorityGroup() const override
		{
			return this;
		}
		virtual KPriorityGroupEntry* ToPriorityGroup() override
		{
			return this;
		}

		virtual bool IsEnabled() const override
		{
			return KModEntry::IsEnabled();
		}
		virtual bool IsInstalled() const override
		{
			return false;
		}
		virtual bool IsLinkedMod() const override
		{
			return false;
		}

		virtual KImageEnum GetIcon() const override;
		virtual intptr_t GetPriority() const override;
		virtual intptr_t GetOrderIndex() const override;

		KModEntry* GetBaseMod() const
		{
			return m_BaseMod;
		}

		bool IsBegin() const
		{
			return m_IsBegin;
		}
		bool IsEnd() const
		{
			return !m_IsBegin;
		}
};
