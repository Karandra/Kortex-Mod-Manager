#include "stdafx.h"
#include "KSettingsWindowController.h"
#include "KSettingsWindow.h"
#include "KSettingsWindowManager.h"
#include "ConfigManager/KConfigManager.h"
#include "KApp.h"

KxTreeListItem KSettingsWindowController::CreateUnknownItemsRoot()
{
	return GetTreeRoot().Add(KTr("ConfigManager.Categories.None"));
}

KSettingsWindowController::KSettingsWindowController(KSettingsWorkspace* workspace, KxTreeList* view)
	:KCMController(workspace, KApp::Get().GetSettingsManager(), view), m_Workspace(workspace)
{
	GetConfigManager()->SetWorkspace(m_Workspace);
}
KSettingsWindowController::~KSettingsWindowController()
{
	GetConfigManager()->SetWorkspace(NULL);
}

wxWindow* KSettingsWindowController::GetParentWindow()
{
	return m_Workspace->GetSettingsWindow();
}

KxStringVector KSettingsWindowController::OnFormatEntryToView(KCMConfigEntryPath* pathEntry)
{
	return KxStringVector({pathEntry->GetFullPath()});
}
KxStringVector KSettingsWindowController::OnFormatEntryToView(KCMConfigEntryStd* stdEntry)
{
	return KxStringVector({stdEntry->GetLabel(), stdEntry->GetDisplayData()});
}
