#include "pch.hpp"
#include "GameID.h"
#include "IGameDefinition.h"

namespace Kortex
{
	GameID::GameID(const IGameDefinition& definition)
		:m_ID(definition.GetGameID())
	{
	}
}
