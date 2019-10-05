#pragma once
#include "stdafx.h"
#include "Application/IWorkspaceDocument.h"

namespace Kortex::GameConfig
{
	class WorkspaceController: public IWorkspaceDocument
	{
		private:
			IWorkspace& m_Workspace;

		public:
			WorkspaceController(IWorkspace& workspace)
				:m_Workspace(workspace)
			{
			}

		public:
			bool HasUnsavedChanges() const override;
			void SaveChanges() override;
			void DiscardChanges() override;
	};
}
