#include "stdafx.h"
#include "GameID.h"
#include "IGameInstance.h"

namespace Kortex
{
	GameID::GameID(const IGameInstance& instance)
		:m_ID(instance.GetGameID())
	{
	}
	GameID::GameID(const IGameInstance* instance)
		:m_ID(instance ? instance->GetGameID() : wxString())
	{
	}
}
