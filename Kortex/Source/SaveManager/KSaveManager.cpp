#include "stdafx.h"
#include "KSaveManager.h"
#include "GameInstance/Config/KSaveManagerConfig.h"
#include "GameInstance/KGameInstance.h"
#include "Profile/KProfile.h"
#include "UI/KWorkspace.h"
#include "GameInstance/KGameInstance.h"
#include "KSaveManagerWorkspace.h"
#include <KxFramework/KxFile.h>

KWorkspace* KSaveManager::CreateWorkspace(KMainWindow* mainWindow)
{
	return new KSaveManagerWorkspace(mainWindow, this);
}

void KSaveManager::OnSavesLocationChanged(KProfileEvent& event)
{
	KWorkspace::ScheduleReloadOf<KSaveManagerWorkspace>();
}

KSaveManager::KSaveManager(const KxXMLNode& configNode, const KSaveManagerConfig* profileSaveManager)
{
	KEvent::Bind(KEVT_PROFILE_CHANGED, &KSaveManager::OnSavesLocationChanged, this);
	KEvent::Bind(KEVT_PROFILE_SELECTED, &KSaveManager::OnSavesLocationChanged, this);
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

wxString KSaveManager::GetSavesLocation() const
{
	return KSaveManagerConfig::GetInstance()->GetLocation();
}
