#include "stdafx.h"
#include "IThemeManager.h"
#include <Kortex/Application.hpp>
#include "Application/SystemApplication.h"
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxSystemSettings.h>

namespace Kortex
{
	IThemeManager& IThemeManager::GetActive()
	{
		return IApplication::GetSystemApp()->GetThemeManager();
	}

	void IThemeManager::InheritColors(wxWindow* window, const wxWindow* from)
	{
		window->SetBackgroundColour(from->GetBackgroundColour());
		window->SetForegroundColour(from->GetForegroundColour());
	}

	IThemeManager::IThemeManager()
	{
		m_Win8OrGreater = KxSystem::IsWindows8OrGreater();
	}
}
