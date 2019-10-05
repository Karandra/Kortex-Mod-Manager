#include "stdafx.h"
#include "KPackageCreatorIDTracker.h"

namespace Kortex::PackageDesigner
{
	void KPackageCreatorIDTracker::TrackID_ReplaceInStringVector(const wxString& trackedID, const wxString& newID, KxStringVector& list) const
	{
		for (auto it = list.begin(); it != list.end(); ++it)
		{
			if (*it == trackedID)
			{
				*it = newID;
				break;
			}
		}
	}
	void KPackageCreatorIDTracker::TrackID_RemoveFromStringVector(const wxString& trackedID, KxStringVector& list) const
	{
		auto it = std::find(list.begin(), list.end(), trackedID);
		if (it != list.end())
		{
			list.erase(it);
		}
	}
	
	void KPackageCreatorIDTracker::TrackID_ReplaceInFlagVector(const wxString& trackedID, const wxString& newID, KPPCFlagEntry::Vector& list) const
	{
		for (auto it = list.begin(); it != list.end(); ++it)
		{
			if (it->GetName() == trackedID)
			{
				it->SetName(newID);
				break;
			}
		}
	}
	void KPackageCreatorIDTracker::TrackID_RemoveFromFlagVector(const wxString& trackedID, KPPCFlagEntry::Vector& list) const
	{
		auto it = std::find_if(list.begin(), list.end(), [&trackedID](const KPPCFlagEntry& flagEntry)
		{
			return flagEntry.GetName() == trackedID;
		});
		if (it != list.end())
		{
			list.erase(it);
		}
	}
}
