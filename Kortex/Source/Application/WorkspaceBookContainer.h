#pragma once
#include "stdafx.h"
#include "IWorkspaceContainer.h"

namespace Kortex::Application
{
	class WorkspaceBookContainer: public KxRTTI::ExtendInterface<WorkspaceBookContainer, IWorkspaceContainer>
	{
		private:
			bool m_HasCurrentWorkspace = false;

		protected:
			bool RunSwitchSequence(IWorkspace* fromWorkspace, IWorkspace& toWorkspace);
			void ShowWorkspace(IWorkspace& workspace) override;
			void HideWorkspace(IWorkspace& workspace) override;

		public:
			wxBookCtrlBase& GetBookCtrl()
			{
				return static_cast<wxBookCtrlBase&>(GetWindow());
			}
			const wxBookCtrlBase& GetBookCtrl() const
			{
				return static_cast<const wxBookCtrlBase&>(GetWindow());
			}

			IWorkspace::RefVector EnumWorkspaces() const override;
			IWorkspace* GetWorkspaceByID(const wxString& id) const override;
			IWorkspace* GetCurrentWorkspace() const override;
			size_t GetWorkspaceCount() const override;

			bool AddWorkspace(IWorkspace& workspace) override;
			bool RemoveWorkspace(IWorkspace& workspace) override;
			bool SwitchWorkspace(IWorkspace& nextWorkspace) override;

			IWorkspace* GetWorkspaceByIndex(size_t index) const;
			std::optional<size_t> GetWorkspaceIndex(const IWorkspace& workspace) const;
	};
}
