#pragma once
#include "Framework.hpp"
#include "FrameworkUI.hpp"
#include "IWorkspace.h"
#include "IWorkspaceContainer.h"

namespace Kortex::Application
{
	class DefaultWorkspace: public kxf::RTTI::Implementation<DefaultWorkspace, IWorkspace>
	{
		private:
			IWorkspaceContainer* m_Container = nullptr;

			size_t m_OpenCount = 0;
			bool m_IsCreated = false;
			bool m_InsideOnCreate = false;
			bool m_ReloadSheduled = false;

		protected:
			// DefaultWorkspace
			void ApplyWorkspaceTheme();

			// IWorkspace
			bool DoOnCreateWorkspace() override;
			bool DoOnOpenWorkspace() override;
			bool DoOnCloseWorkspace() override;

			void SetCurrentContainer(IWorkspaceContainer* contianer) override
			{
				m_Container = contianer;
			}

		public:
			// IWorkspace
			IWorkspaceContainer* GetCurrentContainer() const override
			{
				return m_Container;
			}

			bool IsCreated() const override
			{
				return m_IsCreated;
			}
			bool OpenedOnce() const override
			{
				return m_OpenCount != 0;
			}

			bool Reload() override;
			void ScheduleReload() override;
			bool IsReloadScheduled() const override
			{
				return m_ReloadSheduled;
			}
			bool EnsureCreated() override;
	};

	template<class TWindow>
	class DefaultWindowWorkspace: public TWindow, public DefaultWorkspace
	{
		protected:
			// IWorkspace
			void CreateWorkspaceWindow(wxWindow& parent) override
			{
				if (TWindow::Create(&parent, wxID_NONE))
				{
					ApplyWorkspaceTheme();
				}
			}

		public:
			template<class... Args>
			DefaultWindowWorkspace(Args&&... arg)
				:TWindow(std::forward<Args>(arg)...)
			{
				static_assert(std::is_base_of_v<wxWindow, TWindow>, "TWindow is not a wxWindow");
			}

		public:
			// IWorkspace
			wxWindow& GetWindow() override
			{
				return static_cast<TWindow&>(*this);
			}
	};
}
