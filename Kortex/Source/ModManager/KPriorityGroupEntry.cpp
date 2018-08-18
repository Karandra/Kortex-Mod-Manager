#include "stdafx.h"
#include "KPriorityGroupEntry.h"
#include "KImageProvider.h"

KImageEnum KPriorityGroupEntry::GetIcon() const
{
	return KIMG_NONE;
}

int KPriorityGroupEntry::GetPriority() const
{
	return m_BaseMod->GetPriority();
}
int KPriorityGroupEntry::GetOrderIndex() const
{
	return m_BaseMod->GetOrderIndex() + (m_IsBegin ? -1 : +1);
}
