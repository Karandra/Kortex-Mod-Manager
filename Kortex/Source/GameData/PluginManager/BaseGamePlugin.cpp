#include "stdafx.h"
#include "BaseGamePlugin.h"
#include "IPluginManager.h"
#include <Kortex/Events.hpp>

namespace Kortex::PluginManager
{
	const IGameMod* BaseGamePlugin::GetOwningMod() const
	{
		if (m_OwningMod == nullptr)
		{
			m_OwningMod = IPluginManager::GetInstance()->FindOwningMod(*this);
		}
		return m_OwningMod;
	}
	const StdContentEntry* BaseGamePlugin::GetStdContentEntry() const
	{
		if (m_StdContent == nullptr)
		{
			m_StdContent = IPluginManager::GetInstance()->GetConfig().GetStandardContent(m_FileItem.GetName());
		}
		return m_StdContent;
	}
}
