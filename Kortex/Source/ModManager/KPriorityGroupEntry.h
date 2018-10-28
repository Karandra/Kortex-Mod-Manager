#pragma once
#include "stdafx.h"
#include "KFixedModEntry.h"
enum KImageEnum;
class KModTag;

class KPriorityGroupEntry: public KFixedModEntry
{
	public:
		KModEntry* m_BaseMod = NULL;
		KModTag* m_Tag = NULL;
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

		virtual bool HasColor() const override;
		virtual KxColor GetColor() const override;
		virtual void SetColor(const KxColor& color) override;

		KModEntry* GetBaseMod() const
		{
			return m_BaseMod;
		}
		
		KModTag* GetTag() const
		{
			return m_Tag;
		}
		void SetTag(KModTag* tag)
		{
			m_Tag = tag;
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
