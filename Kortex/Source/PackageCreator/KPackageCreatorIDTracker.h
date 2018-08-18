#pragma once
#include "stdafx.h"
#include "PackageProject/KPackageProjectComponents.h"
#include "PackageProject/KPackageProject.h"

class KPackageCreatorIDTracker
{
	protected:
		void TrackID_ReplaceOrRemove(const wxString& trackedID, const wxString& newID, KxStringVector& list, bool remove) const
		{
			if (remove)
			{
				TrackID_RemoveFromStringsList(trackedID, list);
			}
			else
			{
				TrackID_ReplaceInStringsList(trackedID, newID, list);
			}
		}
		void TrackID_ReplaceInStringsList(const wxString& trackedID, const wxString& newID, KxStringVector& list) const;
		void TrackID_RemoveFromStringsList(const wxString& trackedID, KxStringVector& list) const;

		void TrackID_ReplaceInStringsList(const wxString& trackedID, const wxString& newID, KPPCFlagEntryArray& list) const;
		void TrackID_RemoveFromStringsList(const wxString& trackedID, KPPCFlagEntryArray& list) const;
		void TrackID_ReplaceOrRemove(const wxString& trackedID, const wxString& newID, KPPCFlagEntryArray& list, bool remove) const
		{
			if (remove)
			{
				TrackID_RemoveFromStringsList(trackedID, list);
			}
			else
			{
				TrackID_ReplaceInStringsList(trackedID, newID, list);
			}
		}

	protected:
		virtual bool TrackChangeID(const wxString& trackedID, const wxString& newID) = 0;
		virtual bool TrackRemoveID(const wxString& trackedID) = 0;
		virtual bool TrackAddID(const wxString& trackedID)
		{
			return false;
		}
};
