#pragma once
#include "stdafx.h"
class KGameInstance;

class KGameID
{
	private:
		wxString m_ID;

	public:
		KGameID(const wxString& id = wxEmptyString)
			:m_ID(id)
		{
		}
		KGameID(const KGameInstance& instance);
		KGameID(const KGameInstance* instance);

	public:
		bool IsOK() const
		{
			return !m_ID.IsEmpty();
		}
		
		explicit operator bool() const
		{
			return IsOK();
		}
		bool operator!() const
		{
			return !IsOK();
		}
		
		bool operator==(const KGameID& other) const
		{
			return m_ID == other.m_ID;
		}
		bool operator==(const wxString& other) const
		{
			return m_ID == other;
		}
		bool operator!=(const KGameID& other) const
		{
			return m_ID != other.m_ID;
		}
		bool operator!=(const wxString& other) const
		{
			return m_ID != other;
		}

		wxString ToString() const
		{
			return m_ID;
		}
		operator const wxString&() const
		{
			return m_ID;
		}
};

namespace KGameIDs
{
	static const KGameID NullGameID = KGameID();

	// TES
	static const KGameID Morrowind = KGameID("Morrowind");
	static const KGameID Oblivion = KGameID("Oblivion");
	static const KGameID Skyrim = KGameID("Skyrim");
	static const KGameID SkyrimSE = KGameID("SkyrimSE");
	static const KGameID SkyrimVR = KGameID("SkyrimVR");

	// Fallout
	static const KGameID Fallout3 = KGameID("Fallout3");
	static const KGameID FalloutNV = KGameID("FalloutNV");
	static const KGameID Fallout4 = KGameID("Fallout4");
	static const KGameID Fallout4VR = KGameID("Fallout4VR");
};
