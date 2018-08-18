#include "stdafx.h"
#include "KThemeManager.h"
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxSystemSettings.h>

KThemeManager* KThemeManager::ms_Instance = NULL;

KThemeManager& KThemeManager::Get()
{
	return *ms_Instance;
}
void KThemeManager::Set(KThemeManager* theme)
{
	Cleanup();
	ms_Instance = theme;
}
void KThemeManager::Cleanup()
{
	delete ms_Instance;
}

//////////////////////////////////////////////////////////////////////////
void KThemeManager::InheritColors(wxWindow* window, const wxWindow* from)
{
	window->SetBackgroundColour(from->GetBackgroundColour());
	window->SetForegroundColour(from->GetForegroundColour());
}

KThemeManager::KThemeManager()
{
	m_Win8OrGreater = KxSystem::IsWindows8OrGreater();
}
KThemeManager::~KThemeManager()
{
}
