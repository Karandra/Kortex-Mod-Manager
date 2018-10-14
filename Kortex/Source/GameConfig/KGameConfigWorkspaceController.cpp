#include "stdafx.h"
#include "KGameConfigWorkspaceController.h"
#include "UI/KWorkspaceController.h"
#include "KGameConfigWorkspace.h"
#include "ConfigManager/KConfigManager.h"
#include "ConfigManager/KCMController.h"
#include "GameInstance/KGameInstance.h"
#include "KApp.h"

KxTreeListItem KGameConfigWorkspaceController::CreateUnknownItemsRoot()
{
	return GetTreeRoot().Add(T("ConfigManager.Categories.None"));
}
void KGameConfigWorkspaceController::OnSelectControllerView(wxTreeListEvent& event)
{
	KxTreeListItem item(*GetView(), event.GetItem());
	if (item.IsOK())
	{
		KCMConfigEntryRef* entryRef = GetEntryFromItem(item);
		KCMConfigEntryBase* entry = entryRef ? entryRef->GetEntry() : NULL;
		if (entry)
		{
			GetMainWindow()->SetStatus(GetStatusBarString(entry));
			return;
		}
	}
	GetMainWindow()->ClearStatus();
}
void KGameConfigWorkspaceController::OnVFSToggled(KVFSEvent& event)
{
	Reload();
}

KGameConfigWorkspaceController::KGameConfigWorkspaceController(KGameConfigWorkspace* workspace, KxTreeList* view)
	:KCMController(workspace, new KConfigManager(workspace, KGameInstance::GetActive()->GetGameID()), view)
{
	if (GetConfigManager()->GetGameConfig()->IsENBEnabled())
	{
		GetConfigManager()->AddENB();
	}
	view->Bind(KxEVT_TREELIST_SELECTION_CHANGED, &KGameConfigWorkspaceController::OnSelectControllerView, this);

	KEvent::Bind(KEVT_VFS_TOGGLED, &KGameConfigWorkspaceController::OnVFSToggled, this);
}
KGameConfigWorkspaceController::~KGameConfigWorkspaceController()
{
	delete GetConfigManager();
}
