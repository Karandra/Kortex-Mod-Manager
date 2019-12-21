#pragma once
#include "stdafx.h"
#include "Options/Option.h"
#include "BroadcastProcessor.h"
#include "Resources/ImageResourceID.h"
#include <KxFramework/KxSingleton.h>
#include <Kx/RTTI.hpp>

namespace Kortex
{
	class IMainWindow;
	class IWorkspaceContainer;
}

namespace Kortex
{
	class IWorkspace: public KxRTTI::Interface<IWorkspace>, public Application::WithOptions<IWorkspace>
	{
		KxDecalreIID(IWorkspace, {0x9c8ee9bb, 0xaad3, 0x45df, {0xb8, 0x9c, 0xd6, 0x7b, 0x48, 0x47, 0x39, 0xfb}});

		friend class IWorkspaceContainer;
		friend class KxIObject;

		public:
			using RefVector = std::vector<IWorkspace*>;

		public:
			template<class T> static void ScheduleReloadOf()
			{
				static_assert(std::is_base_of_v<KxSingletonPtr<T>, T>, "T is not a singleton class");

				if (T* workspace = T::GetInstance())
				{
					workspace->ScheduleReload();
				}
			}
			static IWorkspace* FromWindow(const wxWindow* window);

			template<class TWorkspace, class TFunc>
			static bool CallIfCreated(TFunc&& func)
			{
				static_assert(std::is_base_of_v<KxSingletonPtr<TWorkspace>, TWorkspace>, "Must be a 'KxSingletonPtr'");

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
			virtual bool OnCreateWorkspace() = 0;
			virtual void OnReloadWorkspace()
			{
			}
			virtual bool OnOpenWorkspace()
			{
				return true;
			}
			virtual bool OnCloseWorkspace()
			{
				return true;
			}
			
			virtual void SetCurrentContainer(IWorkspaceContainer* contianer) = 0;
			void ShowWorkspace();
			void HideWorkspace();

		public:
			virtual ~IWorkspace() = default;

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

			virtual wxString GetID() const = 0;
			virtual wxString GetName() const = 0;
			virtual ResourceID GetIcon() const = 0;
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
