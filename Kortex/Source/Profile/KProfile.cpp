#include "stdafx.h"
#include "KVariablesDatabase.h"
#include "KQuickThread.h"
#include "KOperationWithProgress.h"
#include "KProfile.h"
#include "SettingsWindow/KSettingsWindowManager.h"
#include "ModManager/KModManager.h"
#include "Events/KLogEvent.h"
#include "KApp.h"
#include "KAux.h"

#include "KConfigManagerConfig.h"
#include "KVirtualizationConfig.h"
#include "KPackageManagerConfig.h"
#include "KProgramManagerConfig.h"
#include "KPluginManagerConfig.h"
#include "KScreenshotsGalleryConfig.h"
#include "KSaveManagerConfig.h"
#include "KLocationsManagerConfig.h"
#include "KNetworkConfig.h"

#include <KxFramework/KxFile.h>
#include <KxFramework/KxINI.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxRegistry.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxFileOperationEvent.h>
#include <KxFramework/KxDualProgressDialog.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxShell.h>

wxDEFINE_EVENT(KEVT_UPDATE_PROFILES, wxNotifyEvent);

KProfile::ProfilesList KProfile::ms_ProfilesList;

const wxString& KProfile::GetTemplatesFolder()
{
	static const wxString ms_TemplatesFolder = KApp::Get().GetDataFolder() + "\\Profile Templates";
	return ms_TemplatesFolder;
}
const KProfile::ProfilesList& KProfile::GetTemplatesList()
{
	if (ms_ProfilesList.empty())
	{
		auto files = KxFile(GetTemplatesFolder()).Find("*.xml", KxFS_FILE, false);
		ms_ProfilesList.reserve(files.size());
		for (const wxString& path: files)
		{
			ms_ProfilesList.push_back(new KProfile(path));
		}

		// Sort by sort order value
		std::sort(ms_ProfilesList.begin(), ms_ProfilesList.end(), [](const KProfile* v1, const KProfile* v2)
		{
			return v1->GetSortOrder() < v2->GetSortOrder();
		});
	}
	return ms_ProfilesList;
}
const KProfile* KProfile::GetProfileTemplate(const wxString& templateID)
{
	auto tElement = std::find_if(GetTemplatesList().cbegin(), GetTemplatesList().cend(), [&templateID](const KProfile* v)
	{
		return v->GetID() == templateID;
	});

	if (tElement != GetTemplatesList().cend())
	{
		return *tElement;
	}
	return NULL;
}
bool KProfile::IsProfileIDValid(const wxString& configID)
{
	// Restrict max ID length to 128 symbols
	if (!configID.IsEmpty() && configID.Length() <= 128)
	{
		return !KAux::HasForbiddenFileNameChars(configID);
	}
	return false;
}

KProfile* KProfile::GetCurrent()
{
	return KApp::Get().GetCurrentProfile();
}

