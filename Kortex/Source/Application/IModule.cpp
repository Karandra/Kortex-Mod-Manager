#include "stdafx.h"
#include "IModule.h"
#include "IManager.h"
#include <Kortex/Utility.hpp>

namespace
{
	using namespace Kortex;

	IModule::RefList ms_Instances;

	bool CanUseInstanceList(const IModule* module)
	{
		return module->GetModuleDisposition() != IModule::Disposition::Local;
	}
}

namespace Kortex
{
	const IModule::RefList& IModule::GetInstances()
	{
		return ms_Instances;
	}

	void IModule::InitModulesWithDisposition(Disposition disposition)
	{
		ForEachModuleAndManager([disposition](IModule& module, IManager* manager = nullptr)
		{
			if (module.GetModuleDisposition() == disposition)
			{
				if (manager)
				{
					Utility::Log::LogInfo("Initializing manager: %1::%2", module.GetModuleInfo().GetID(), manager->GetManagerInfo().GetID());
					manager->OnInit();
				}
				else
				{
					Utility::Log::LogInfo("Initializing module: %1", module.GetModuleInfo().GetID());
					module.OnInit();
				}
			}
		});
	}
	void IModule::UninitModulesWithDisposition(Disposition disposition)
	{
		Utility::Log::LogInfo("Uninitializing modules.");

		IModule::ForEachModuleAndManager([disposition](IModule& module, IManager* manager = nullptr)
		{
			if (module.GetModuleDisposition() == disposition)
			{
				if (manager)
				{
					Utility::Log::LogInfo("Uninitializing manager: %1::%1", module.GetModuleInfo().GetID(), manager->GetManagerInfo().GetID());
					manager->OnExit();
				}
				else
				{
					Utility::Log::LogInfo("Uninitializing module: %1", module.GetModuleInfo().GetID());
					module.OnExit();
				}
			}
		});
	}

	IModule::IModule(Disposition type)
		:m_Disposition(type)
	{
		if (CanUseInstanceList(this))
		{
			ms_Instances.push_back(this);
		}
	}
	IModule::~IModule()
	{
		if (CanUseInstanceList(this))
		{
			ms_Instances.remove(this);
		}
	}
}
