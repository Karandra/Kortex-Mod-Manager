#pragma once
#include "Framework.hpp"
#include "Options/Option.h"

namespace Kortex
{
	class IGameInstance;
	class IWorkspace;
}

namespace Kortex
{
	class KORTEX_API IModule: public kxf::RTTI::Interface<IModule>, public Application::WithOptions<IModule>
	{
		KxRTTI_DeclareIID(IModule, {0x6406fcf7, 0xbb23, 0x4f35, {0xbd, 0xf1, 0xdc, 0x19, 0x65, 0x84, 0x85, 0x33}});

		public:
			virtual ~IModule() = default;

		public:
			void OnLoadInstance(IGameInstance& instance, const kxf::XMLNode& node)
			{
				QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IModule::OnLoadInstance, instance, node);
			}
			void OnInit()
			{
				QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IModule::OnInit);
			}
			void OnExit()
			{
				QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IModule::OnExit);
			}

		public:
			virtual kxf::String GetName() const = 0;
			virtual kxf::Version GetVersion() const = 0;
			virtual kxf::ResourceID GetIcon() const = 0;

			virtual kxf::Enumerator<IWorkspace&> EnumWorkspaces() const = 0;
			void ScheduleWorkspacesReload();
	};
}
