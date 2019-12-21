#pragma once
#include "stdafx.h"
#include "IWorkspaceContainer.h"

namespace Kortex::Application
{
	class BookWorkspaceContainer: public KxRTTI::ExtendInterface<BookWorkspaceContainer, IWorkspaceContainer>
	{
		KxDecalreIID(BookWorkspaceContainer, {0x1b6cdd23, 0xff47, 0x4cf9, {0x87, 0x5a, 0x21, 0x15, 0xde, 0x1, 0x2b, 0x5b}});

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
