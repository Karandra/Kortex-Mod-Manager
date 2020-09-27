#pragma once
#include "Framework.hpp"
#include "Options/Option.h"
#include "BroadcastProcessor.h"
//#include "Resources/ImageResourceID.h"

namespace Kortex
{
	class IMainWindow;
	class IWorkspaceContainer;
}

namespace Kortex
{
	class IWorkspace: public kxf::RTTI::Interface<IWorkspace>, public Application::WithOptions<IWorkspace>
	{
		KxRTTI_DeclareIID(IWorkspace, {0x9c8ee9bb, 0xaad3, 0x45df, {0xb8, 0x9c, 0xd6, 0x7b, 0x48, 0x47, 0x39, 0xfb}});

		friend class IWorkspaceContainer;

		public:
			static IWorkspace* FromWindow(wxWindow& window);

		protected:
			virtual bool DoOnCreateWorkspace() = 0;
			virtual bool DoOnOpenWorkspace() = 0;
			virtual bool DoOnCloseWorkspace() = 0;

			virtual void CreateWorkspaceWindow(wxWindow& parent) = 0;
			virtual void SetCurrentContainer(IWorkspaceContainer* contianer) = 0;

			void ShowWorkspace();
			void HideWorkspace();

		public:
			virtual ~IWorkspace() = default;

		public:
			bool OnCreateWorkspace()
			{
				return QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IWorkspace::OnCreateWorkspace);
			}
			void OnReloadWorkspace()
			{
				QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IWorkspace::OnReloadWorkspace);
			}
			bool OnOpenWorkspace()
			{
				return QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IWorkspace::OnOpenWorkspace);
			}
			bool OnCloseWorkspace()
			{
				return QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IWorkspace::OnCloseWorkspace);
			}

		public:
			virtual IWorkspaceContainer* GetCurrentContainer() const = 0;
			virtual IWorkspaceContainer* GetPreferredContainer() const
			{
				return nullptr;
			}

			virtual wxWindow& GetWindow() = 0;
			const wxWindow& GetWindow() const
			{
				return const_cast<IWorkspace&>(*this).GetWindow();
			}

			virtual kxf::String GetID() const = 0;
			virtual kxf::String GetName() const = 0;
			virtual kxf::ResourceID GetIcon() const = 0;
			virtual bool IsCreated() const = 0;
			virtual bool OpenedOnce() const = 0;
			
			virtual bool Reload() = 0;
			virtual void ScheduleReload() = 0;
			virtual bool IsReloadScheduled() const = 0;
			virtual bool EnsureCreated() = 0;
			
			bool IsCurrent() const;
			bool IsActive() const;
			bool IsSubWorkspace() const;
			bool SwitchHere();
	};
}

namespace Kortex::Application
{
	class WorkspaceClientData final: public wxClientData
	{
		private:
			IWorkspace& m_Workspace;

		public:
			WorkspaceClientData(IWorkspace& workspace)
				:m_Workspace(workspace)
			{
			}

		public:
			IWorkspace& GetWorkspace() const
			{
				return m_Workspace;
			}
	};
}
