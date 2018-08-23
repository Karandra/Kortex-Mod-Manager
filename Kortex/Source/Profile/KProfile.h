#pragma once
#include "stdafx.h"
#include "KVariablesDatabase.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxINI.h>
#include "KVariablesTable.h"
#include "KProfileID.h"
#include "KAux.h"
#include <KxFramework/KxFileStream.h>

class KLocationsManagerConfig;
class KConfigManagerConfig;
class KVirtualizationConfig;
class KPackageManagerConfig;
class KProgramManagerConfig;
class KPluginManagerConfig;
class KScreenshotsGalleryConfig;
class KSaveManagerConfig;
class KNetworkConfig;

wxDECLARE_EVENT(KEVT_UPDATE_PROFILES, wxNotifyEvent);

class KProfileAddConfig
{
	public:
		bool CopyProfileConfig = false;
		bool CopyGameConfig = false;
		bool CopyMods = false;
		bool CopyModTags = false;
		bool CopyRunManagerPrograms = false;
};

class KProfile
{
	private:
		typedef std::vector<KProfile*> ProfilesList;
		static ProfilesList ms_ProfilesList;

	public:
		static const wxString& GetTemplatesFolder();
		static const ProfilesList& GetTemplatesList();
		static const KProfile* GetProfileTemplate(const wxString& templateID);
		static bool IsTemplateExist(const wxString& templateID)
		{
			return GetProfileTemplate(templateID) != NULL;
		}
		static bool IsProfileIDValid(const wxString& configID);

		static KProfile* GetCurrent();

		// Data path for profiles. This is $(ProfilesFolder) variable
		static wxString GetDataPath();

		// Data path for profiles belongs to the specified template
		static wxString GetDataPath(const wxString& templateID);

		// Data path for real profile with config ID
		static wxString GetDataPath(const wxString& templateID, const wxString& configID);

		// Path to the config file for specified profile
		static wxString GetConfigFilePath(const wxString& templateID, const wxString& configID);

		// Gets or sets game root for specified profile. This needs to be static because game path can be queried/assigned before any full profile even loaded
		static wxString GetGameRootPath(const wxString& templateID, const wxString& configID);
		static bool SetGameRootPath(const wxString& templateID, const wxString& configID, const wxString& newPath);

	private:
		wxString m_ID;
		wxString m_Name;
		wxString m_NameShort;
		KVariablesTable m_Variables;
		long m_SortOrder = -1;

		wxString m_ConfigID;
		KxXMLDocument m_ProfileXML;
		KxINI m_ProfileConfig;
		KxStringVector m_ConfigsList;

		KLocationsManagerConfig* m_LocationsConfig = NULL;
		KConfigManagerConfig* m_GameConfig = NULL;
		KVirtualizationConfig* m_VirtualizationConfig = NULL;
		KPackageManagerConfig* m_PackageManagerConfig = NULL;
		KProgramManagerConfig* m_ProgramConfig = NULL;
		KPluginManagerConfig* m_PluginManagerConfig = NULL;
		KScreenshotsGalleryConfig* m_ScreenshotsGallery = NULL;
		KSaveManagerConfig* m_SaveManagerConfig = NULL;
		KNetworkConfig* m_NetworkConfig = NULL;

		KxFileStream m_ProfileFolderLock;

	private:
		void InitConfigsList();
		void CheckConfigFile();

		bool LoadGeneric(const wxString& sTemplatePath);
		void LoadConfig();
		void InitVariables();
		wxString LoadRegistryVariable(const KxXMLNode& node);
		void DetectArchitecture();
		template<class T> T* InitModuleConfig(const wxString& name, bool bAlwaysEnabled = false)
		{
			KxXMLNode node = m_ProfileXML.QueryElement("Profile/" + name);
			if (bAlwaysEnabled || (node.IsOK() && (node.GetAttributeBool("Enabled") || !node.HasAttribute("Enabled"))))
			{
				return new T(*this, node);
			}
			return NULL;
		}

	public:
		KProfile();
		KProfile(const wxString& sTemplatePath);
		void Create(const wxString& sTemplatePath, const wxString& configID);
		virtual ~KProfile();

	public:
		bool IsOK() const
		{
			return !m_ID.IsEmpty();
		}
		bool IsFullProfile() const
		{
			return !m_ConfigID.IsEmpty();
		}
		KxINI& GetConfig()
		{
			return m_ProfileConfig;
		}

		wxString GetID() const
		{
			return m_ID;
		}
		void SetID(const wxString& value)
		{
			m_ID = value;
		}
		
		wxString GetName() const
		{
			return KAux::StrOr(m_Name, m_NameShort);
		}
		void SetName(const wxString& value)
		{
			m_Name = value;
		}
		
		wxString GetShortName() const
		{
			return KAux::StrOr(m_NameShort, m_Name);
		}
		void SetShortName(const wxString& value)
		{
			m_NameShort = value;
		}
		
		int GetSortOrder() const
		{
			return m_SortOrder;
		}
		void SetSortOrder(int value)
		{
			m_SortOrder = value;
		}
		
		wxString GetTemplateFile() const
		{
			return m_Variables.GetVariable("ProfileTemplateFile");
		}
		wxString GetIconPath() const
		{
			return wxString::Format("%s\\Icons\\%s.ico", GetTemplatesFolder(), GetID());
		}
		wxImage GetIcon() const
		{
			return wxImage(GetIconPath(), wxBITMAP_TYPE_ICO);
		}
		wxString GetGameRoot() const
		{
			return m_Variables.GetVariable(KVAR_GAME_ROOT);
		}
		wxString GetVirtualRoot() const
		{
			return m_Variables.GetVariable(KVAR_VIRTUAL_GAME_ROOT);
		}

		bool IsVirtualModsEnabled() const
		{
			return true;
		}
		bool IsVirtualConfigEnabled() const
		{
			return true;
		}

		const KVariablesTable& GetVariables() const
		{
			return m_Variables;
		}
		KVariablesTable& GetVariables()
		{
			return m_Variables;
		}
		wxString ExpandVariables(const wxString& variables) const;

		const wxString& GetConfigID() const
		{
			return m_ConfigID;
		}
		bool HasConfig(const wxString& configID) const
		{
			return std::find(m_ConfigsList.cbegin(), m_ConfigsList.cend(), configID) != m_ConfigsList.cend();
		}
		const KxStringVector& GetConfigsList() const
		{
			return m_ConfigsList;
		}
		wxString GetConfigFilePath()
		{
			return GetConfigFilePath(m_ID, m_ConfigID);
		}

		bool RemoveConfig(const wxString& configID);
		bool AddConfig(const wxString& configID, const wxString& sBaseConfigID, wxWindow* parent, const KProfileAddConfig& tCopyConfig);

		// Get 'Relative to Current Profile Directory' path.
		// This concatenates strings in 'tElements' using '\' as separator.
		// Will result in call to 'GetDataPath(GetID(), GetConfigID())' if 'tElements' is empty.
		static wxString GetRCPD(const wxString& templateID, const wxString& configID, const KxStringVector& tElements);
		wxString GetRCPD(const KxStringVector& tElements) const;
};
