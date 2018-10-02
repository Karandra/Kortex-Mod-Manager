#pragma once
#include "stdafx.h"
#include "KFixedModEntry.h"
enum KImageEnum;

class KMandatoryModEntry: public KFixedModEntry
{
	public:
		KMandatoryModEntry(intptr_t priority = -1)
			:KFixedModEntry(priority)
		{
		}

	public:
		virtual bool IsLinkedMod() const override
		{
			return true;
		}
		virtual wxString GetLocation(KModManagerLocation index) const override;
};
