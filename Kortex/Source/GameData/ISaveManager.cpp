#include "stdafx.h"
#include "ISaveManager.h"
#include "GameData/GameDataModule.h"
#include "KxFramework/KxUtility.h"

namespace Kortex
{
	namespace SaveManager::Internal
	{
		const SimpleManagerInfo TypeInfo("SaveManager", "SaveManager.Name");
	}

	ISaveManager::ISaveManager()
		:ManagerWithTypeInfo(GameDataModule::GetInstance())
	{
	}
	ISaveManager::~ISaveManager()
	{
	}

	bool ISaveManager::RemoveSave(IGameSave& save)
	{
		auto it = std::find_if(m_Saves.begin(), m_Saves.end(), [&save](auto& value)
		{
			return value.get() == &save;
		});
		if (it != m_Saves.end())
		{
			m_Saves.erase(it);
			return true;
		}
		return false;
	}
	IGameSave::RefVector ISaveManager::GetSaves() const
	{
		return KxUtility::ConvertVector<IGameSave*>(m_Saves, [](auto& value)
		{
			return value.get();
		});
	}
}
