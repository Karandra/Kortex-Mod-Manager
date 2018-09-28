#include "stdafx.h"
#include "KFixedModEntry.h"
#include "KImageProvider.h"

bool KFixedModEntry::IsEnabled() const
{
	return true;
}
bool KFixedModEntry::IsInstalled() const
{
	return true;
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
