#include "stdafx.h"
#include "KFixedModEntry.h"
#include "KImageProvider.h"

bool KFixedModEntry::IsEnabled() const
{
	return KModEntry::IsEnabledUnchecked();
}
bool KFixedModEntry::IsInstalled() const
{
	return KModEntry::IsInstalled();
}
bool KFixedModEntry::IsLinkedMod() const
{
	return KModEntry::IsLinkedMod();
}

KImageEnum KFixedModEntry::GetIcon() const
{
	return KIMG_FOLDERS;
}
int KFixedModEntry::GetOrderIndex() const
{
	return m_Priority;
}
