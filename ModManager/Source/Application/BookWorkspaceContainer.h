#pragma once
#include "Framework.hpp"
#include "IWorkspaceContainer.h"
#include <wx/bookctrl.h>

namespace Kortex::Application
{
	class KORTEX_API BookWorkspaceContainer: public kxf::RTTI::ExtendInterface<BookWorkspaceContainer, IWorkspaceContainer>
	{
		KxRTTI_DeclareIID(BookWorkspaceContainer, {0x1b6cdd23, 0xff47, 0x4cf9, {0x87, 0x5a, 0x21, 0x15, 0xde, 0x1, 0x2b, 0x5b}});

		private:
			bool m_HasCurrentWorkspace = false;

		private:
			// BookWorkspaceContainer
			bool DoInsertWorkspacePage(IWorkspace& workspace, size_t index);

		protected:
			bool DoSwitchWorkspace(IWorkspace* fromWorkspace, IWorkspace& toWorkspace);

		public:
			// BookWorkspaceContainer
			wxBookCtrlBase& GetBookCtrl()
			{
				return static_cast<wxBookCtrlBase&>(GetWindow());
			}
			const wxBookCtrlBase& GetBookCtrl() const
			{
				return static_cast<const wxBookCtrlBase&>(GetWindow());
			}

			// IWorkspaceContainer
			IWorkspaceContainer* GetParentContainer()
			{
				return dynamic_cast<IWorkspaceContainer*>(GetWindow().GetParent());
			}
			const IWorkspaceContainer* GetParentContainer() const
			{
				return dynamic_cast<const IWorkspaceContainer*>(GetWindow().GetParent());
			}

			IWorkspace* GetWorkspaceByID(const kxf::String& id) const override;
			IWorkspace* GetWorkspaceByIndex(size_t index) const override;
			IWorkspace* GetCurrentWorkspace() const override;

			size_t GetWorkspacesCount() const override;
			size_t EnumWorkspaces(std::function<bool(IWorkspace&)> func) override;
			std::optional<size_t> GetWorkspaceIndex(const IWorkspace& workspace) const override;
			bool ChangeWorkspaceIndex(IWorkspace& workspace, size_t newIndex) override;

			bool AttachWorkspace(IWorkspace& workspace) override;
			bool DetachWorkspace(IWorkspace& workspace) override;

			void ShowWorkspace(IWorkspace& workspace) override;
			void HideWorkspace(IWorkspace& workspace) override;
			bool SwitchWorkspace(IWorkspace& nextWorkspace) override;
	};
}
