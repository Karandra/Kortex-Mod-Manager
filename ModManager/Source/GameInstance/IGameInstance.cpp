#include "pch.hpp"
#include "IGameInstance.h"
#include "IGameProfile.h"
#include "Application/IApplication.h"
#include "Application/SystemApplication.h"
#include <kxf/FileSystem/FileItem.h>
#include <kxf/System/ShellOperations.h>
#include <kxf/Drawing/Common.h>

namespace Kortex
{
	bool IGameInstance::IsActive() const
	{
		return IApplication::GetInstance().GetActiveGameInstance() == this;
	}
	IGameProfile* IGameInstance::GetProfile(const kxf::String& id)
	{
		IGameProfile* result = nullptr;
		EnumProfiles([&](IGameProfile& profile)
		{
			if (profile.GetID() == id)
			{
				result = &profile;
				return false;
			}
			return true;
		});
		return result;
	}
}
