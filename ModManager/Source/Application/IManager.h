#pragma once
#include "Framework.hpp"
#include "Options/Option.h"

namespace Kortex
{
	class IModule;
	class IWorkspace;
	class IMainWindow;
	class IApplication;
	class IGameInstance;

	namespace GameInstance
	{
		class InstanceModuleLoader;
	}
}

namespace Kortex
{
	class IManager: public kxf::RTTI::Interface<IManager>, public Application::WithOptions<IManager>
	{
		KxRTTI_DeclareIID(IManager, {0x80d27f02, 0x98d6, 0x47cb, {0x84, 0xc5, 0x62, 0xd3, 0x7f, 0xa7, 0x91, 0xbf}});

		friend class IModule;
		friend class IMainWindow;
		friend class IApplication;
		friend class GameInstance::InstanceModuleLoader;

		protected:
			void OnLoadInstance(IGameInstance& instance, const kxf::XMLNode& node)
			{
				QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IManager::OnLoadInstance, instance, node);
			}
			void OnInit()
			{
				QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IManager::OnInit);
			}
			void OnExit()
			{
				QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IManager::OnExit);
			}

			size_t OnCreateWorkspaces()
			{
				return QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IManager::OnCreateWorkspaces);
			}

		public:
			IManager() noexcept = default;
			virtual ~IManager() = default;

		public:
			virtual kxf::String GetID() const = 0;
			virtual kxf::String GetName() const = 0;

			virtual IModule& GetModule() = 0;
			virtual size_t EnumWorkspaces(std::function<bool(IWorkspace&)> func) const = 0;
			void ScheduleWorkspacesReload();
	};
}
