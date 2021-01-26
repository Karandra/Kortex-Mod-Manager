#include "pch.hpp"
#include "IGameInstance.h"
#include "IGameProfile.h"
#include "IGameMod.h"
#include "IGamePlugin.h"

namespace Kortex
{
	bool IGameProfile::ValidateName(const kxf::String& name, kxf::String* validName)
	{
		// Using same rules
		return IGameInstance::ValidateName(name, validName);
	}

	bool IGameProfile::IsActive() const
	{
		if (!IsNull())
		{
			return GetOwningInstance().GetActiveProfile() == this;
		}
		return false;
	}
}

namespace Kortex
{
	GameProfileMod::GameProfileMod(kxf::String signature, bool active, int priority)
		:m_Signature(std::move(signature)), m_Priority(priority >= 0 ? priority : std::numeric_limits<int>::max()), m_IsActive(active)
	{
	}
	GameProfileMod::GameProfileMod(const IGameMod& mod, bool active)
		:m_Signature(mod.GetSignature()), m_Priority(mod.GetPriority()), m_IsActive(active)
	{
	}

	IGameMod* GameProfileMod::ResolveMod() const
	{
		//if (IModManager* manager = IModManager::GetInstance())
		//{
		//	return manager->FindModBySignature(m_Signature);
		//}
		return nullptr;
	}
}

namespace Kortex
{
	GameProfilePlugin::GameProfilePlugin(const IGamePlugin& plugin, bool active)
		:m_Name(plugin.GetName()), m_Priority(plugin.GetPriority()), m_IsActive(active)
	{
	}
	GameProfilePlugin::GameProfilePlugin(kxf::String name, bool active, int priority)
		: m_Name(std::move(name)), m_Priority(priority >= 0 ? priority : std::numeric_limits<int>::max()), m_IsActive(active)
	{
	}

	IGamePlugin* GameProfilePlugin::ResolvePlugin() const
	{
		//if (IPluginManager* manager = IPluginManager::GetInstance())
		//{
		//	return manager->FindPluginByName(m_Name);
		//}
		return nullptr;
	}
}
