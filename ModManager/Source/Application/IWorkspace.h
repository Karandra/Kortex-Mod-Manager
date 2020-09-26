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

			template<class T>
			static void ScheduleReloadOf()
			{
				if (T* workspace = T::GetInstance())
				{
					workspace->ScheduleReload();
				}
			}

			template<class TWorkspace, class TFunc>
			static bool CallIfCreated(TFunc&& func)
			{
				TWorkspace* workspace = TWorkspace::GetInstance();
				if (workspace && workspace->IsCreated())
				{
					std::invoke(func, static_cast<TWorkspace&>(*workspace));
					return true;
				}
				return false;
			}

		protected:
			virtual bool CallOnCreateWorkspace() = 0;
			virtual bool CallOnOpenWorkspace() = 0;
			virtual bool CallOnCloseWorkspace() = 0;

			virtual void CreateWorkspaceWindow(wxWindow& parent) = 0;
			virtual void SetCurrentContainer(IWorkspaceContainer* contianer) = 0;

			void ShowWorkspace();
			void HideWorkspace();

		public:
			virtual ~IWorkspace() = default;

		public:
			bool EvtOnCreateWorkspace()
			{
				return QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IWorkspace::EvtOnCreateWorkspace);
			}
			void EvtOnReloadWorkspace()
			{
				QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IWorkspace::EvtOnReloadWorkspace);
			}
			bool EvtOnOpenWorkspace()
			{
				return QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IWorkspace::EvtOnOpenWorkspace);
			}
			bool EvtOnCloseWorkspace()
			{
				return QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IWorkspace::EvtOnCloseWorkspace);
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
