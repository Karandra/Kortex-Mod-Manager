#include "stdafx.h"
#include "IModDispatcher.h"
#include "IModManager.h"

namespace Kortex
{
	IGameMod* IModDispatcher::DoIterateMods(const IGameMod::RefVector& mods, const IterationFunctor& functor, IterationOrder order) const
	{
		switch (order)
		{
			case IterationOrder::Forward:
			{
				for (IGameMod* mod: mods)
				{
					if (!functor(*mod))
					{
						return mod;
					}
				}
				break;
			}
			case IterationOrder::Backward:
			{
				for (auto it = mods.rbegin(); it != mods.rend(); ++it)
				{
					if (!functor(**it))
					{
						return *it;
					}
				}
				break;
			}
		};
		return nullptr;
	}

	IGameMod* IModDispatcher::IterateModsForward(IterationFunctor functor, ModManager::GetModsFlags flags) const
	{
		return DoIterateMods(IModManager::GetInstance()->GetMods(flags), functor, IterationOrder::Forward);
	}
	IGameMod* IModDispatcher::IterateModsBackward(IterationFunctor functor, ModManager::GetModsFlags flags) const
	{
		return DoIterateMods(IModManager::GetInstance()->GetMods(flags), functor, IterationOrder::Backward);
	}
}
