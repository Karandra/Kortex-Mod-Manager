#include "pch.hpp"
#include "GameID.h"
#include "IGameInstance.h"

namespace Kortex
{
	GameID::GameID(const IGameInstance& instance)
		:m_ID(instance ? std::move(instance.GetGameID().m_ID) : kxf::NullString)
	{
	}
}

namespace Kortex::GameIDs
{
	namespace TheElderScrolls
	{
		const GameID Morrowind = GameID("Morrowind");
		const GameID Oblivion = GameID("Oblivion");
		const GameID Skyrim = GameID("Skyrim");
		const GameID SkyrimSE = GameID("SkyrimSE");
		const GameID SkyrimVR = GameID("SkyrimVR");
	}
	namespace Fallout
	{
		const GameID Fallout3 = GameID("Fallout3");
		const GameID FalloutNV = GameID("FalloutNV");
		const GameID Fallout4 = GameID("Fallout4");
		const GameID Fallout4VR = GameID("Fallout4VR");
	}
	
	const GameID Sacred2 = GameID("Sacred2");
}
