#pragma once
#include "Framework.hpp"
#include "Options/Option.h"

namespace Kortex
{
	class IManager;
	class IApplication;
	class IGameInstance;

	namespace GameInstance
	{
		class InstanceModuleLoader;
	}
}

namespace Kortex
{
	class IModule: public kxf::RTTI::Interface<IModule>, public Application::WithOptions<IModule>
	{
		KxRTTI_DeclareIID(IModule, {0x6406fcf7, 0xbb23, 0x4f35, {0xbd, 0xf1, 0xdc, 0x19, 0x65, 0x84, 0x85, 0x33}});

		friend class IManager;
		friend class IApplication;
		friend class GameInstance::InstanceModuleLoader;

		public:
			enum class Disposition
			{
				Local,
				Global,
				ActiveInstance,
			};

		protected:
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
			IModule() noexcept = default;
			virtual ~IModule();

		public:
			virtual kxf::String GetID() const = 0;
			virtual kxf::String GetName() const = 0;
			virtual kxf::Version GetVersion() const = 0;
			virtual kxf::ResourceID GetIcon() const = 0;

			virtual Disposition GetDisposition() const = 0;
			virtual size_t EnumManagers(std::function<bool(IManager&)> func) = 0;
	};
}
