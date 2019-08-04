#include "stdafx.h"
#include "IModTag.h"
#include <Kortex/ModTagManager.hpp>

namespace Kortex
{
	bool IModTag::IsDefaultTag() const
	{
		IModTagManager* manager = IModTagManager::GetInstance();
		return manager && manager->FindTagByID(manager->GetDefaultTags(), GetID()) != nullptr;
	}
}
