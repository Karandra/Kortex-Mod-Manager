#include "stdafx.h"
#include "DefaultGameInstance.h"
#include "ActiveGameInstance.h"
#include "DefaultGameProfile.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include "IGameProfile.h"
#include "ProfileEvent.h"
#include "Utility/KOperationWithProgress.h"
#include "Utility/KBitmapSize.h"
#include "Utility/KAux.h"
#include "Utility/Log.h"
#include "Util.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxXML.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxRegistry.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxFileOperationEvent.h>
#include <KxFramework/KxDualProgressDialog.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxIndexedEnum.h>

namespace
{
	struct NameToRegKeyDef: public KxIndexedEnum::Definition<NameToRegKeyDef, KxRegistryHKey, wxString>
	{
		inline static const TItem ms_Index[] =
		{
			{KxREG_HKEY_CLASSES_ROOT, wxS("HKEY_CLASSES_ROOT")},
			{KxREG_HKEY_CURRENT_USER, wxS("HKEY_CURRENT_USER")},
			{KxREG_HKEY_LOCAL_MACHINE, wxS("HKEY_LOCAL_MACHINE")},
			{KxREG_HKEY_USERS, wxS("HKEY_USERS")},
			{KxREG_HKEY_CURRENT_CONFIG, wxS("HKEY_CURRENT_CONFIG")},
		};
	};
	struct NameToRegTypeDef: public KxIndexedEnum::Definition<NameToRegTypeDef, KxRegistryValueType, wxString>
	{
		inline static const TItem ms_Index[] =
		{
			{KxREG_VALUE_SZ, wxS("REG_VALUE_SZ")},
			{KxREG_VALUE_EXPAND_SZ, wxS("REG_VALUE_EXPAND_SZ")},
			{KxREG_VALUE_MULTI_SZ, wxS("REG_VALUE_MULTI_SZ")},
			{KxREG_VALUE_DWORD, wxS("REG_VALUE_DWORD")},
			{KxREG_VALUE_QWORD, wxS("REG_VALUE_QWORD")},
		};
	};
}

namespace Kortex::Application::OName
{
	KortexDefOption(GameID);
}

