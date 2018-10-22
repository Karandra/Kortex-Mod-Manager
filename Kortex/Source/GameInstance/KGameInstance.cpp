#include "stdafx.h"
#include "KGameInstance.h"
#include "KActiveGameInstance.h"
#include "KVariablesDatabase.h"
#include "Profile/KProfile.h"
#include "ModManager/KDispatcher.h"
#include "Config/KProgramManagerConfig.h"
#include "KOperationWithProgress.h"
#include "KBitmapSize.h"
#include "KEvents.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxINI.h>
#include <KxFramework/KxXML.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxRegistry.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxFileOperationEvent.h>
#include <KxFramework/KxDualProgressDialog.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxComparator.h>

namespace
{
	std::unique_ptr<KActiveGameInstance> ms_ActiveInstance;
	KGameInstance::Vector ms_InstanceTemplates;
	//////////////////////////////////////////////////////////////////////////

	template <class T> struct is_unique_ptr: std::false_type {};
	template <class T> struct is_unique_ptr<std::unique_ptr<T>>: std::true_type {};
	
	enum class FindBy
	{
		GameID,
		InstanceID,
		ProfileID
	};
	template<class ObjectT, FindBy findBy, class VectorT> ObjectT* FindObjectInVector(VectorT& storageVector, const wxString& value, typename VectorT::const_iterator* itOut = NULL)
	{
		auto it = std::find_if(storageVector.begin(), storageVector.end(), [&value](const auto& object)
		{
			if constexpr(findBy == FindBy::GameID)
			{
				return KxComparator::IsEqual(object->GetGameID(), value, true);
			}
			else if constexpr(findBy == FindBy::InstanceID)
			{
				return KxComparator::IsEqual(object->GetInstanceID(), value, true);
			}
			else if constexpr(findBy == FindBy::ProfileID)
			{
				return KxComparator::IsEqual(object->GetID(), value, true);
			}
			else
			{
				static_assert(false, "unsupported find request");
			}
		});

		if (it != storageVector.end())
		{
			if (itOut)
			{
				*itOut = it;
			}
			return it->get();
		}
		return NULL;
	}
	template<class ObjectT, class MapT> ObjectT* FindObjectInMap(MapT& map, const wxString& id)
	{
		auto it = map.find(id);
		if (it != map.end())
		{
			return &it->second;
		}
		return NULL;
	}
	
	template<class VectorT> void SortByOrder(VectorT& items)
	{
		std::sort(items.begin(), items.end(), [](const auto& v1, const auto& v2)
		{
			return v1->GetSortOrder() < v2->GetSortOrder();
		});
	}
	template<class VectorT> void SortByInstanceID(VectorT& items)
	{
		std::sort(items.begin(), items.end(), [](const auto& v1, const auto& v2)
		{
			return KxComparator::IsLess(v1->GetInstanceID(), v2->GetInstanceID(), true);
		});
	}

	wxString GetVariablesSectionName()
	{
		return wxS("KGameInstance::Variables");
	}
}

//////////////////////////////////////////////////////////////////////////
bool KGameInstance::IsValidInstanceID(const wxString& id)
{
	// Restrict max ID length to 64 symbols
	if (!id.IsEmpty() && id.Length() <= 64)
	{
		return !KAux::HasForbiddenFileNameChars(id);
	}
	return false;
}
bool KGameInstance::IsValidProfileID(const wxString& id)
{
	// Same rules
	return IsValidInstanceID(id);
}

KGameInstance* KGameInstance::CreateActive(const KGameInstance& instanceTemplate, const wxString& instanceID)
{
	KActiveGameInstance* instance = new KActiveGameInstance(instanceTemplate, instanceID);
	AssignActive(*instance);
	if (instance->InitInstance())
	{
		instance->SaveConfig();
		return instance;
	}
	else
	{
		DestroyActive();
		return NULL;
	}
}

