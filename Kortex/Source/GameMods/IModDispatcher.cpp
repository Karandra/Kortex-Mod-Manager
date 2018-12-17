#include "stdafx.h"
#include "IModDispatcher.h"
#include "IModManager.h"

namespace Kortex
{
	IGameMod* IModDispatcher::DoIterateMods(const IGameMod::RefVector& mods, const IterationFunctor& functor, IterationOrder order) const
	{
		switch (order)
		{
			case IterationOrder::Direct:
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
			case IterationOrder::Reversed:
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

	Kortex::IGameMod* IModDispatcher::IterateModsForward(IterationFunctor functor, bool includeWriteTarget) const
	{
		RebuildTreeIfNeeded();
		return DoIterateMods(IModManager::GetInstance()->GetAllMods(includeWriteTarget), functor, IterationOrder::Direct);
	}
	Kortex::IGameMod* IModDispatcher::IterateModsBackward(IterationFunctor functor, bool includeWriteTarget) const
	{
		RebuildTreeIfNeeded();
		return DoIterateMods(IModManager::GetInstance()->GetAllMods(includeWriteTarget), functor, IterationOrder::Reversed);
	}
}
