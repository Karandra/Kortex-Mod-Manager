#include "stdafx.h"
#include "KSaveManager.h"
#include "Profile/KSaveManagerConfig.h"
#include "ModManager/KModListManager.h"
#include "UI/KWorkspace.h"
#include "Profile/KProfile.h"
#include "Events/KModListEventInternal.h"
#include "KSaveManagerWorkspace.h"
#include <KxFramework/KxFile.h>

KWorkspace* KSaveManager::CreateWorkspace(KMainWindow* mainWindow)
{
	return new KSaveManagerWorkspace(mainWindow, this);
}

void KSaveManager::OnSavesLocationChanged(KModListEvent& event)
{
	if (event.HasModList() && event.GetModList()->IsCurrentList())
	{
		KWorkspace::ScheduleReloadOf<KSaveManagerWorkspace>();
	}
}

KSaveManager::KSaveManager(const KxXMLNode& configNode, const KSaveManagerConfig* profileSaveManager)
{
	KEvent::Bind(KEVT_MODLIST_INT_SELECTED, &KSaveManager::OnSavesLocationChanged, this);
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
	return KxFile(KVAR_EXP(KVAR_SAVES_ROOT_LOCAL) + wxS('\\') + KSaveManagerConfig::GetInstance()->GetRelativeLocation()).GetFullPath();
}