wxString KGameInstance::GetTemplatesFolder()
{
	return KApp::Get().GetDataFolder() + wxS("\\InstanceTemplates");
}
wxString KGameInstance::GetUserTemplatesFolder()
{
	return KApp::Get().GetUserSettingsFolder() + wxS("\\InstanceTemplates");
}
void KGameInstance::LoadTemplates()
{
	ms_InstanceTemplates.clear();
	FindInstanceTemplates(GetTemplatesFolder(), true);
	FindInstanceTemplates(GetUserTemplatesFolder(), false);

	SortByOrder(ms_InstanceTemplates);
}

size_t KGameInstance::GetTemplatesCount()
{
	return ms_InstanceTemplates.size();
}
KGameInstance::Vector& KGameInstance::GetTemplates()
{
	return ms_InstanceTemplates;
}
KGameInstance* KGameInstance::GetTemplate(const KGameID& id)
{
	return FindObjectInVector<KGameInstance, FindBy::GameID>(ms_InstanceTemplates, id);
}
bool KGameInstance::HasTemplate(const KGameID& id)
{
	return GetTemplate(id) != NULL;
}

void KGameInstance::FindInstanceTemplates(const wxString& path, bool isSystem)
{
	KxFileFinder finder(path, wxS("*.xml"));
	KxFileItem item = finder.FindNext();
	while (item.IsOK())
	{
		if (item.IsFile() && item.IsNormalItem())
		{
			KGameInstance& instance = *ms_InstanceTemplates.emplace_back(std::make_unique<KGameInstance>(item.GetFullPath(), wxEmptyString, isSystem));
			if (!instance.InitInstance())
			{
				ms_InstanceTemplates.pop_back();
			}
		}
		item = finder.FindNext();
	};
}

KActiveGameInstance* KGameInstance::GetActive()
{
	return ms_ActiveInstance.get();
}
void KGameInstance::AssignActive(KActiveGameInstance& instance)
{
	ms_ActiveInstance.reset(&instance);
}
void KGameInstance::DestroyActive()
{
	ms_ActiveInstance.reset();
}

