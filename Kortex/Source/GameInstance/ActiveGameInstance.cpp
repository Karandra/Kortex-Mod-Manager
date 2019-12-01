#include "stdafx.h"
#include "ActiveGameInstance.h"
#include "IGameProfile.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/Common/GameData.hpp>
#include <Kortex/Common/GameConfig.hpp>

namespace Kortex::GameInstance
{
	void InstanceModuleLoader::LoadGlobalModule(IModule& module, const KxXMLDocument& instanceConfig)
	{
		LoadModule(module, instanceConfig.GetFirstChildElement("Definition").GetFirstChildElement(module.GetModuleInfo().GetID()));
	}
	void InstanceModuleLoader::LoadModule(IModule& module, const KxXMLNode& instanceNode)
	{
		module.OnLoadInstance(*m_Instance, instanceNode);
		IModule::ForEachManager(&module, [this, &instanceNode](IManager& manager)
		{
			manager.OnLoadInstance(*m_Instance, instanceNode.GetFirstChildElement(manager.GetManagerInfo().GetID()));
		});
	}
}

namespace Kortex::GameInstance
{
	void ActiveGameInstance::InitModulesConfig(const KxXMLDocument& instanceConfig)
	{
		// Load global modules
		IModule::ForEachModule([this, &instanceConfig](IModule& module)
		{
			if (module.GetModuleDisposition() == IModule::Disposition::Global)
			{
				LoadGlobalModule(module, instanceConfig);
			}
		});

		// Load instance modules
		m_GameDataModule = InitModule<GameDataModule>(instanceConfig);
		m_GameConfigModule = InitModule<GameConfigModule>(instanceConfig);
	}
	void ActiveGameInstance::InitVariables(const IGameProfile& profile)
	{
		IVariableTable& variables = GetVariables();

		variables.SetVariable(Variables::KVAR_CONFIG_DIR, profile.GetConfigDir());
		variables.SetVariable(Variables::KVAR_SAVES_DIR, profile.GetSavesDir());
		variables.SetVariable(Variables::KVAR_OVERWRITES_DIR, profile.GetOverwritesDir());

		variables.SetVariable(Variables::KVAR_PROFILE_ID, profile.GetID());
		variables.SetVariable(Variables::KVAR_PROFILE_DIR, profile.GetProfileDir());
	}

	bool ActiveGameInstance::OnLoadInstance(const KxXMLDocument& instanceConfig)
	{
		// Lock instance folder
		m_DirectoryLock.Open(GetInstanceDir(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read|KxFileStream::Share::Write, KxFileStream::Flags::BackupSemantics);

		if (ConfigurableGameInstance::OnLoadInstance(instanceConfig))
		{
			IGameProfile::UpdateVariablesUsingActive(GetVariables());
			InitModulesConfig(instanceConfig);
			return true;
		}
		return false;
	}
	bool ActiveGameInstance::ShouldInitProfiles() const
	{
		return true;
	}

	ActiveGameInstance::ActiveGameInstance(const IGameInstance& instanceTemplate, const wxString& instanceID)
		:InstanceModuleLoader(this), ConfigurableGameInstance(instanceTemplate, instanceID)
	{
	}

	const wxString& ActiveGameInstance::GetActiveProfileID() const
	{
		return m_CurrentProfileID;
	}
	void ActiveGameInstance::SetCurrentProfileID(const wxString& id)
	{
		using namespace Application;

		m_CurrentProfileID = id;
		GetInstanceOption(OName::Profile).SetAttribute(OName::ID, id);
	}

	void ActiveGameInstance::DoChangeProfileTo(IGameProfile& profile)
	{
		KxFile(profile.GetConfigDir()).CreateFolder();
		KxFile(profile.GetSavesDir()).CreateFolder();
		KxFile(profile.GetOverwritesDir()).CreateFolder();

		IGameProfile* oldProfile = GetActiveProfile();

		profile.LoadConfig();
		SetCurrentProfileID(profile.GetID());
		InitVariables(profile);

		// Perform all required updates here
		if (IModManager* modManager = IModManager::GetInstance())
		{
			IGameMod& overwrites = modManager->GetWriteTarget();
			overwrites.LinkLocation(profile.GetOverwritesDir());
			overwrites.UpdateFileTree();

			for (IGameMod* mod: modManager->GetMandatoryMods())
			{
				mod->UpdateFileTree();
			}
		}

		// Finally send event
		BroadcastProcessor::Get().ProcessEvent(ProfileEvent::EvtSelected, profile, oldProfile);
	}
	void ActiveGameInstance::LoadSavedProfileOrDefault()
	{
		// Load saved profile or create a default one in none is exist yet
		IGameProfile* profile = GetProfile(GetInstanceOption("Profile").GetAttribute("ID"));
		if (profile)
		{
			ChangeProfileTo(*profile);
		}
		else if (!HasProfiles())
		{
			profile = CreateProfile(CreateDefaultProfileID());
			if (profile)
			{
				ChangeProfileTo(*profile);
			}
		}
		else
		{
			ChangeProfileTo(*GetProfiles().front());
		}
	}
}
