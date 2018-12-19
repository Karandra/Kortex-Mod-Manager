#include "stdafx.h"
#include "DefaultGameInstance.h"
#include "ActiveGameInstance.h"
#include "DefaultGameProfile.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/Events.hpp>
#include "IGameProfile.h"
#include "ProgramManager/KProgramManager.h"
#include "KOperationWithProgress.h"
#include "KBitmapSize.h"
#include "KAux.h"
#include "Util.h"
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
	enum class FindBy
	{
		GameID,
		InstanceID,
		ProfileID
	};
	template<class ObjectT, FindBy findBy, class VectorT> ObjectT* FindObjectInVector(VectorT& storageVector, const wxString& value, typename VectorT::const_iterator* itOut = nullptr)
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
		return nullptr;
	}
	template<class ObjectT, class MapT> ObjectT* FindObjectInMap(MapT& map, const wxString& id)
	{
		auto it = map.find(id);
		if (it != map.end())
		{
			return &it->second;
		}
		return nullptr;
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
}

namespace Kortex::GameInstance
{
	void DefaultGameInstance::LoadInstancesList()
	{
		m_Instances.clear();

		KxFileFinder finder(GetInstanceDir(), wxS("*"));
		KxFileItem item = finder.FindNext();
		while (item.IsOK())
		{
			if (item.IsDirectory() && item.IsNormalItem())
			{
				auto instance = std::make_unique<ConfigurableGameInstance>(*this, item.GetName());
				if (instance->InitInstance())
				{
					m_Instances.emplace_back(std::move(instance));
				}
			}
			item = finder.FindNext();
		};
		SortByOrder(m_Instances);
	}
	bool DefaultGameInstance::InitInstance()
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
			m_Variables.SetVariable(wxS("GameID"), VariableValue(m_GameID));
			m_Variables.SetVariable(wxS("GameName"), KAux::StrOr(m_Name, m_NameShort, m_GameID));
			m_Variables.SetVariable(wxS("GameShortName"), KAux::StrOr(m_NameShort, m_Name, m_GameID));
			m_Variables.SetVariable(wxS("GameSortOrder"), VariableValue(KxFormat(wxS("%1")).arg(m_SortOrder)));

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
			m_GameID = GameIDs::NullGameID;
		}
		return IsOK();
	}
	wxString DefaultGameInstance::CreateProfileID(const wxString& id) const
	{
		if (id.IsEmpty() || HasProfile(id))
		{
			return KxString::Format(wxS("%1 (%2)"), id.IsEmpty() ? wxS("Profile") : id, GetProfilesCount() + 1);
		}
		return id;
	}
	wxString DefaultGameInstance::CreateDefaultProfileID() const
	{
		return wxS("Default");
	}

	// Variables
	wxString DefaultGameInstance::ExpandVariablesLocally(const wxString& variables) const
	{
		return m_Variables.Expand(variables);
	}
	wxString DefaultGameInstance::ExpandVariables(const wxString& variables) const
	{
		return IApplication::GetInstance()->ExpandVariablesLocally(ExpandVariablesLocally(variables));
	}

	// Properties
	bool DefaultGameInstance::IsActiveInstance() const
	{
		IGameInstance* Instance = IGameInstance::GetActive();
		return Instance && Instance->GetGameID() == m_GameID && Instance->GetInstanceID() == m_InstanceID;
	}

	wxString DefaultGameInstance::GetIconLocation() const
	{
		wxString path = KxString::Format(wxS("%1\\Icons\\%2.ico"), IGameInstance::GetTemplatesFolder(), GetGameID());
		if (!KxFile(path).IsFileExist())
		{
			path = KxString::Format(wxS("%1\\Icons\\Generic.ico"), IGameInstance::GetTemplatesFolder());
		}
		return path;
	}
	wxBitmap DefaultGameInstance::GetIcon() const
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
						bitmap = KxShell::GetFileIcon(Kortex::IModDispatcher::GetInstance()->ResolveLocationPath(programEntry.GetIconPath()));
					}
					else
					{
						bitmap = KxShell::GetFileIcon(Kortex::IModDispatcher::GetInstance()->ResolveLocationPath(programEntry.GetExecutable()));
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

	wxString DefaultGameInstance::GetInstanceTemplateDir() const
	{
		return IApplication::GetInstance()->GetInstancesFolder() + wxS('\\') + GetGameID();
	}
	wxString DefaultGameInstance::GetInstanceDir() const
	{
		return GetInstanceTemplateDir() + wxS('\\') + GetInstanceID();
	}
	wxString DefaultGameInstance::GetInstanceRelativePath(const wxString& name) const
	{
		return GetInstanceDir() + wxS('\\') + name;
	}

	wxString DefaultGameInstance::GetConfigFile() const
	{
		return GetInstanceRelativePath(wxS("Instance.xml"));
	}
	wxString DefaultGameInstance::GetModTagsFile() const
	{
		return GetInstanceRelativePath(wxS("ModTags.xml"));
	}
	wxString DefaultGameInstance::GetProgramsFile() const
	{
		return GetInstanceRelativePath(wxS("Programs.xml"));
	}
	wxString DefaultGameInstance::GetModsDir() const
	{
		return GetInstanceRelativePath(wxS("Mods"));
	}
	wxString DefaultGameInstance::GetProfilesDir() const
	{
		return GetInstanceRelativePath(wxS("Profiles"));
	}
	wxString DefaultGameInstance::GetGameDir() const
	{
		return GetVariables().GetVariable(Variables::KVAR_ACTUAL_GAME_DIR);
	}
	wxString DefaultGameInstance::GetVirtualGameDir() const
	{
		return GetInstanceRelativePath(wxS("VirtualGameDir"));
	}

	// Instances
	const IGameInstance* DefaultGameInstance::GetInstance(const wxString& id) const
	{
		return FindObjectInVector<const IGameInstance, FindBy::InstanceID>(m_Instances, id);
	}
	IGameInstance* DefaultGameInstance::GetInstance(const wxString& id)
	{
		return FindObjectInVector<IGameInstance, FindBy::InstanceID>(m_Instances, id);
	}

	IGameInstance* DefaultGameInstance::AddInstance(const wxString& instanceID)
	{
		if (IsTemplate() && !HasInstance(instanceID))
		{
			auto instance = std::make_unique<ConfigurableGameInstance>(*this, instanceID);
			if (instance->InitInstance())
			{
				IGameInstance* ptr = instance.get();
				m_Instances.emplace_back(std::move(instance));
				SortByInstanceID(m_Instances);
				return ptr;
			}
		}
		return nullptr;
	}
	IGameInstance* DefaultGameInstance::AddInstanceToTemplate(const wxString& instanceID)
	{
		if (!IsTemplate())
		{
			IGameInstance* instanceTemplate = FindObjectInVector<IGameInstance, FindBy::GameID>(GetTemplates(), m_GameID);
			return instanceTemplate->AddInstance(instanceID);
		}
		return nullptr;
	}

	bool DefaultGameInstance::Deploy(const IGameInstance* baseInstance, uint32_t copyOptions)
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
	bool DefaultGameInstance::IsDeployed() const
	{
		return !IsTemplate() && KxFile(GetInstanceDir()).IsFolderExist();
	}
	bool DefaultGameInstance::WithdrawDeploy()
	{
		if (!IsTemplate() && IsDeployed())
		{
			// Move to recycle bin
			KxFile path(GetInstanceDir());
			path.RemoveFolderTree(true, true);
			path.RemoveFolder(true);

			Vector::const_iterator it;
			Vector& items = GetTemplate().GetActiveInstances();
			if (FindObjectInVector<IGameInstance, FindBy::InstanceID>(items, m_InstanceID, &it))
			{
				items.erase(it);
			}
			return true;
		}
		return false;
	}

	// Profiles
	const IGameProfile* DefaultGameInstance::GetProfile(const wxString& profileID) const
	{
		return FindObjectInVector<const IGameProfile, FindBy::ProfileID>(m_Profiles, IGameProfile::ProcessID(profileID));
	}
	IGameProfile* DefaultGameInstance::GetProfile(const wxString& profileID)
	{
		return FindObjectInVector<IGameProfile, FindBy::ProfileID>(m_Profiles, IGameProfile::ProcessID(profileID));
	}

	std::unique_ptr<IGameProfile> DefaultGameInstance::NewProfile()
	{
		return std::make_unique<DefaultGameProfile>();
	}
	IGameProfile* DefaultGameInstance::CreateProfile(const wxString& profileID, const IGameProfile* baseProfile, uint32_t copyOptions)
	{
		ProfileEvent event(Events::ProfileAdding);
		event.Send();

		wxString id = profileID;
		if (id.IsEmpty())
		{
			id = CreateProfileID(id);
		}

		if (event.IsAllowed() && !HasProfile(id))
		{
			IGameProfile& newProfile = *m_Profiles.emplace_back(NewProfile());
			newProfile.SetID(id);
			KxFile(newProfile.GetProfileDir()).CreateFolder();
			newProfile.SaveConfig();

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
					ProfileEvent* event = new ProfileEvent(Events::ProfileAdded, newProfile);
					event->Queue();
				}
				{
					ProfileEvent* event = new ProfileEvent(Events::ProfileRefreshList);
					event->Queue();
				}
			});

			// Configure and run
			operation->SetDialogCaption(KTr("Generic.FileCopyInProgress"));
			operation->Run();
			return &newProfile;
		}
		return nullptr;
	}
	IGameProfile* DefaultGameInstance::ShallowCopyProfile(const IGameProfile& profile, const wxString& nameSuggets)
	{
		ProfileEvent event(Events::ProfileAdding);
		event.Send();

		wxString newName = CreateProfileID(nameSuggets.IsEmpty() ? profile.GetID() : nameSuggets);
		if (event.IsAllowed() && !HasProfile(newName))
		{
			IGameProfile& newProfile = *m_Profiles.emplace_back(profile.Clone());
			newProfile.SetID(newName);

			KxFile(newProfile.GetProfileDir()).CreateFolder();
			newProfile.SaveConfig();

			ProfileEvent event(Events::ProfileAdded, newProfile);
			event.Send();

			return &newProfile;
		}
		return nullptr;
	}

	bool DefaultGameInstance::RemoveProfile(IGameProfile& profile)
	{
		ProfileEvent removingEvent(Events::ProfileRemoving);
		removingEvent.Send();

		bool isCurrent = profile.IsActive();
		if (removingEvent.IsAllowed() && !isCurrent && HasProfile(profile.GetID()))
		{
			// Move files to recycle bin
			KxFile path(profile.GetProfileDir());
			path.RemoveFolderTree(true, true);
			path.RemoveFolder(true);

			// Remove it from profiles list
			ProfilesVector::const_iterator it;
			if (FindObjectInVector<const IGameProfile, FindBy::ProfileID>(m_Profiles, profile.GetID(), &it))
			{
				wxString id = profile.GetID();
				m_Profiles.erase(it);

				ProfileEvent removedEvent(Events::ProfileRemoved, id);
				removedEvent.Send();

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
	bool DefaultGameInstance::RenameProfile(IGameProfile& profile, const wxString& newID)
	{
		const wxString oldPath = profile.GetProfileDir();
		const wxString oldID = profile.GetID();
		const bool isCurrent = profile.IsActive();

		profile.SetID(newID);
		if (KxFile(oldPath).Rename(profile.GetProfileDir(), false))
		{
			if (isCurrent && IGameInstance::GetActive())
			{
				ActiveGameInstance* instance = static_cast<ActiveGameInstance*>(IGameInstance::GetActive());
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
	bool DefaultGameInstance::ChangeProfileTo(const IGameProfile& profile)
	{
		if (IGameInstance::GetActive())
		{
			ActiveGameInstance* instance = static_cast<ActiveGameInstance*>(IGameInstance::GetActive());
			instance->DoChangeProfileTo(profile);
			return true;
		}
		return false;
	}
	void DefaultGameInstance::LoadSavedProfileOrDefault()
	{
		if (IGameInstance::GetActive())
		{
			ActiveGameInstance* instance = static_cast<ActiveGameInstance*>(IGameInstance::GetActive());
			instance->LoadSavedProfileOrDefault();
		}
	}
}

namespace Kortex::GameInstance
{
	void ConfigurableGameInstance::LoadVariables(const KxXMLDocument& instanceConfig)
	{
		IVariableTable& variables = GetVariables();

		// System variables
		variables.SetVariable(Variables::KVAR_INSTANCE_DIR, GetInstanceDir());
		variables.SetVariable(Variables::KVAR_VIRTUAL_GAME_DIR, GetVirtualGameDir());
		variables.SetVariable(Variables::KVAR_MODS_DIR, GetModsDir());
		variables.SetVariable(Variables::KVAR_PROFILES_DIR, GetProfilesDir());

		auto LoadVariables = [this, &variables](const KxXMLNode& arrayNode, bool noEmptyValues)
		{
			for (KxXMLNode node = arrayNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
			{
				const wxString id = node.GetAttribute("ID");
				const wxString typeString = node.GetAttribute("Type");
				const wxString source = node.GetAttribute("Source");
				const bool saveAsOverride = node.GetAttributeBool("SaveAsOverride");

				wxString value;
				if (source == wxS("Registry"))
				{
					value = LoadRegistryVariable(node);
				}
				else
				{
					value = ExpandVariables(node.GetValue());
				}
				if (noEmptyValues && value.IsEmpty())
				{
					continue;
				}

				// Process depending on type
				VariableValue::Type type = VariableValue::Type::None;
				if (typeString == wxS("FSPath"))
				{
					type = VariableValue::Type::FSPath;
					value = KxFile(value).GetPath();
				}

				variables.SetVariable(id, VariableValue(value, saveAsOverride, type));
			}
		};
		
		// Load template variables
		LoadVariables(instanceConfig.QueryElement("Instance/Variables"), false);

		// Override any variables from file
		LoadVariables(m_Config.QueryElement("Instance/Variables"), true);
	}
	void ConfigurableGameInstance::LoadProfiles(const KxXMLDocument& instanceConfig)
	{
		GetProfiles().clear();

		KxFileFinder finder(GetProfilesDir(), wxS("*"));
		KxFileItem item = finder.FindNext();
		while (item.IsOK())
		{
			if (item.IsDirectory() && item.IsNormalItem())
			{
				IGameProfile& profile = *GetProfiles().emplace_back(NewProfile());
				profile.SetID(item.GetName());
				if (ShouldInitProfiles())
				{
					profile.LoadConfig();
				}
			}
			item = finder.FindNext();
		};
	}
	wxString ConfigurableGameInstance::LoadRegistryVariable(const KxXMLNode& node) const
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
	void ConfigurableGameInstance::DetectGameArchitecture(const KxXMLDocument& instanceConfig)
	{
		IVariableTable& variables = GetVariables();
		bool is64Bit = KxFile(variables.GetVariable("GameExecutable")).GetBinaryType() == KxFBF_WIN64;

		variables.SetVariable("GameArchitecture", KAux::ArchitectureToNumber(is64Bit));
		variables.SetVariable("GameArchitectureName", KAux::ArchitectureToString(is64Bit));
	}

	bool ConfigurableGameInstance::OnLoadInstance(const KxXMLDocument& instanceConfig)
	{
		// Load config file
		KxFileStream configStream(GetConfigFile(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
		m_Config.Load(configStream);

		// Load data
		LoadVariables(instanceConfig);
		DetectGameArchitecture(instanceConfig);
		LoadProfiles(instanceConfig);

		return true;
	}

	ConfigurableGameInstance::ConfigurableGameInstance(const IGameInstance& instanceTemplate, const wxString& instanceID)
	{
		Create(instanceTemplate.GetTemplateFile(), instanceID, instanceTemplate.IsSystemTemplate());
	}

	void ConfigurableGameInstance::OnConfigChanged(IAppOption& option)
	{
		wxLogInfo("ConfigurableGameInstance::OnConfigChanged -> %s", option.GetXPath());
	}
	void ConfigurableGameInstance::SaveConfig()
	{
		KxXMLNode variablesNode = GetInstanceOption("Variables").GetConfigNode();
		variablesNode.ClearChildren();

		GetVariables().Accept([this, &variablesNode](const wxString& name, const VariableValue& value)
		{
			if (value.IsOverride())
			{
				KxXMLNode node = variablesNode.NewElement("Entry");
				
				node.SetAttribute("ID", name);
				switch (value.GetType())
				{
					case VariableValue::Type::FSPath:
					{
						node.SetAttribute("Type", "FSPath");
						break;
					}
				};
				
				node.SetValue(value.GetValue());
			}
			return true;
		});

		KxFileStream configStream(GetConfigFile(), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
		m_Config.Save(configStream);
	}
	void ConfigurableGameInstance::OnExit()
	{
		SaveConfig();
	}
}
