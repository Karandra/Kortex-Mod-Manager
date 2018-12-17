#pragma once
#include "stdafx.h"

namespace Kortex
{
	class IGameInstance;

	class GameID
	{
		private:
			wxString m_ID;

		public:
			GameID(const wxString& id = wxEmptyString)
				:m_ID(id)
			{
			}
			GameID(const IGameInstance& instance);
			GameID(const IGameInstance* instance);

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
		
			bool operator==(const GameID& other) const
			{
				return m_ID == other.m_ID;
			}
			bool operator==(const wxString& other) const
			{
				return m_ID == other;
			}
			bool operator!=(const GameID& other) const
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
}

namespace Kortex::GameIDs
{
	static const GameID NullGameID = GameID();

	// TES
	static const GameID Morrowind = GameID("Morrowind");
	static const GameID Oblivion = GameID("Oblivion");
	static const GameID Skyrim = GameID("Skyrim");
	static const GameID SkyrimSE = GameID("SkyrimSE");
	static const GameID SkyrimVR = GameID("SkyrimVR");

	// Fallout
	static const GameID Fallout3 = GameID("Fallout3");
	static const GameID FalloutNV = GameID("FalloutNV");
	static const GameID Fallout4 = GameID("Fallout4");
	static const GameID Fallout4VR = GameID("Fallout4VR");
};
