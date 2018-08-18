#pragma once
#include "stdafx.h"
#include "ConfigManager/KCMController.h"
class KConfigManager;
class KGameConfigWorkspace;
class KVFSEvent;
class KxTreeList;

class KGameConfigWorkspaceController: public KCMController
{
	private:
		virtual KxTreeListItem CreateUnknownItemsRoot() override;
		void OnSelectControllerView(wxTreeListEvent& event);
		void OnVFSToggled(KVFSEvent& event);

	public:
		KGameConfigWorkspaceController(KGameConfigWorkspace* workspace, KxTreeList* view);
		virtual ~KGameConfigWorkspaceController();
};
