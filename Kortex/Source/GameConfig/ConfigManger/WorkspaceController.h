#pragma once
#include "stdafx.h"
#include "UI/KWorkspaceController.h"

namespace Kortex::GameConfig
{
	class WorkspaceController: public KWorkspaceController
	{
		public:
			WorkspaceController(KWorkspace* workspace)
				:KWorkspaceController(workspace)
			{
			}

		public:
			bool HasUnsavedChanges() const override;
			void SaveChanges() override;
			void DiscardChanges() override;
	};
}
