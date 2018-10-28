#pragma once
#include "stdafx.h"
#include "KModEntry.h"
enum KImageEnum;

class KFixedModEntry: public KModEntry
{
	private:
		intptr_t m_Priority = -1;

	public:
		KFixedModEntry(intptr_t priority = -1)
			:m_Priority(priority)
		{
		}

	public:
		virtual void CreateFromSignature(const wxString& signature) override
		{
		}
		virtual bool Save() override
		{
			return false;
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
		virtual intptr_t GetPriority() const override
		{
			return m_Priority;
		}
		virtual intptr_t GetOrderIndex() const override;
};

