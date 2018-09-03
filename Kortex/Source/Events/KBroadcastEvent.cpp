#include "stdafx.h"
#include "KBroadcastEvent.h"
#include "UI/KWorkspace.h"

bool KBroadcastEvent::IsTargetWorkspace(const KWorkspace* workspace) const
{
	return workspace->GetID() == m_TargetWorkspaceID;
}
