#include "stdafx.h"
#include "ProgramsModule.h"
#include <Kortex/ProgramManager.hpp>

namespace Kortex
{
	namespace Internal
	{
		const SimpleModuleInfo ProgramModuleTypeInfo("Programs", "ProgramsModule.Name", "2.0", KIMG_APPLICATION_RUN);
	}

	void KProgramModule::OnLoadInstance(IGameInstance& instance, const KxXMLNode& node)
	{
		m_ProgramManager = std::make_unique<ProgramManager::DefaultProgramManager>();
	}
	void KProgramModule::OnInit()
	{
	}
	void KProgramModule::OnExit()
	{
	}

	KProgramModule::KProgramModule()
		:ModuleWithTypeInfo(Disposition::Global)
	{
	}

	IModule::ManagerRefVector KProgramModule::GetManagers()
	{
		return ToManagersList(m_ProgramManager);
	}
}
