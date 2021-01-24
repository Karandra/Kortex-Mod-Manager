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
		if (auto instance = GetOwningInstance())
		{
			return instance->GetActiveProfile() == this;
		}
		return false;
	}
}

namespace Kortex::GameInstance
{
	ProfileMod::ProfileMod(const kxf::String& signature, bool active, int priority)
		:m_Signature(signature), m_Priority(priority >= 0 ? priority : std::numeric_limits<int>::max()), m_IsActive(active)
	{
	}
	ProfileMod::ProfileMod(const IGameMod& mod, bool active)
		:m_Signature(mod.GetSignature()), m_Priority(mod.GetPriority()), m_IsActive(active)
	{
	}

	kxf::object_ptr<IGameMod> ProfileMod::GetMod() const
	{
		//if (IModManager* manager = IModManager::GetInstance())
		//{
		//	return manager->FindModBySignature(m_Signature);
		//}
		return nullptr;
	}
}

namespace Kortex::GameInstance
{
	ProfilePlugin::ProfilePlugin(const IGamePlugin& plugin, bool active)
		:m_Name(plugin.GetName()), m_Priority(plugin.GetPriority()), m_IsActive(active)
	{
	}
	ProfilePlugin::ProfilePlugin(const kxf::String& name, bool active, int priority)
		: m_Name(name), m_Priority(priority >= 0 ? priority : std::numeric_limits<int>::max()), m_IsActive(active)
	{
	}

	kxf::object_ptr<IGamePlugin> ProfilePlugin::GetPlugin() const
	{
		//if (IPluginManager* manager = IPluginManager::GetInstance())
		//{
		//	return manager->FindPluginByName(m_Name);
		//}
		return nullptr;
	}
}
