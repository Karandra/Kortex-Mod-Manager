#pragma once
#include "stdafx.h"
#include "IWorkspace.h"
#include "IWorkspaceContainer.h"

namespace Kortex::Application
{
	class DefaultWorkspace: public IWorkspace
	{
		private:
			IWorkspaceContainer* m_Container = nullptr;

			size_t m_OpenCount = 0;
			bool m_IsCreated = false;
			bool m_InsideOnCreate = false;
			bool m_ReloadSheduled = false;

		protected:
			bool CallOnCreateWorkspace() override;
			bool CallOnOpenWorkspace() override;
			bool CallOnCloseWorkspace() override;

			void SetWorkspaceContainer(IWorkspaceContainer* contianer) override
			{
				m_Container = contianer;
			}
			virtual void CreateWindow() = 0;

		public:
			bool HasWorkspaceContainer() const override
			{
				return m_Container != nullptr;
			}
			IWorkspaceContainer& GetWorkspaceContainer() override
			{
				return *m_Container;
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
			void CreateWindow() override
			{
				TWindow::Create(&GetWorkspaceContainer().GetWindow(), KxID_NONE);
			}

		public:
			template<class... Args> DefaultWindowWorkspace(Args&&... arg)
				:TWindow(std::forward<Args>(arg)...)
			{
				static_assert(std::is_base_of_v<wxWindow, TWindow>, "TWindow is not a wxWindow");
			}

		public:
			wxWindow& GetWindow() override
			{
				return static_cast<TWindow&>(*this);
			}
	};
}
