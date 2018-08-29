#include "stdafx.h"
#include "KFixedModEntry.h"
#include "KImageProvider.h"

bool KFixedModEntry::IsEnabled() const
{
	return KModEntry::IsEnabled();
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
intptr_t KFixedModEntry::GetOrderIndex() const
{
	return m_Priority;
}
