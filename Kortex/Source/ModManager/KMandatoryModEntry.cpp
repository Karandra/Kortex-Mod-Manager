#include "stdafx.h"
#include "KMandatoryModEntry.h"
#include "KModManager.h"

wxString KMandatoryModEntry::GetLocation(KModManagerLocation index) const
{
	wxString location = KFixedModEntry::GetLocation(index);
	if (index == KMM_LOCATION_MOD_FILES)
	{
		location = V(location);
	}
	return location;
}
