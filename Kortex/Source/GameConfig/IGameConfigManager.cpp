#include "stdafx.h"
#include "IGameConfigManager.h"
#include <Kortex/Application.hpp>

namespace Kortex
{
	wxString IGameConfigManager::GetDefinitionFileByID(const wxString& id)
	{
		return IApplication::GetInstance()->GetDataFolder() + wxS("\\ConfigDefinitions\\") + id + wxS(".xml");
	}

	IGameConfigManager::IGameConfigManager()
	{
	}
}