wxString KGameInstance::GetActiveProfileID()
{
	if (const KProfile* profile = GetActiveProfile())
	{
		return profile->GetID();
	}
	return wxEmptyString;
}
KProfile* KGameInstance::GetActiveProfile()
{
	if (KGameInstance::GetActive())
	{
		KActiveGameInstance* instance = static_cast<KActiveGameInstance*>(KGameInstance::GetActive());
		return instance->GetActiveProfile();
	}
	return NULL;
}
bool KGameInstance::IsActiveProfileID(const wxString& id)
{
	if (const KProfile* profile = GetActiveProfile())
	{
		return KxComparator::IsEqual(profile->GetID(), id, true);
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void KGameInstance::LoadInstancesList()
{
	m_Instances.clear();

	KxFileFinder finder(GetInstanceDir(), wxS("*"));
	KxFileItem item = finder.FindNext();
	while (item.IsOK())
	{
		if (item.IsDirectory() && item.IsNormalItem())
		{
			KGameInstance& instance = *m_Instances.emplace_back(std::make_unique<KConfigurableGameInstance>(*this, item.GetName()));
			if (!instance.InitInstance())
			{
				m_Instances.pop_back();
			}
		}
		item = finder.FindNext();
	};
	SortByOrder(m_Instances);
}
bool KGameInstance::InitInstance()
{
	m_Variables.SetVariable("InstanceTemplateLocation", m_TemplateFile);

	// Load template XML
	KxFileStream instanceConfigStream(m_TemplateFile);
	KxXMLDocument instanceConfig(instanceConfigStream);

	// Load ID and SortOrder
	KxXMLNode node = instanceConfig.QueryElement("Instance");
	m_GameID = node.GetAttribute("GameID");
	if (m_GameID.IsOK())
	{
		m_SortOrder = node.GetAttributeInt("SortOrder");

		// Load name
		node = node.QueryElement("Name");
		m_Name = node.GetValue();
		m_NameShort = node.GetAttribute("Short");

		// Set base variables
		m_Variables.SetVariable(wxS("GameID"), KIVariableValue(m_GameID));
		m_Variables.SetVariable(wxS("GameName"), KAux::StrOr(m_Name, m_NameShort, m_GameID));
		m_Variables.SetVariable(wxS("GameShortName"), KAux::StrOr(m_NameShort, m_Name, m_GameID));
		m_Variables.SetVariable(wxS("GameSortOrder"), KIVariableValue(KxFormat(wxS("%1")).arg(m_SortOrder)));

		if (IsTemplate())
		{
			LoadInstancesList();
			return IsOK();
		}
		if (OnLoadInstance(instanceConfig))
		{
			return IsOK();
		}

		// Reset ID if instance is not loaded correctly
		m_GameID = KGameIDs::NullGameID;
	}
	return IsOK();
}
wxString KGameInstance::CreateProfileID(const wxString& id) const
{
	if (id.IsEmpty() || HasProfile(id))
	{
		return KxFormat("%1 (%2)").arg(id.IsEmpty() ? wxS("Profile") : id).arg(GetProfilesCount() + 1);
	}
	return id;
}
wxString KGameInstance::CreateDefaultProfileID() const
{
	return wxS("Default");
}

KGameInstance::KGameInstance(const wxString& templateFile, const wxString& instanceID, bool isSystemTemplate)
	:m_TemplateFile(templateFile), m_InstanceID(instanceID), m_IsSystemTemplate(isSystemTemplate)
{
}
KGameInstance::~KGameInstance()
{
}

// Variables
wxString KGameInstance::ExpandVariablesLocally(const wxString& variables) const
{
	return m_Variables.Expand(variables);
}
wxString KGameInstance::ExpandVariables(const wxString& variables) const
{
	return KApp::Get().ExpandVariablesLocally(ExpandVariablesLocally(variables));
}

// Properties
bool KGameInstance::IsActiveInstance() const
{
	KGameInstance* Instance = KGameInstance::GetActive();
	return Instance && Instance->GetGameID() == m_GameID && Instance->GetInstanceID() == m_InstanceID;
}

wxString KGameInstance::GetIconLocation() const
{
	wxString path = KxFormat(wxS("%1\\Icons\\%2.ico")).arg(KGameInstance::GetTemplatesFolder()).arg(GetGameID());
	if (!KxFile(path).IsFileExist())
	{
		path = KxFormat(wxS("%1\\Icons\\Generic.ico")).arg(KGameInstance::GetTemplatesFolder());
	}
	return path;
}
wxBitmap KGameInstance::GetIcon() const
{
	wxBitmap bitmap(GetIconLocation(), wxBITMAP_TYPE_ANY);
	if (!bitmap.IsOk())
	{
		if (KProgramManager* programsConfig = KProgramManager::GetInstance())
		{
			const KProgramEntry::Vector& items = programsConfig->GetProgramList();
			if (!items.empty())
			{
				const KProgramEntry& programEntry = items.front();
				if (!programEntry.GetIconPath().IsEmpty())
				{
					bitmap = KxShell::GetFileIcon(KDispatcher::GetInstance()->ResolveLocationPath(programEntry.GetIconPath()));
				}
				else
				{
					bitmap = KxShell::GetFileIcon(KDispatcher::GetInstance()->ResolveLocationPath(programEntry.GetExecutable()));
				}
			}
		}
	}

	if (bitmap.IsOk())
	{
		KBitmapSize size;
		size.FromSystemIcon();
		if (bitmap.GetWidth() != size.GetWidth() || bitmap.GetHeight() != size.GetHeight())
		{
			bitmap = size.ScaleBitmapAspect(bitmap);
		}
	}
	return bitmap;
}

wxString KGameInstance::GetInstanceTemplateDir() const
{
	return KApp::Get().GetInstancesRoot() + wxS('\\') + GetGameID();
}
wxString KGameInstance::GetInstanceDir() const
{
	return GetInstanceTemplateDir() + wxS('\\') + GetInstanceID();
}
wxString KGameInstance::GetInstanceRelativePath(const wxString& name) const
{
	return GetInstanceDir() + wxS('\\') + name;
}

wxString KGameInstance::GetConfigFile() const
{
	return GetInstanceRelativePath(wxS("Instance.ini"));
}
wxString KGameInstance::GetModTagsFile() const
{
	return GetInstanceRelativePath(wxS("ModTags.xml"));
}
wxString KGameInstance::GetProgramsFile() const
{
	return GetInstanceRelativePath(wxS("Programs.xml"));
}
wxString KGameInstance::GetModsDir() const
{
	return GetInstanceRelativePath(wxS("Mods"));
}
wxString KGameInstance::GetProfilesDir() const
{
	return GetInstanceRelativePath(wxS("Profiles"));
}
wxString KGameInstance::GetGameDir() const
{
	return GetVariables().GetVariable(KVAR_ACTUAL_GAME_DIR);
}
wxString KGameInstance::GetVirtualGameDir() const
{
	return GetInstanceRelativePath(wxS("VirtualGameDir"));
}

// Instances
const KGameInstance* KGameInstance::GetInstance(const wxString& id) const
{
	return FindObjectInVector<const KGameInstance, FindBy::InstanceID>(m_Instances, id);
}
KGameInstance* KGameInstance::GetInstance(const wxString& id)
{
	return FindObjectInVector<KGameInstance, FindBy::InstanceID>(m_Instances, id);
}

KGameInstance* KGameInstance::AddInstance(const wxString& instanceID)
{
	if (IsTemplate() && !HasInstance(instanceID))
	{
		KGameInstance& instance = *m_Instances.emplace_back(std::make_unique<KConfigurableGameInstance>(*this, instanceID)).get();
		if (instance.InitInstance())
		{
			SortByInstanceID(m_Instances);
			return &instance;
		}
		m_Instances.pop_back();
	}
	return NULL;
}
KGameInstance* KGameInstance::AddInstanceToTemplate(const wxString& instanceID)
{
	if (!IsTemplate())
	{
		KGameInstance* instanceTemplate = FindObjectInVector<KGameInstance, FindBy::GameID>(ms_InstanceTemplates, m_GameID);
		return instanceTemplate->AddInstance(instanceID);
	}
	return NULL;
}

bool KGameInstance::Deploy(const KGameInstance* baseInstance, uint32_t copyOptions)
{
	if (!IsTemplate() && !IsDeployed())
	{
		KxFile(GetModsDir()).CreateFolder();
		KxFile(GetProfilesDir()).CreateFolder();

		if (baseInstance)
		{
			if (copyOptions & CopyOptionsInstance::Config)
			{
				KxFile(baseInstance->GetConfigFile()).CopyFile(GetConfigFile(), false);
			}
			if (copyOptions & CopyOptionsInstance::ModTags)
			{
				KxFile(baseInstance->GetModTagsFile()).CopyFile(GetModTagsFile(), false);
			}
			if (copyOptions & CopyOptionsInstance::Programs)
			{
				KxFile(baseInstance->GetProgramsFile()).CopyFile(GetProgramsFile(), false);
			}
		}
		return true;
	}
	return false;
}
bool KGameInstance::IsDeployed() const
{
	return !IsTemplate() && KxFile(GetInstanceDir()).IsFolderExist();
}
bool KGameInstance::WithdrawDeploy()
{
	if (!IsTemplate() && IsDeployed())
	{
		// Move to recycle bin
		KxFile path(GetInstanceDir());
		path.RemoveFolderTree(true, true);
		path.RemoveFolder(true);

		Vector::const_iterator it;
		Vector& items = GetTemplate().GetActiveInstances();
		if (FindObjectInVector<KGameInstance, FindBy::InstanceID>(items, m_InstanceID, &it))
		{
			items.erase(it);
		}
		return true;
	}
	return false;
}

// Profiles
const KProfile* KGameInstance::GetProfile(const wxString& profileID) const
{
	return FindObjectInVector<const KProfile, FindBy::ProfileID>(m_Profiles, KProfile::ProcessID(profileID));
}
KProfile* KGameInstance::GetProfile(const wxString& profileID)
{
	return FindObjectInVector<KProfile, FindBy::ProfileID>(m_Profiles, KProfile::ProcessID(profileID));
}

KProfile* KGameInstance::CreateProfile(const wxString& profileID, const KProfile* baseProfile, uint32_t copyOptions)
{
	KProfileEvent event(KEVT_PROFILE_ADDING);
	event.Send();

	wxString id = profileID;
	if (id.IsEmpty())
	{
		id = CreateProfileID(id);
	}

	if (event.IsAllowed() && !HasProfile(id))
	{
		KProfile& newProfile = *m_Profiles.emplace_back(std::make_unique<KProfile>(id));
		KxFile(newProfile.GetProfileDir()).CreateFolder();
		newProfile.Save();

		// Do copy
		KOperationWithProgressDialogBase* operation = new KOperationWithProgressDialog<KxFileOperationEvent>(false);
		operation->OnRun([this, baseProfile, &newProfile, copyOptions](KOperationWithProgressBase* self)
		{
			// Begin copying data
			if (baseProfile)
			{
				if (copyOptions & CopyOptionsProfile::GameConfig)
				{
					KxEvtFile source(baseProfile->GetConfigDir());
					self->LinkHandler(&source, KxEVT_FILEOP_COPY_FOLDER);
					source.CopyFolder(KxFile::NullFilter, newProfile.GetConfigDir(), true, true);
				}
				if (copyOptions & CopyOptionsProfile::GameSaves)
				{
					KxEvtFile source(baseProfile->GetSavesDir());
					self->LinkHandler(&source, KxEVT_FILEOP_COPY_FOLDER);
					source.CopyFolder(KxFile::NullFilter, newProfile.GetSavesDir(), true, true);
				}
			}
		});

		// Reload after task is completed (successfully or not)
		operation->OnEnd([this, &newProfile](KOperationWithProgressBase* self)
		{
			{
				KProfileEvent* event = new KProfileEvent(KEVT_PROFILE_ADDED, newProfile);
				event->Queue();
			}
			{
				KProfileEvent* event = new KProfileEvent(KEVT_PROFILE_UPDATE_LIST);
				event->Queue();
			}
		});

		// Configure and run
		operation->SetDialogCaption(T("Generic.FileCopyInProgress"));
		operation->Run();
		return &newProfile;
	}
	return NULL;
}
KProfile* KGameInstance::ShallowCopyProfile(const KProfile& profile, const wxString& nameSuggets)
{
	KProfileEvent event(KEVT_PROFILE_ADDING);
	event.Send();

	wxString newName = CreateProfileID(nameSuggets.IsEmpty() ? profile.GetID() : nameSuggets);
	if (event.IsAllowed() && !HasProfile(newName))
	{
		KProfile& newProfile = *m_Profiles.emplace_back(std::make_unique<KProfile>(profile));
		newProfile.SetID(newName);

		KxFile(newProfile.GetProfileDir()).CreateFolder();
		newProfile.Save();

		return &newProfile;
	}
	return NULL;
}

bool KGameInstance::RemoveProfile(KProfile& profile)
{
	KProfileEvent addingEvent(KEVT_PROFILE_REMOVING);
	addingEvent.Send();

	bool isCurrent = profile.IsActive();
	if (addingEvent.IsAllowed() && !isCurrent && HasProfile(profile.GetID()))
	{
		// Move files to recycle bin
		KxFile path(profile.GetProfileDir());
		path.RemoveFolderTree(true, true);
		path.RemoveFolder(true);

		// Remove it from profiles list
		ProfilesVector::const_iterator it;
		if (FindObjectInVector<const KProfile, FindBy::ProfileID>(m_Profiles, profile.GetID(), &it))
		{
			wxString id = profile.GetID();
			m_Profiles.erase(it);

			KProfileEvent addedEvent(KEVT_PROFILE_REMOVED, id);
			addedEvent.Send();

			if (IsActiveInstance())
			{
				if (!HasProfiles())
				{
					ChangeProfileTo(*CreateProfile(CreateDefaultProfileID()));
				}
				
				if (isCurrent)
				{
					ChangeProfileTo(*GetProfiles().front());
				}
			}
			return true;
		}
	}
	return false;
}
bool KGameInstance::RenameProfile(KProfile& profile, const wxString& newID)
{
	const wxString oldPath = profile.GetProfileDir();
	const wxString oldID = profile.GetID();
	const bool isCurrent = profile.IsActive();

	profile.SetID(newID);
	if (KxFile(oldPath).Rename(profile.GetProfileDir(), false))
	{
		if (isCurrent && KGameInstance::GetActive())
		{
			KActiveGameInstance* instance = static_cast<KActiveGameInstance*>(KGameInstance::GetActive());
			instance->SetCurrentProfileID(newID);
		}
		return true;
	}
	else
	{
		profile.SetID(oldID);
		return false;
	}
}
bool KGameInstance::ChangeProfileTo(const KProfile& profile)
{
	if (KGameInstance::GetActive())
	{
		KActiveGameInstance* instance = static_cast<KActiveGameInstance*>(KGameInstance::GetActive());
		instance->ChangeProfileTo(profile);
		return true;
	}
	return false;
}
void KGameInstance::LoadSavedProfileOrDefault()
{
	if (KGameInstance::GetActive())
	{
		KActiveGameInstance* instance = static_cast<KActiveGameInstance*>(KGameInstance::GetActive());
		instance->LoadSavedProfileOrDefault();
	}
}

//////////////////////////////////////////////////////////////////////////
void KConfigurableGameInstance::LoadVariables(const KxXMLDocument& instanceConfig)
{
	KIVariablesTable& variables = GetVariables();
	const wxString variablesSectionName = GetVariablesSectionName();

	// System variables
	variables.SetVariable(KVAR_INSTANCE_DIR, GetInstanceDir());
	variables.SetVariable(KVAR_VIRTUAL_GAME_DIR, GetVirtualGameDir());
	variables.SetVariable(KVAR_MODS_DIR, GetModsDir());
	variables.SetVariable(KVAR_PROFILES_DIR, GetProfilesDir());

	KxXMLNode node = instanceConfig.QueryElement("Instance/Variables");
	for (node = node.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
	{
		const wxString id = node.GetAttribute("ID");
		const wxString typeString = node.GetAttribute("Type");
		const wxString source = node.GetAttribute("Source");
		const bool saveAsOverride = node.GetAttributeBool("SaveAsOverride");

		wxString value;
		KIVariableValue::Type type = KIVariableValue::Type::None;

		// Load from source
		if (source == wxS("Registry"))
		{
			value = LoadRegistryVariable(node);
		}
		else
		{
			value = ExpandVariables(node.GetValue());
		}

		// Process depending on type
		if (typeString == wxS("FSPath"))
		{
			type = KIVariableValue::Type::FSPath;
			value = KxFile(value).GetPath();
		}

		variables.SetVariable(id, KIVariableValue(value, saveAsOverride, type));
	}

	// Override any variables from file
	for (const wxString& name: m_Config.GetKeyNames(variablesSectionName))
	{
		wxString value = m_Config.GetValue(variablesSectionName, name);
		if (!value.IsEmpty())
		{
			variables.SetVariable(name, KIVariableValue(ExpandVariables(value), true));
		}
	}
}
void KConfigurableGameInstance::LoadProfiles(const KxXMLDocument& instanceConfig)
{
	GetProfiles().clear();

	KxFileFinder finder(GetProfilesDir(), wxS("*"));
	KxFileItem item = finder.FindNext();
	while (item.IsOK())
	{
		if (item.IsDirectory() && item.IsNormalItem())
		{
			KProfile& profile = *GetProfiles().emplace_back(std::make_unique<KProfile>(item.GetName()));
			if (ShouldInitProfiles())
			{
				profile.Load();
			}
		}
		item = finder.FindNext();
	};
}
wxString KConfigurableGameInstance::LoadRegistryVariable(const KxXMLNode& node) const
{
	static const std::unordered_map<wxString, KxRegistryHKey> ms_NameToRegKey =
	{
		std::make_pair(wxS("HKEY_CLASSES_ROOT"), KxREG_HKEY_CLASSES_ROOT),
		std::make_pair(wxS("HKEY_CURRENT_USER"), KxREG_HKEY_CURRENT_USER),
		std::make_pair(wxS("HKEY_LOCAL_MACHINE"), KxREG_HKEY_LOCAL_MACHINE),
		std::make_pair(wxS("HKEY_USERS"), KxREG_HKEY_USERS),
		std::make_pair(wxS("HKEY_CURRENT_CONFIG"), KxREG_HKEY_CURRENT_CONFIG)
	};

	static const std::unordered_map<wxString, KxRegistryValueType> ms_NameToRegType =
	{
		std::make_pair(wxS("REG_VALUE_SZ"), KxREG_VALUE_SZ),
		std::make_pair(wxS("REG_VALUE_EXPAND_SZ"), KxREG_VALUE_EXPAND_SZ),
		std::make_pair(wxS("REG_VALUE_MULTI_SZ"), KxREG_VALUE_MULTI_SZ),
		std::make_pair(wxS("REG_VALUE_DWORD"), KxREG_VALUE_DWORD),
		std::make_pair(wxS("REG_VALUE_QWORD"), KxREG_VALUE_QWORD)
	};

	// 32 or 64 bit registry branch
	KxRegistryNode regBranch = KxREG_NODE_SYS;
	switch (node.GetFirstChildElement("Branch").GetValueInt())
	{
		case 32:
		{
			regBranch = KxREG_NODE_32;
			break;
		}
		case 64:
		{
			regBranch = KxREG_NODE_64;
			break;
		}
	};

	// Main key
	const KxRegistryHKey* mainKey = FindObjectInMap<const KxRegistryHKey>(ms_NameToRegKey, node.GetFirstChildElement("Root").GetValue());
	if (mainKey)
	{
		wxString path = ExpandVariables(node.GetFirstChildElement("Path").GetValue());
		wxString name = ExpandVariables(node.GetFirstChildElement("Name").GetValue());
		const KxRegistryValueType* type = FindObjectInMap<const KxRegistryValueType>(ms_NameToRegType, node.GetFirstChildElement("Type").GetValue());

		wxAny data = KxRegistry::GetValue(*mainKey, path, name, type ? *type : KxREG_VALUE_ANY, regBranch, true);
		return data.As<wxString>();
	}
	return wxEmptyString;
}
void KConfigurableGameInstance::DetectGameArchitecture(const KxXMLDocument& instanceConfig)
{
	KIVariablesTable& variables = GetVariables();
	bool is64Bit = KxFile(variables.GetVariable("GameExecutable")).GetBinaryType() == KxFBF_WIN64;

	variables.SetVariable("GameArchitecture", KAux::ArchitectureToNumber(is64Bit));
	variables.SetVariable("GameArchitectureName", KAux::ArchitectureToString(is64Bit));
}

bool KConfigurableGameInstance::OnLoadInstance(const KxXMLDocument& instanceConfig)
{
	// Load config file
	KxFileStream configStream(GetConfigFile(), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	m_Config.Load(configStream);

	// Load data
	LoadVariables(instanceConfig);
	DetectGameArchitecture(instanceConfig);
	LoadProfiles(instanceConfig);

	return true;
}

KConfigurableGameInstance::KConfigurableGameInstance(const KGameInstance& instanceTemplate, const wxString& instanceID)
	:KGameInstance(instanceTemplate.GetTemplateFile(), instanceID, instanceTemplate.IsSystemTemplate())
{
}
KConfigurableGameInstance::~KConfigurableGameInstance()
{
}

bool KConfigurableGameInstance::SaveConfig()
{
	GetVariables().Accept([this](const wxString& name, const KIVariableValue& value)
	{
		if (value.IsOverride())
		{
			m_Config.SetValue(GetVariablesSectionName(), name, value.GetValue());
		}
		return true;
	});

	KxFileStream configStream(GetConfigFile(), KxFS_ACCESS_WRITE, KxFS_DISP_CREATE_ALWAYS, KxFS_SHARE_READ);
	return m_Config.Save(configStream);
}
