#include "stdafx.h"
#include "IModManager.h"
#include "IModDispatcher.h"
#include "GameMods/GameModsModule.h"
#include <Kortex/GameInstance.hpp>
#include "Utility/KUPtrVectorUtil.h"

namespace Kortex
{
	namespace ModManager::Internal
	{
		const SimpleManagerInfo TypeInfo("ModManager", "ModManager.Name");
	}

	void IModManager::RecalculatePriority(size_t startAt)
	{
		intptr_t priority = 0;
		for (auto& gameMod: m_Mods)
		{
			if (gameMod->GetPriority() != -1)
			{
				gameMod->SetPriority(priority);
				priority++;
			}
		}
	}
	void IModManager::SortByPriority()
	{
		std::sort(m_Mods.begin(), m_Mods.end(), [](const auto& left, const auto& right)
		{
			return left->GetPriority() < right->GetPriority();
		});
	}

	IModManager::IModManager()
		:ManagerWithTypeInfo(GameModsModule::GetInstance())
	{
	}

	void IModManager::ResortMods()
	{
		ResortMods(*IGameInstance::GetActiveProfile());
	}
	void IModManager::ResortMods(const IGameProfile& profile)
	{
		// Reset priority
		for (auto& gameMod: m_Mods)
		{
			gameMod->SetPriority(-1);
		}

		const auto& profileModList = profile.GetMods();
		for (const GameInstance::ProfileMod& profileMod: profileModList)
		{
			if (IGameMod* gameMod = FindModBySignature(profileMod.GetSignature()))
			{
				gameMod->SetPriority(profileMod.GetPriority());
				gameMod->SetActive(profileMod.IsActive());
			}
		}

		// Set priority for all unsorted mods to be after sorted
		intptr_t count = 0;
		for (auto& gameMod: m_Mods)
		{
			if (gameMod->GetPriority() == -1)
			{
				gameMod->SetPriority(profileModList.size() + count);
			}
		}

		// Sort and invalidate virtual tree
		SortByPriority();
		IModDispatcher::GetInstance()->InvalidateVirtualTree();
	}

	bool IModManager::MoveModsBefore(const IGameMod::RefVector& movedMods, const IGameMod& anchor)
	{
		if (KUPtrVectorUtil::MoveBefore(m_Mods, movedMods, anchor))
		{
			RecalculatePriority();
			return true;
		}
		return false;
	}
	bool IModManager::MoveModsAfter(const IGameMod::RefVector& movedMods, const IGameMod& anchor)
	{
		if (KUPtrVectorUtil::MoveAfter(m_Mods, movedMods, anchor))
		{
			RecalculatePriority();
			return true;
		}
		return false;
	}
	bool IModManager::ChangeModPriority(IGameMod& movedMod, intptr_t targetPriority)
	{
		if (movedMod.GetPriority() != targetPriority && targetPriority >= 0 && (size_t)targetPriority < m_Mods.size())
		{
			// Extract mod from list
			auto it = m_Mods.begin() + movedMod.GetPriority();
			auto temp = std::move(*it);
			m_Mods.erase(it);

			// Place it into position
			m_Mods.insert(m_Mods.begin() + targetPriority, std::move(temp));

			RecalculatePriority();
			return true;
		}
		return false;
	}
}
