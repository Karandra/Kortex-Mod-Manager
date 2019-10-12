#include "stdafx.h"
#include "IThemeManager.h"
#include "Application/SystemApplication.h"
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxSystemSettings.h>

namespace
{
	enum class Win8OrGreater
	{
		Unknown = -1,
		True = 1,
		False = 0
	};
	Win8OrGreater g_Win8OrGreater = Win8OrGreater::Unknown;
}

namespace Kortex
{
	IThemeManager& IThemeManager::GetActive()
	{
		return SystemApplication::GetInstance()->GetThemeManager();
	}

	void IThemeManager::InheritColors(wxWindow* window, const wxWindow* from) const
	{
		window->SetBackgroundColour(from->GetBackgroundColour());
		window->SetForegroundColour(from->GetForegroundColour());
	}
	bool IThemeManager::IsWin8OrGreater() const
	{
		if (g_Win8OrGreater == Win8OrGreater::Unknown)
		{
			g_Win8OrGreater = KxSystem::IsWindows8OrGreater() ? Win8OrGreater::True : Win8OrGreater::False;
		}
		return g_Win8OrGreater == Win8OrGreater::True;
	}
}
