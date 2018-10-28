#include "stdafx.h"
#include "KSaveManager.h"
#include "KSaveFile.h"
#include "KSaveFileGeneric.h"
#include "KSaveFileBethesdaMorrowind.h"
#include "KSaveFileBethesdaOblivion.h"
#include "KSaveFileBethesdaSkyrim.h"
#include "KSaveFileBethesdaSkyrimSE.h"
#include "KSaveFileBethesdaFallout3.h"
#include "KSaveFileBethesdaFalloutNV.h"
#include "KSaveFileBethesdaFallout4.h"
#include "GameInstance/KInstanceManagement.h"
#include "GameInstance/Config/KSaveManagerConfig.h"
#include "Profile/KProfile.h"
#include "UI/KWorkspace.h"
#include "GameInstance/KGameInstance.h"
#include "KSaveManagerWorkspace.h"
#include <KxFramework/KxFile.h>

namespace
{
	template<class T> bool TryCreateSaveObject(std::unique_ptr<KSaveFile>& ptr, const wxString& requestedInterface, const wxChar* name)
	{
		if (requestedInterface == name)
		{
			ptr = std::make_unique<T>();
			return true;
		}
		return false;
	}
}

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
	return KTr("SaveManager.Name");
}
wxString KSaveManager::GetVersion() const
{
	return "1.0.2";
}

KWorkspace* KSaveManager::GetWorkspace() const
{
	return KSaveManagerWorkspace::GetInstance();
}

std::unique_ptr<KSaveFile> KSaveManager::QuerySaveInterface() const
{
	const wxString requestedInterface = KSaveManagerConfig::GetInstance()->GetSaveInterface();
	std::unique_ptr<KSaveFile> object;

	TryCreateSaveObject<KSaveFileBethesdaMorrowind>(object, requestedInterface, wxS("BethesdaMorrowind")) ||
	TryCreateSaveObject<KSaveFileBethesdaOblivion>(object, requestedInterface, wxS("BethesdaOblivion")) ||
	TryCreateSaveObject<KSaveFileBethesdaSkyrim>(object, requestedInterface, wxS("BethesdaSkyrim")) ||
	TryCreateSaveObject<KSaveFileBethesdaSkyrimSE>(object, requestedInterface, wxS("BethesdaSkyrimSE")) ||

	TryCreateSaveObject<KSaveFileBethesdaFallout3>(object, requestedInterface, wxS("BethesdaFallout3")) ||
	TryCreateSaveObject<KSaveFileBethesdaFalloutNV>(object, requestedInterface, wxS("BethesdaFalloutNV")) ||
	TryCreateSaveObject<KSaveFileBethesdaFallout4>(object, requestedInterface, wxS("BethesdaFallout4")) ||
	TryCreateSaveObject<KSaveFileGeneric>(object, requestedInterface, wxS("Sacred2"));

	return object;
}
wxString KSaveManager::GetSavesLocation() const
{
	return KSaveManagerConfig::GetInstance()->GetLocation();
}
