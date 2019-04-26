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
		for (auto& gameMod: GetMods())
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
		auto& mods = GetMods();
		std::sort(mods.begin(), mods.end(), [](const auto& left, const auto& right)
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
		auto& mods = GetMods();

		// Reset priority
		for (auto& gameMod: mods)
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
		for (auto& gameMod: mods)
		{
			if (gameMod->GetPriority() == -1)
			{
				gameMod->SetPriority(profileModList.size() + count);
			}
		}

		// Sort and invalidate virtual tree
		SortByPriority();
		if (IModDispatcher::HasInstance())
		{
			IModDispatcher::GetInstance()->InvalidateVirtualTree();
		}
	}

	bool IModManager::MoveModsBefore(const IGameMod::RefVector& movedMods, const IGameMod& anchor)
	{
		if (KUPtrVectorUtil::MoveBefore(GetMods(), movedMods, anchor))
		{
			RecalculatePriority();
			return true;
		}
		return false;
	}
	bool IModManager::MoveModsAfter(const IGameMod::RefVector& movedMods, const IGameMod& anchor)
	{
		if (KUPtrVectorUtil::MoveAfter(GetMods(), movedMods, anchor))
		{
			RecalculatePriority();
			return true;
		}
		return false;
	}
	bool IModManager::ChangeModPriority(IGameMod& movedMod, intptr_t targetPriority)
	{
		auto& mods = GetMods();
		if (movedMod.GetPriority() != targetPriority && targetPriority >= 0 && (size_t)targetPriority < mods.size())
		{
			// Extract mod from list
			auto it = mods.begin() + movedMod.GetPriority();
			auto temp = std::move(*it);
			mods.erase(it);

			// Place it into position
			mods.insert(mods.begin() + targetPriority, std::move(temp));

			RecalculatePriority();
			return true;
		}
		return false;
	}
}
