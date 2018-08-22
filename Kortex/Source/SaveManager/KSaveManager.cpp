#include "stdafx.h"
#include "KSaveManager.h"
#include "Profile/KSaveManagerConfig.h"
#include "UI/KWorkspace.h"
#include "KSaveManagerWorkspace.h"

KxSingletonPtr_Define(KSaveManager);

KWorkspace* KSaveManager::CreateWorkspace(KMainWindow* mainWindow)
{
	return new KSaveManagerWorkspace(mainWindow, this);
}

KSaveManager::KSaveManager(const KxXMLNode& configNode, const KSaveManagerConfig* profileSaveManager)
{
}
KSaveManager::~KSaveManager()
{
}

wxString KSaveManager::GetID() const
{
	return "KSaveManager";
}
wxString KSaveManager::GetName() const
{
	return T("ToolBar.SaveManager");
}
wxString KSaveManager::GetVersion() const
{
	return "1.0.2";
}

KWorkspace* KSaveManager::GetWorkspace() const
{
	return KSaveManagerWorkspace::GetInstance();
}

bool KSaveManager::Save()
{
	return false;
}
bool KSaveManager::Load()
{
	return false;
}
bool KSaveManager::Reload()
{
	return false;
}
