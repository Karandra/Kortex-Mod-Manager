#include "stdafx.h"
#include "KActiveGameInstance.h"
#include "Profile/KProfile.h"
#include "ModManager/KModManager.h"
#include "SettingsWindow/KSettingsWindowManager.h"
#include "KEvents.h"
#include "KApp.h"
#include "Config/KConfigManagerConfig.h"
#include "Config/KVirtualizationConfig.h"
#include "Config/KPackageManagerConfig.h"
#include "Config/KProgramManagerConfig.h"
#include "Config/KPluginManagerConfig.h"
#include "Config/KScreenshotsGalleryConfig.h"
#include "Config/KSaveManagerConfig.h"
#include "Config/KLocationsManagerConfig.h"
#include "Config/KNetworkConfig.h"

namespace
{
	template<class T, bool isAlwaysEnabled = false> std::unique_ptr<T> InitModuleConfig(KActiveGameInstance* instance, const KxXMLDocument& instanceConfig, const wxString& name)
	{
		const KxXMLNode node = instanceConfig.QueryElement("Instance/" + name);
		if (isAlwaysEnabled || node.GetAttributeBool("Enabled", true))
		{
			return std::make_unique<T>(*instance, node);
		}
		return std::unique_ptr<T>();
	}
}

void KActiveGameInstance::InitModulesConfig(const KxXMLDocument& instanceConfig)
{
	// Always enabled
	m_GameConfig = InitModuleConfig<KConfigManagerConfig, true>(this, instanceConfig, "GameConfig");

	m_LocationsConfig = InitModuleConfig<KLocationsManagerConfig, true>(this, instanceConfig, "LocationsConfig");
	m_VirtualizationConfig = InitModuleConfig<KVirtualizationConfig, true>(this, instanceConfig, "Virtualization");
	m_PackageManagerConfig = InitModuleConfig<KPackageManagerConfig, true>(this, instanceConfig, "PackageManager");
	m_NetworkConfig = InitModuleConfig<KNetworkConfig, true>(this, instanceConfig, "Network");

	// Can be disabled
	m_ProgramConfig = InitModuleConfig<KProgramManagerConfig>(this, instanceConfig, "ProgramManager");
	m_PluginManagerConfig = InitModuleConfig<KPluginManagerConfig>(this, instanceConfig, "PluginManager");
	m_ScreenshotsGallery = InitModuleConfig<KScreenshotsGalleryConfig>(this, instanceConfig, "ScreenshotsGallery");
	m_SaveManagerConfig = InitModuleConfig<KSaveManagerConfig>(this, instanceConfig, "SaveManager");
}
void KActiveGameInstance::InitVariables(const KProfile& profile)
{
	KIVariablesTable& variables = GetVariables();

	variables.SetVariable(KVAR_CONFIG_DIR, profile.GetConfigDir());
	variables.SetVariable(KVAR_SAVES_DIR, profile.GetSavesDir());
	variables.SetVariable(KVAR_OVERWRITES_DIR, profile.GetOverwritesDir());

	variables.SetVariable(KVAR_PROFILE_ID, profile.GetID());
	variables.SetVariable(KVAR_PROFILE_DIR, profile.GetProfileDir());
}

bool KActiveGameInstance::OnLoadInstance(const KxXMLDocument& instanceConfig)
{
	// Lock instance folder
	m_DirectoryLock.Open(GetInstanceDir(), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ|KxFS_SHARE_WRITE, KxFS_FLAG_BACKUP_SEMANTICS);
	
	if (KConfigurableGameInstance::OnLoadInstance(instanceConfig))
	{
		KProfile::SetGlobalPaths(GetVariables());
		InitModulesConfig(instanceConfig);
		return true;
	}
	return false;
}
bool KActiveGameInstance::ShouldInitProfiles() const
{
	return true;
}

KActiveGameInstance::KActiveGameInstance(const KGameInstance& instanceTemplate, const wxString& instanceID)
	:KConfigurableGameInstance(instanceTemplate, instanceID)
{
}
KActiveGameInstance::~KActiveGameInstance()
{
}

const wxString& KActiveGameInstance::GetCurrentProfileID() const
{
	return m_CurrentProfileID;
}
void KActiveGameInstance::SetCurrentProfileID(const wxString& id)
{
	m_CurrentProfileID = id;
	GetConfig().SetValue("KGameInstance::General", "ProfileID", m_CurrentProfileID);
	SaveConfig();
}

void KActiveGameInstance::ChangeProfileTo(const KProfile& profile)
{
	KxFile(profile.GetConfigDir()).CreateFolder();
	KxFile(profile.GetSavesDir()).CreateFolder();
	KxFile(profile.GetOverwritesDir()).CreateFolder();

	SetCurrentProfileID(profile.GetID());
	InitVariables(profile);

	// Perform all required updates here
	if (KModManager* modManager = KModManager::GetInstance())
	{
		KModEntry* overwrites = modManager->GetModEntry_WriteTarget();
		overwrites->SetLinkedModLocation(profile.GetOverwritesDir());
		overwrites->UpdateFileTree();

		for (KModEntry& mod: modManager->GetModEntry_Mandatory())
		{
			mod.UpdateFileTree();
		}

		// This will invalidate virtual tree
		modManager->ResortMods(profile);
	}

	// Finally send event
	KProfileEvent(KEVT_PROFILE_SELECTED, const_cast<KProfile&>(profile)).Send();

	SaveConfig();
}
void KActiveGameInstance::LoadSavedProfileOrDefault()
{
	// Load saved profile or create a default one in none is exist yet
	const KProfile* profile = GetProfile(GetConfig().GetValue("KGameInstance::General", "ProfileID"));
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
