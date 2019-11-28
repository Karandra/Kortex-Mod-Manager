#include "stdafx.h"
#include "IDTracker.h"

namespace Kortex::PackageDesigner
{
	void IDTracker::TrackID_ReplaceInStringVector(const wxString& trackedID, const wxString& newID, KxStringVector& list) const
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
	void IDTracker::TrackID_RemoveFromStringVector(const wxString& trackedID, KxStringVector& list) const
	{
		auto it = std::find(list.begin(), list.end(), trackedID);
		if (it != list.end())
		{
			list.erase(it);
		}
	}
	
	void IDTracker::TrackID_ReplaceInFlagVector(const wxString& trackedID, const wxString& newID, KPPCFlagEntry::Vector& list) const
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
	void IDTracker::TrackID_RemoveFromFlagVector(const wxString& trackedID, KPPCFlagEntry::Vector& list) const
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
