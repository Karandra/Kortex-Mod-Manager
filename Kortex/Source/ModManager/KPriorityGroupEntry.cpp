#include "stdafx.h"
#include "KPriorityGroupEntry.h"
#include "KModTagsManager.h"
#include "KImageProvider.h"

KImageEnum KPriorityGroupEntry::GetIcon() const
{
	return KIMG_NONE;
}

intptr_t KPriorityGroupEntry::GetPriority() const
{
	return m_BaseMod->GetPriority();
}
intptr_t KPriorityGroupEntry::GetOrderIndex() const
{
	return m_BaseMod->GetOrderIndex() + (m_IsBegin ? -1 : +1);
}

bool KPriorityGroupEntry::HasColor() const
{
	return m_Tag && m_Tag->HasColor();
}
KxColor KPriorityGroupEntry::GetColor() const
{
	return m_Tag ? m_Tag->GetColor() : KxColor();
}
void KPriorityGroupEntry::SetColor(const KxColor& color)
{
	if (m_Tag)
	{
		m_Tag->SetColor(color);
	}
}