namespace Kortex::GameInstance
{
	bool DefaultGameInstance::InitInstance()
	{
		m_Variables.SetVariable("GameDefinitionFile", m_DefinitionFile);

		// Load template XML
		KxFileStream templateConfigStream(m_DefinitionFile);
		KxXMLDocument templateConfig(templateConfigStream);

		// Load ID and SortOrder
		KxXMLNode node = templateConfig.QueryElement("Definition");
		m_GameID = node.GetAttribute("GameID");
		if (m_GameID.IsOK())
		{
			m_SortOrder = node.GetAttributeInt("SortOrder");

			// Load name
			node = node.QueryElement("Name");
			m_GameName = node.GetValue();
			m_GameShortName = node.GetAttribute("ShortName");

			// Set base variables
			m_Variables.SetVariable(wxS("GameID"), VariableValue(m_GameID));
			m_Variables.SetVariable(wxS("GameName"), KAux::StrOr(m_GameName, m_GameShortName, m_GameID));
			m_Variables.SetVariable(wxS("GameShortName"), KAux::StrOr(m_GameShortName, m_GameName, m_GameID));
			m_Variables.SetVariable(wxS("GameSortOrder"), VariableValue(KxString::Format(wxS("%1"), m_SortOrder)));

			if (IsTemplate())
			{
				LoadVariables(templateConfig);
			}
			else if (OnLoadInstance(templateConfig))
			{
				return IsOK();
			}
			else
			{
				// Reset ID if instance is not loaded correctly
				m_GameID = GameIDs::NullGameID;
			}
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

	void DefaultGameInstance::LoadVariables(const KxXMLDocument& instanceConfig, const KxXMLDocument* userConfig)
	{
		IVariableTable& variables = GetVariables();

		// System variables
		variables.SetVariable(Variables::KVAR_INSTANCE_DIR, GetInstanceDir());
		variables.SetVariable(Variables::KVAR_VIRTUAL_GAME_DIR, GetVirtualGameDir());
		variables.SetVariable(Variables::KVAR_MODS_DIR, GetModsDir());
		variables.SetVariable(Variables::KVAR_PROFILES_DIR, GetProfilesDir());

		auto LoadVariablesFrom = [this, &variables](const KxXMLNode& arrayNode, bool noEmptyValues)
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
				using Type = VariableValue::Type;

				std::optional<Type> type = Type::String;
				if (typeString == wxS("FSPath"))
				{
					type = Type::FSPath;
					value = KxFile(value).GetPath();
				}
				else if (typeString == wxS("Integer"))
				{
					type = Type::Integer;
				}

				// Override mode
				using Override = VariableValue::Override;

				std::optional<Override> overrideMode;
				if (saveAsOverride)
				{
					overrideMode = Override::True;
				}

				variables.SetVariable(id, VariableValue(value, overrideMode, type));
			}
		};

		// Load template variables
		LoadVariablesFrom(instanceConfig.QueryElement("Definition/Variables"), false);

		// Override any variables from file
		if (userConfig)
		{
			LoadVariablesFrom(userConfig->QueryElement("Instance/Variables"), true);
		}
	}
	wxString DefaultGameInstance::LoadRegistryVariable(const KxXMLNode& node) const
	{
		// 32 or 64 bit registry branch
		KxRegistryNode regBranch = KxREG_NODE_SYS;
		switch (node.GetFirstChildElement("Branch").GetValueInt(0))
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
		auto mainKey = NameToRegKeyDef::TryFromString(node.GetFirstChildElement("Root").GetValue());
		if (mainKey)
		{
			wxString path = ExpandVariables(node.GetFirstChildElement("Path").GetValue());
			wxString name = ExpandVariables(node.GetFirstChildElement("Name").GetValue());
			auto type = NameToRegTypeDef::TryFromString(node.GetFirstChildElement("Type").GetValue());

			return KxRegistry::GetValue(*mainKey, path, name, type ? *type : KxREG_VALUE_ANY, regBranch, true).As<wxString>();
		}
		return wxEmptyString;
	}
	void DefaultGameInstance::DetectGameArchitecture(const KxXMLDocument& instanceConfig)
	{
		IVariableTable& variables = GetVariables();
		bool is64Bit = KxFile(variables.GetVariable("GameExecutable").AsString()).GetBinaryType() == KxFBF_WIN64;

		variables.SetVariable("GameArchitecture", KAux::ArchitectureToNumber(is64Bit));
		variables.SetVariable("GameArchitectureName", KAux::ArchitectureToString(is64Bit));
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
	wxString DefaultGameInstance::GetIconLocation() const
	{
		return GetDefaultIconLocation();
	}
	wxBitmap DefaultGameInstance::GetIcon() const
	{
		wxBitmap bitmap = LoadIcon(GetIconLocation());
		return bitmap.IsOk() ? bitmap : GetGenericIcon();
	}

	wxString DefaultGameInstance::GetInstanceTemplateDir() const
	{
		return IApplication::GetInstance()->GetInstancesFolder();
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
		return GetVariables().GetVariable(Variables::KVAR_ACTUAL_GAME_DIR).AsString();
	}
	wxString DefaultGameInstance::GetVirtualGameDir() const
	{
		return GetInstanceRelativePath(wxS("VirtualGameDir"));
	}

	// Profiles
	const IGameProfile* DefaultGameInstance::GetProfile(const wxString& profileID) const
	{
		return Util::FindObjectInVector<const IGameProfile, Util::FindBy::ProfileID>(m_Profiles, IGameProfile::ProcessID(profileID));
	}
	IGameProfile* DefaultGameInstance::GetProfile(const wxString& profileID)
	{
		return Util::FindObjectInVector<IGameProfile, Util::FindBy::ProfileID>(m_Profiles, IGameProfile::ProcessID(profileID));
	}

	std::unique_ptr<IGameProfile> DefaultGameInstance::NewProfile()
	{
		return std::make_unique<DefaultGameProfile>();
	}
	IGameProfile* DefaultGameInstance::CreateProfile(const wxString& profileID, const IGameProfile* baseProfile, uint32_t copyOptions)
	{
		wxString id = profileID;
		if (id.IsEmpty())
		{
			id = CreateProfileID(id);
		}

		bool isAllowed = BroadcastProcessor::Get().ProcessEventEx(ProfileEvent::EvtAdding).Do().IsAllowed();
		if (isAllowed && !HasProfile(id))
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
				BroadcastProcessor::Get().QueueEvent(ProfileEvent::EvtAdded, newProfile);
				BroadcastProcessor::Get().QueueEvent(ProfileEvent::EvtRefreshList);
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
		wxString newName = CreateProfileID(nameSuggets.IsEmpty() ? profile.GetID() : nameSuggets);

		bool isAllowed = BroadcastProcessor::Get().ProcessEventEx(ProfileEvent::EvtAdding).Do().IsAllowed();
		if (isAllowed && !HasProfile(newName))
		{
			IGameProfile& newProfile = *m_Profiles.emplace_back(profile.Clone());
			newProfile.SetID(newName);

			KxFile(newProfile.GetProfileDir()).CreateFolder();
			newProfile.SaveConfig();

			BroadcastProcessor::Get().ProcessEvent(ProfileEvent::EvtAdded, newProfile);
			return &newProfile;
		}
		return nullptr;
	}

	bool DefaultGameInstance::RemoveProfile(IGameProfile& profile)
	{
		bool isAllowed = BroadcastProcessor::Get().ProcessEventEx(ProfileEvent::EvtRemoving, profile).Do().IsAllowed();
		if (isAllowed && !profile.IsActive())
		{
			// Move files to recycle bin
			KxFile path(profile.GetProfileDir());
			path.RemoveFolderTree(true, true);
			path.RemoveFolder(true);

			// Remove it from profiles list
			ProfilesVector::const_iterator it;
			if (Util::FindObjectInVector<const IGameProfile, Util::FindBy::ProfileID>(m_Profiles, profile.GetID(), &it))
			{
				wxString id = profile.GetID();
				m_Profiles.erase(it);

				BroadcastProcessor::Get().ProcessEvent(ProfileEvent::EvtRemoved, id);
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
	void ConfigurableGameInstance::LoadConfigFile()
	{
		KxFileStream configStream(GetConfigFile(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
		m_Config.Load(configStream);
	}
	bool ConfigurableGameInstance::InitInstance()
	{
		if (m_WasCreatedUsingOnlyInstanceID)
		{
			// This instance were created using only instance ID, so load config file
			// as it's accessible using just instance ID, load game ID from there and
			// recreate itself with correct data. Actually that just assigns template
			// file path and 'is system' attribute.

			using namespace Application;

			LoadConfigFile();
			if (m_Config.IsOK())
			{
				const IGameInstance* templateInstance = GetTemplate(GetInstanceOption().GetAttribute(OName::GameID));
				if (templateInstance)
				{
					// Found a template for our game ID, query it for template definition.
					Create(templateInstance->GetDefinitionFile(), GetInstanceID(), templateInstance->IsSystemTemplate());
					return DefaultGameInstance::InitInstance();
				}
			}
			return false;
		}
		return DefaultGameInstance::InitInstance();
	}
	bool ConfigurableGameInstance::OnLoadInstance(const KxXMLDocument& templateConfig)
	{
		// Load config file if not loaded already
		if (!m_Config.IsOK())
		{
			LoadConfigFile();
		}

		// Load data
		LoadVariables(templateConfig, &m_Config);
		DetectGameArchitecture(templateConfig);
		LoadProfiles(templateConfig);

		return true;
	}

	ConfigurableGameInstance::ConfigurableGameInstance(const wxString& instanceID)
		:m_WasCreatedUsingOnlyInstanceID(true)
	{
		Create(wxString(), instanceID, false);
	}
	ConfigurableGameInstance::ConfigurableGameInstance(const IGameInstance& instanceTemplate, const wxString& instanceID)
	{
		Create(instanceTemplate.GetDefinitionFile(), instanceID, instanceTemplate.IsSystemTemplate());
		m_GameID = instanceTemplate.GetGameID();
	}

	void ConfigurableGameInstance::OnConfigChanged(IAppOption& option)
	{
		Utility::Log::LogInfo("ConfigurableGameInstance::OnConfigChanged -> %1", option.GetXPath());
	}
	void ConfigurableGameInstance::SaveConfig()
	{
		using namespace Application;

		// Save game ID
		GetInstanceOption().SetAttribute(OName::GameID, GetGameID());

		KxXMLNode variablesNode = GetInstanceOption(OName::Variables).GetNode();
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
				
				node.SetValue(value.AsString());
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
