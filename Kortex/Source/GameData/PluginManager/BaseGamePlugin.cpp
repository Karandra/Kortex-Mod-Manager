#include "stdafx.h"
#include "BaseGamePlugin.h"
#include "GameData/IPluginManager.h"

namespace Kortex::PluginManager
{
	const StdContentEntry* BaseGamePlugin::GetStdContentEntry() const
	{
		if (m_StdContent == nullptr)
		{
			m_StdContent = IPluginManager::GetInstance()->GetConfig().GetStandardContent(m_FileItem.GetName());
		}
		return m_StdContent;
	}
}
