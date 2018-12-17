#include "stdafx.h"
#include "IModule.h"

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
