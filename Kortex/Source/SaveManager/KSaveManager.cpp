#include "stdafx.h"
#include "KSaveManager.h"
#include "Profile/KSaveManagerConfig.h"
#include "UI/KWorkspace.h"
#include "KSaveManagerWorkspace.h"

KxSingletonPtr_Define(KSaveManager);

KWorkspace* KSaveManager::CreateWorkspace(KMainWindow* mainWindow)
{
	m_Workspace = new KSaveManagerWorkspace(mainWindow, this);
	return m_Workspace;
}

KSaveManager::KSaveManager(const KxXMLNode& configNode, const KSaveManagerConfig* profileSaveManager)
	:m_ProfileSaveManager(profileSaveManager), m_RequiresVFS(configNode.GetAttributeBool("RequiresVFS", true))
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