wxString KProfile::GetDataPath()
{
	return V(KVAR(KVAR_PROFILES_ROOT));
}
wxString KProfile::GetDataPath(const wxString& templateID)
{
	return wxString::Format("%s\\%s", GetDataPath(), templateID);
}
wxString KProfile::GetDataPath(const wxString& templateID, const wxString& configID)
{
	return wxString::Format("%s\\%s", GetDataPath(templateID), configID);
}
wxString KProfile::GetConfigFilePath(const wxString& templateID, const wxString& configID)
{
	return wxString::Format("%s\\%s", GetDataPath(templateID, configID), "ProfileConfig.ini");
}
wxString KProfile::GetGameRootPath(const wxString& templateID, const wxString& configID)
{
	KxFileStream file(GetConfigFilePath(templateID, configID), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	if (file.IsOk())
	{
		KxINI ini(file);
		return ini.GetValue("KProfile::VariablesOverride", KVAR_GAME_ROOT);
	}
	return wxEmptyString;
}
bool KProfile::SetGameRootPath(const wxString& templateID, const wxString& configID, const wxString& newPath)
{
	KxFileStream file(GetConfigFilePath(templateID, configID), KxFS_ACCESS_RW, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	if (file.IsOk())
	{
		KxINI ini(file);
		if (ini.SetValue("KProfile::VariablesOverride", "GameRoot", newPath))
		{
			file.Seek(0, KxFS_SEEK_BEGIN);
			return ini.Save(file);
		}
	}
	return false;
}

void KProfile::InitConfigsList()
{
	if (IsOK())
	{
		m_ConfigsList.clear();

		wxString root = GetDataPath(GetID());
		auto folders = KxFile(root).Find(KxFile::NullFilter, KxFS_FOLDER, false);
		for (const wxString& path: folders)
		{
			m_ConfigsList.push_back(KxFile(path).GetFullName());
		}
	}
}
void KProfile::CheckConfigFile()
{
	// If this profile is new, or it's config file somehow get deleted
	// copy template config file in its place
	KxFile sConfigFile(KProfile::GetConfigFilePath(GetID(), m_ConfigID));
	if (!sConfigFile.IsFileExist())
	{
		KLogEvent(TF("Init.Error2").arg(m_ConfigID), KLOG_INFO).Send();
		KxFile(KApp::Get().GetDataFolder() + "\\ProfileConfigTemplate.ini").CopyFile(sConfigFile, true);
	}
}

bool KProfile::LoadGeneric(const wxString& templatePath)
{
	m_Variables.SetVariable("ProfileTemplateFile", templatePath);

	// Load template XML
	KxFileStream xmlStream(templatePath);
	m_ProfileXML.Load(xmlStream);

	// Load ID and SortOrder
	KxXMLNode node = m_ProfileXML.QueryElement("Profile");
	m_ID = node.GetAttribute("ID");
	if (!m_ID.IsEmpty())
	{
		node.GetAttribute("SortOrder").ToCLong(&m_SortOrder);

		// Load name
		node = node.QueryElement("Name");
		m_Name = node.GetValue();
		m_NameShort = node.GetAttribute("Short");

		// Load config file
		if (IsFullProfile())
		{
			KxFileStream tConfigStream(GetConfigFilePath(m_ID, m_ConfigID));
			m_ProfileConfig.Load(tConfigStream);
		}

		// Set variables
		m_Variables.SetVariable("ID", m_ID);
		m_Variables.SetVariable("Name", KAux::StrOr(m_Name, m_NameShort, m_ID));
		m_Variables.SetVariable("NameShort", KAux::StrOr(m_NameShort, m_Name, m_ID));
		m_Variables.SetVariable("SortOrder", std::to_string(m_SortOrder));
		InitVariables();

		// Requires valid ID so it need to be loaded after ID is initialized
		InitConfigsList();

		return true;
	}
	return false;
}
void KProfile::LoadConfig()
{
	CheckConfigFile();
	DetectArchitecture();

	// Always enabled
	m_GameConfig = InitModuleConfig<KConfigManagerConfig>("GameConfig", true);

	// Init current profile config. It's depend on 'KConfigManagerConfig'.
	KApp::Get().GetSettingsManager()->InitCurrentProfileConfig();

	m_LocationsConfig = InitModuleConfig<KLocationsManagerConfig>("LocationsConfig", true);
	m_VirtualizationConfig = InitModuleConfig<KVirtualizationConfig>("Virtualization", true);
	m_PackageManagerConfig = InitModuleConfig<KPackageManagerConfig>("PackageManager", true);
	m_NetworkConfig = InitModuleConfig<KNetworkConfig>("Network", true);

	// Can be disabled
	m_ProgramConfig = InitModuleConfig<KProgramManagerConfig>("ProgramConfig");
	m_PluginManagerConfig = InitModuleConfig<KPluginManagerConfig>("PluginManager");
	m_ScreenshotsGallery = InitModuleConfig<KScreenshotsGalleryConfig>("ScreenshotsGallery");
	m_SaveManagerConfig = InitModuleConfig<KSaveManagerConfig>("SaveManager");
}
void KProfile::InitVariables()
{
	// System variables
	if (IsFullProfile())
	{
		m_Variables.SetVariable(KVAR_PROFILE_ROOT, GetRCPD({}));
		m_Variables.SetVariable(KVAR_VIRTUAL_GAME_ROOT, GetRCPD({"VirtualGameRoot"}));
		m_Variables.SetVariable(KVAR_CONFIG_ROOT_LOCAL, GetRCPD({"VirtualGameConfig"}));
		m_Variables.SetVariable(KVAR_MODS_ROOT, GetRCPD({"Mods"}));
	}

	KxXMLNode node = m_ProfileXML.QueryElement("Profile/Variables");
	for (node = node.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
	{
		wxString id = node.GetAttribute("ID");
		wxString sEntryType = node.GetAttribute("EntryType");
		wxString sSubType = node.GetAttribute("SubType");
		wxString value;
		if (sEntryType == "Registry")
		{
			value = LoadRegistryVariable(node);
		}
		else
		{
			value = ExpandVariables(node.GetValue());
		}

		if (sSubType == "FolderPath")
		{
			value = KxFile(value).GetFullPath();
		}

		if (IsFullProfile())
		{
			// Load variable value from overrides if it's present there
			// or save variable value to overrides if this variable needs to be overridden
			wxString overrideValue = m_ProfileConfig.GetValue("KProfile::VariablesOverride", id);
			if (node.GetAttributeBool("UseOverride") && overrideValue.IsEmpty())
			{
				m_ProfileConfig.SetValue("KProfile::VariablesOverride", id, value);
			}
			if (!overrideValue.IsEmpty())
			{
				value = overrideValue;
			}
		}
		m_Variables.SetVariable(id, value);
	}

	if (IsFullProfile())
	{
		KxFileStream file(GetConfigFilePath(), KxFS_ACCESS_WRITE, KxFS_DISP_CREATE_ALWAYS, KxFS_SHARE_READ);
		m_ProfileConfig.Save(file);
	}
}
wxString KProfile::LoadRegistryVariable(const KxXMLNode& node)
{
	static const std::unordered_map<wxString, KxRegistryHKey> ms_NameToRegKey =
	{
		std::make_pair("HKEY_CLASSES_ROOT", KxREG_HKEY_CLASSES_ROOT),
		std::make_pair("HKEY_CURRENT_USER", KxREG_HKEY_CURRENT_USER),
		std::make_pair("HKEY_LOCAL_MACHINE", KxREG_HKEY_LOCAL_MACHINE),
		std::make_pair("HKEY_USERS", KxREG_HKEY_USERS),
		std::make_pair("HKEY_CURRENT_CONFIG", KxREG_HKEY_CURRENT_CONFIG)
	};

	static const std::unordered_map<wxString, KxRegistryValueType> ms_NameToRegType =
	{
		std::make_pair("REG_VALUE_SZ", KxREG_VALUE_SZ),
		std::make_pair("REG_VALUE_EXPAND_SZ", KxREG_VALUE_EXPAND_SZ),
		std::make_pair("REG_VALUE_MULTI_SZ", KxREG_VALUE_MULTI_SZ),
		std::make_pair("REG_VALUE_DWORD", KxREG_VALUE_DWORD),
		std::make_pair("REG_VALUE_QWORD", KxREG_VALUE_QWORD)
	};

	// 32 or 64 bit registry branch
	long nodeValue = 0;
	KxRegistryNode regNode = KxREG_NODE_SYS;
	node.GetFirstChildElement("Node").GetValue().ToLong(&nodeValue);
	if (nodeValue == 32)
	{
		regNode = KxREG_NODE_32;
	}
	else if (nodeValue == 64)
	{
		regNode = KxREG_NODE_64;
	}

	// Main key
	KxRegistryHKey mainKey = KxREG_HKEY_MAX;
	wxString root = node.GetFirstChildElement("Root").GetValue();
	if (ms_NameToRegKey.count(root))
	{
		mainKey = ms_NameToRegKey.at(root);
	}

	wxString path = ExpandVariables(node.GetFirstChildElement("Path").GetValue());
	wxString valueName = ExpandVariables(node.GetFirstChildElement("ValueName").GetValue());
	
	KxRegistryValueType type = KxREG_VALUE_ANY;
	wxString sValueType = node.GetFirstChildElement("ValueType").GetValue();
	if (ms_NameToRegType.count(sValueType))
	{
		type = ms_NameToRegType.at(sValueType);
	}

	wxAny data = KxRegistry::GetValue(mainKey, path, valueName, type, regNode, true);
	if (type == KxREG_VALUE_DWORD || type == KxREG_VALUE_QWORD)
	{
		return std::to_string(data.As<int64_t>());
	}
	else
	{
		return data.As<wxString>();
	}
}
void KProfile::DetectArchitecture()
{
	KxFileBinaryFormat type = KxFile(m_Variables.GetVariable("GameExecutable")).GetBinaryType();
	bool is32Bit = type != KxFBF_WIN64;

	m_Variables.SetVariable("Architecture", KAux::ArchitectureToNumber(is32Bit));
	m_Variables.SetVariable("ArchitectureName", KAux::ArchitectureToString(is32Bit));
}

KProfile::KProfile()
{
}
KProfile::KProfile(const wxString& templatePath)
{
	LoadGeneric(templatePath);
	m_ProfileXML.Load(wxEmptyString);
}
void KProfile::Create(const wxString& templatePath, const wxString& configID)
{
	m_ConfigID = configID;

	if (LoadGeneric(templatePath))
	{
		// Lock this folder
		if (IsFullProfile())
		{
			m_ProfileFolderLock.Open(GetDataPath(m_ID, m_ConfigID), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ|KxFS_SHARE_WRITE, KxFS_FLAG_BACKUP_SEMANTICS);
		}
		LoadConfig();
	}
	m_ProfileXML.Load(wxEmptyString);
}
KProfile::~KProfile()
{
	delete m_LocationsConfig;
	delete m_VirtualizationConfig;
	delete m_GameConfig;
	delete m_PackageManagerConfig;
	delete m_NetworkConfig;

	delete m_ProgramConfig;
	delete m_PluginManagerConfig;
	delete m_ScreenshotsGallery;
	delete m_SaveManagerConfig;
}
wxBitmap KProfile::GetIcon() const
{
	wxBitmap icon(GetIconPath(), wxBITMAP_TYPE_ANY);
	if (!icon.IsOk())
	{
		if (KProgramManagerConfig* programsConfig = KProgramManagerConfig::GetInstance())
		{
			const KProgramManagerEntry* programEntry = programsConfig->GetEntryAt(KProgramManagerConfig::ProgramType::Main, 0);
			if (programEntry)
			{
				if (!programEntry->GetIconPath().IsEmpty())
				{
					icon = KxShell::GetFileIcon(KModManager::GetDispatcher().ResolveLocationPath(programEntry->GetIconPath()));
				}
				else
				{
					icon = KxShell::GetFileIcon(KModManager::GetDispatcher().ResolveLocationPath(programEntry->GetExecutable()));
				}
			}
		}
	}
	return icon;
}

wxString KProfile::ExpandVariables(const wxString& variables) const
{
	return KApp::Get().ExpandVariablesLocally(ExpandVariablesLocally(variables));
}

bool KProfile::RemoveConfig(const wxString& configID)
{
	if (HasConfig(configID))
	{
		KxFile path(GetDataPath(GetID(), configID));

		path.RemoveFolderTree(true);
		path.RemoveFolder(true);

		m_ConfigsList.erase(std::remove_if(m_ConfigsList.begin(), m_ConfigsList.end(), [configID](const wxString& id)
		{
			return id == configID;
		}), m_ConfigsList.end());
	}
	return false;
}
bool KProfile::AddConfig(const wxString& configID, const wxString& sBaseConfigID, wxWindow* parent, const KProfileAddConfig& tCopyConfig)
{
	if (!HasConfig(configID))
	{
		wxString sMainFolder = GetDataPath();
		wxString sModsFolder = GetRCPD(GetID(), configID, {"Mods"});
		wxString sConfigFolder = GetRCPD(GetID(), configID, {"VirtualGameConfig"});

		auto CreateFolders = [this, configID, &sMainFolder, &sModsFolder, &sConfigFolder]()
		{
			KxFile(sMainFolder).CreateFolder();
			KxFile(sModsFolder).CreateFolder();
			KxFile(sConfigFolder).CreateFolder();
		};
		
		// Create all required folders
		CreateFolders();

		// Copy template config file or source profile config
		if (tCopyConfig.CopyProfileConfig)
		{
			KxFile(KProfile::GetConfigFilePath(GetID(), sBaseConfigID)).CopyFile(KProfile::GetConfigFilePath(GetID(), configID), true);
		}
		else
		{
			KxFile(KApp::Get().GetDataFolder() + "\\ProfileConfigTemplate.ini").CopyFile(KProfile::GetConfigFilePath(GetID(), configID), true);
		}
		

		// Do copy
		KOperationWithProgressDialogBase* operation = new KOperationWithProgressDialog<KxFileOperationEvent>(true, parent);
		operation->OnRun([this, configID, sBaseConfigID, sModsFolder, sConfigFolder, tCopyConfig, operation](KOperationWithProgressBase* self)
		{
			// Begin copying data
			if (!sBaseConfigID.IsEmpty())
			{
				if (tCopyConfig.CopyGameConfig)
				{
					KxEvtFile source(GetRCPD(GetID(), sBaseConfigID, {"VirtualGameConfig"}));
					self->LinkHandler(&source, KxEVT_FILEOP_COPY_FOLDER);
					source.CopyFolder(KxFile::NullFilter, sConfigFolder, true, true);
				}

				if (tCopyConfig.CopyMods)
				{
					KxEvtFile source(GetRCPD(GetID(), sBaseConfigID, {"Mods"}));
					self->LinkHandler(&source, KxEVT_FILEOP_COPY_FOLDER);
					source.CopyFolder(KxFile::NullFilter, sModsFolder, true, true);
				}

				if (tCopyConfig.CopyModTags)
				{
					KxEvtFile source(KModManager::GetTagManager().GetUserTagsFile(GetID(), sBaseConfigID));
					self->LinkHandler(&source, KxEVT_FILEOP_COPY);
					source.CopyFile(KModManager::GetTagManager().GetUserTagsFile(GetID(), configID), true);
				}

				if (tCopyConfig.CopyRunManagerPrograms)
				{
					KxEvtFile source(KProgramManager::GetProgramsListFile(GetID(), sBaseConfigID));
					self->LinkHandler(&source, KxEVT_FILEOP_COPY);
					source.CopyFile(KProgramManager::GetProgramsListFile(GetID(), configID), true);
				}
			}

			// Ask user to copy game config to profile if we aren't copying config from another profile
			if (IsVirtualConfigEnabled() && !tCopyConfig.CopyGameConfig)
			{
				KxEvtFile source(m_Variables.GetVariable(KVAR_CONFIG_ROOT_TARGET));
				if (source.IsFolderExist() && !KxFileFinder::IsDirectoryEmpty(source.GetFullPath()))
				{
					// Copy destination link
					wxString messageLink = wxString::Format("<a href=\"%s\">%s</a>", source.GetFullPath(), T("ProfileCreatorDialog.CopyNonVirtualGameConfigMessage2"));

					KxTaskDialog msg
					(
						operation->GetDialog(),
						KxID_NONE,
						T("ProfileCreatorDialog.CopyNonVirtualGameConfigCaption"),
						TF("ProfileCreatorDialog.CopyNonVirtualGameConfigMessage").arg(GetShortName()).arg(messageLink).arg(source.GetFullPath()),
						KxBTN_CANCEL
					);
					msg.SetOptionEnabled(KxTD_HYPERLINKS_ENABLED);
					msg.AddButton(KxID_COPY);
					msg.Bind(wxEVT_TEXT_URL, [&msg](const wxTextUrlEvent& event)
					{
						KxShell::Execute(&msg, event.GetString(), "open");
					});

					if (msg.ShowModal() == KxID_COPY)
					{
						self->LinkHandler(&source, KxEVT_FILEOP_COPY_FOLDER);
						source.CopyFolder(KxFile::NullFilter, sConfigFolder, true, true);
						source.Rename(wxString::Format("%s (Backup %s)", source.GetFullPath(), KAux::FormatDateTimeFS(wxDateTime::Now())), false);
					}
				}
			}
		});

		// If canceled, remove added profile
		operation->OnCancel([this, configID](KOperationWithProgressBase* self)
		{
			RemoveConfig(configID);
		});

		// Reload after task is completed (successfully or not)
		operation->OnEnd([this, parent, configID](KOperationWithProgressBase* self)
		{
			m_ConfigsList.push_back(configID);

			wxNotifyEvent event(KEVT_UPDATE_PROFILES);
			parent->HandleWindowEvent(event);
		});

		// Configure and run
		operation->SetDialogCaption(T("ProfileCreatorDialog.CopyingProfileData"));
		operation->Run();
		return true;
	}
	return false;
}

wxString KProfile::GetRCPD(const wxString& templateID, const wxString& configID, const KxStringVector& elements)
{
	wxString path = GetDataPath(templateID, configID);
	if (!elements.empty())
	{
		path.Append('\\');
		path.Append(KxString::Join(elements, "\\"));
	}
	return path;
}
wxString KProfile::GetRCPD(const KxStringVector& elements) const
{
	return GetRCPD(GetID(), GetConfigID(), elements);
}
