#pragma once
#include "Framework.hpp"
#include "Options/Option.h"
#include "IWorkspaceContainer.h"
#include <kxf/UI/Controls/AUI/AuiToolBar.h>
#include <kxf/UI/Controls/StatusBarEx.h>
#include <kxf/UI/Windows/Frame.h>
#include <kxf/UI/Menus/Menu.h>

namespace Kortex
{
	class IWorkspace;
	class IWorkspaceContainer;

	class VirtualFSEvent;
}

namespace Kortex
{
	class IMainWindow: public kxf::RTTI::Interface<IMainWindow>, public Application::WithOptions<IMainWindow>
	{
		KxRTTI_DeclareIID(IMainWindow, {0x2efd7947, 0x5371, 0x4cf8, {0xbf, 0x5b, 0x94, 0x13, 0x1, 0x72, 0x8, 0x7c}});

		public:
			static kxf::Size GetDialogBestSize(const wxWindow& dialog);

		public:
			void CreateWorkspaces();

		public:
			virtual kxf::UI::Frame& GetFrame() = 0;
			const kxf::UI::Frame& GetFrame() const
			{
				return const_cast<IMainWindow&>(*this).GetFrame();
			}

			virtual kxf::UI::AuiToolBar& GetMainToolBar() = 0;
			virtual kxf::UI::AuiToolBar& GetQuickToolBar() = 0;
			virtual kxf::UI::StatusBarEx& GetStatusBar() = 0;
			virtual kxf::UI::Menu& GetWorkspacesMenu() = 0;

			virtual IWorkspaceContainer& GetWorkspaceContainer() = 0;
			const IWorkspaceContainer& GetWorkspaceContainer() const
			{
				return const_cast<IMainWindow&>(*this).GetWorkspaceContainer();
			}

			virtual void ClearStatus(int index = 0) = 0;
			virtual void SetStatus(const kxf::String& label, int index = 0, const kxf::ResourceID& image = {}) = 0;
			virtual void SetStatusProgress(int current) = 0;
			virtual void SetStatusProgress(int64_t current, int64_t total) = 0;

			virtual kxf::UI::AuiToolBarItem* AddToolBarItem(IWorkspace& workspace) = 0;
			virtual kxf::UI::MenuItem* AddToolBarMenuItem(IWorkspace& workspace) = 0;
	};
}
