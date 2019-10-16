#pragma once
#include "stdafx.h"
#include "IWorkspaceContainer.h"

namespace Kortex::Application
{
	class BookWorkspaceContainer: public KxRTTI::ExtendInterface<BookWorkspaceContainer, IWorkspaceContainer>
	{
		private:
			bool m_HasCurrentWorkspace = false;

		private:
			bool DoInsertWorkspacePage(IWorkspace& workspace, size_t index);

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
			IWorkspace* GetWorkspaceByIndex(size_t index) const override;
			IWorkspace* GetCurrentWorkspace() const override;
			size_t GetWorkspaceCount() const override;
			std::optional<size_t> GetWorkspaceIndex(const IWorkspace& workspace) const override;
			bool ChangeWorkspaceIndex(IWorkspace& workspace, size_t newIndex) override;

			bool AddWorkspace(IWorkspace& workspace) override;
			bool RemoveWorkspace(IWorkspace& workspace) override;
			bool SwitchWorkspace(IWorkspace& nextWorkspace) override;

	};
}
