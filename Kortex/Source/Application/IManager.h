#pragma once
#include "stdafx.h"
#include "Application/Module/ManagerInfo.h"
#include "Application/Options/Option.h"
class KWorkspace;

namespace Kortex
{
	class IModule;
	class IApplication;
	class IGameInstance;
	class IPluggableManager;

	namespace GameInstance
	{
		class InstanceModuleLoader;
	}

	class IManager: public Application::WithOptions<IManager>
	{
		friend class IModule;
		friend class IApplication;
		friend class GameInstance::InstanceModuleLoader;

		public:
			using RefList = std::list<IManager*>;

		private:
			IModule& m_Module;

		protected:
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) = 0;
			virtual void OnInit() = 0;
			virtual void OnExit() = 0;

		public:
			IManager(IModule* module)
				:m_Module(*module)
			{
			}
			virtual ~IManager() = default;

		public:
			virtual const IManagerInfo& GetManagerInfo() const = 0;
			const IModule& GetModule() const
			{
				return m_Module;
			}
			IModule& GetModule()
			{
				return m_Module;
			}

			virtual KWorkspace* GetWorkspace() const
			{
				return nullptr;
			}
			bool HasWorkspace() const
			{
				return GetWorkspace() != nullptr;
			}
			void ScheduleReloadWorkspace() const;

			virtual const Kortex::IPluggableManager* ToPluggableManager() const
			{
				return nullptr;
			}
			virtual Kortex::IPluggableManager* ToPluggableManager()
			{
				return nullptr;
			}
	};
}

namespace Kortex
{
	template<class t_Base, const auto& t_TypeInfo> class ManagerWithTypeInfo: public t_Base
	{
		public:
			static const IManagerInfo& GetManagerTypeInfo()
			{
				return t_TypeInfo;
			}

		protected:
			ManagerWithTypeInfo(IModule* module)
				:t_Base(module)
			{
			}

		public:
			const IManagerInfo& GetManagerInfo() const override
			{
				return t_TypeInfo;
			}
	};
}
