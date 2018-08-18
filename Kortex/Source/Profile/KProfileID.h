#pragma once
#include "stdafx.h"
class KProfile;

class KProfileID
{
	private:
		wxString m_ID;

	public:
		KProfileID(const wxString& id = wxEmptyString)
			:m_ID(id)
		{
		}
		KProfileID(const KProfile* profile);

	public:
		bool IsOK() const
		{
			return !m_ID.IsEmpty();
		}
		
		operator const wxString&() const
		{
			return m_ID;
		}
};

namespace KProfileIDs
{
	static const KProfileID NullProfileID = KProfileID();

	// TES
	static const KProfileID Morrowind = KProfileID("Morrowind");
	static const KProfileID Oblivion = KProfileID("Oblivion");
	static const KProfileID Skyrim = KProfileID("Skyrim");
	static const KProfileID SkyrimSE = KProfileID("SkyrimSE");
	static const KProfileID SkyrimVR = KProfileID("SkyrimVR");

	// Fallout
	static const KProfileID Fallout3 = KProfileID("Fallout3");
	static const KProfileID FalloutNV = KProfileID("FalloutNV");
	static const KProfileID Fallout4 = KProfileID("Fallout4");
	static const KProfileID Fallout4VR = KProfileID("Fallout4VR");
};
