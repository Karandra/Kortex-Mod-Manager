#include "stdafx.h"
#include "KMandatoryModEntry.h"
#include "KModManager.h"

wxString KMandatoryModEntry::GetModFilesDir() const
{
	return KVarExp(KFixedModEntry::GetModFilesDir());
}
