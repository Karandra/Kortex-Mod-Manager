#pragma once
#include "Framework.hpp"
#include "IWorkspace.h"
#include <kxf/UI/IWidget.h>

namespace Kortex
{
	class KORTEX_API IWorkspaceContainer: public kxf::RTTI::Interface<IWorkspaceContainer>
	{
		KxRTTI_DeclareIID(IWorkspaceContainer, {0xef898b45, 0x2a8a, 0x4290, {0xb3, 0x7b, 0xc5, 0xfb, 0xff, 0x34, 0x28, 0xb6}});

		friend class IWorkspace;

		protected:
			bool CallOnCreate(IWorkspace& workspace)
			{
				return workspace.DoOnCreateWorkspace();
			}
			bool CallOnOpen(IWorkspace& workspace)
			{
				return workspace.DoOnOpenWorkspace();
			}
			bool CallOnClose(IWorkspace& workspace)
			{
				return workspace.DoOnCloseWorkspace();
			}

		public:
			virtual ~IWorkspaceContainer() = default;

		public:
			virtual kxf::IWidget& GetWidget() const = 0;

			virtual IWorkspaceContainer* GetParentContainer()
			{
				return nullptr;
			}
			virtual const IWorkspaceContainer* GetParentContainer() const
			{
				return nullptr;
			}
			bool IsSubContainer() const
			{
				return GetParentContainer() != nullptr;
			}
			bool ContainsWorkspace(const IWorkspace& workspace) const;

			virtual IWorkspace* GetWorkspaceByID(const kxf::String& id) const = 0;
			virtual IWorkspace* GetWorkspaceByIndex(size_t index) const = 0;
			virtual IWorkspace* GetCurrentWorkspace() const = 0;

			virtual size_t GetWorkspacesCount() const = 0;
			virtual size_t EnumWorkspaces(std::function<bool(IWorkspace&)> func) = 0;
			virtual std::optional<size_t> GetWorkspaceIndex(const IWorkspace& workspace) const = 0;
			virtual bool ChangeWorkspaceIndex(IWorkspace& workspace, size_t newIndex) = 0;

			virtual bool AttachWorkspace(IWorkspace& workspace) = 0;
			virtual bool DetachWorkspace(IWorkspace& workspace) = 0;

			virtual void HideWorkspace(IWorkspace& workspace) = 0;
			virtual void ShowWorkspace(IWorkspace& workspace) = 0;
			virtual bool SwitchWorkspace(IWorkspace& nextWorkspace) = 0;
			bool SwitchWorkspaceByID(const kxf::String& id);
	};
}
