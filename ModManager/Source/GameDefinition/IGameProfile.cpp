#include "pch.hpp"
#include "IGameInstance.h"
#include "IGameProfile.h"
#include "IGameMod.h"
#include "IGamePlugin.h"
#include "Application/IApplication.h"
#include "Modules/GameModManager/IGameModManager.h"

namespace
{
	constexpr int CorrectOrder(int order) noexcept
	{
		return order >= 0 ? order : std::numeric_limits<int>::max();
	}
}

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
	GameProfileMod::GameProfileMod(const IGameMod& mod)
		:m_Signature(mod.GetSignature()), m_Order(mod.GetOrder()), m_IsActive(mod.IsActive())
	{
	}
	GameProfileMod::GameProfileMod(const IGameMod& mod, bool active, int order)
		:m_Signature(mod.GetSignature()), m_Order(CorrectOrder(order)), m_IsActive(active)
	{
	}
	GameProfileMod::GameProfileMod(kxf::String signature, bool active, int order)
		:m_Signature(std::move(signature)), m_Order(CorrectOrder(order)), m_IsActive(active)
	{
	}

	IGameMod* GameProfileMod::ResolveMod() const
	{
		if (auto manager = IApplication::GetInstance().GetModule<IGameModManager>())
		{
			return manager->GetModBySignature(m_Signature);
		}
		return nullptr;
	}
}

namespace Kortex
{
	GameProfilePlugin::GameProfilePlugin(const IGamePlugin& plugin)
		:m_Name(plugin.GetName()), m_Order(plugin.GetOrder()), m_IsActive(plugin.IsActive())
	{
	}
	GameProfilePlugin::GameProfilePlugin(const IGamePlugin& plugin, bool active, int order)
		:m_Name(plugin.GetName()), m_Order(CorrectOrder(order)), m_IsActive(active)
	{
	}
	GameProfilePlugin::GameProfilePlugin(kxf::String name, bool active, int order)
		: m_Name(std::move(name)), m_Order(CorrectOrder(order)), m_IsActive(active)
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
