#include "pch.hpp"
#include "IGameInstance.h"
#include "IGameDefinition.h"
#include "IGameProfile.h"
#include "Application/IApplication.h"
#include "Application/SystemApplication.h"
#include <kxf/FileSystem/FileItem.h>
#include <kxf/System/ShellOperations.h>
#include <kxf/Drawing/Common.h>

namespace Kortex
{
	bool IGameInstance::ValidateName(const kxf::String& name, kxf::String* validName)
	{
		return IGameDefinition::ValidateName(name, validName);
	}

	bool IGameInstance::IsActive() const
	{
		return IApplication::GetInstance().GetActiveGameInstance() == this;
	}
	IGameProfile* IGameInstance::GetProfile(const kxf::String& profileName)
	{
		IGameProfile* result = nullptr;
		EnumProfiles([&](IGameProfile& profile)
		{
			if (profile.GetName() == profileName)
			{
				result = &profile;
				return false;
			}
			return true;
		});
		return result;
	}
}
