#include "stdafx.h"
#include "KSaveManager.h"
#include "Profile/KSaveManagerConfig.h"
#include "UI/KWorkspace.h"
#include "KSaveManagerWorkspace.h"

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
	return T("SaveManager.Name");
}
wxString KSaveManager::GetVersion() const
{
	return "1.0.2";
}

KWorkspace* KSaveManager::GetWorkspace() const
{
	return KSaveManagerWorkspace::GetInstance();
}

