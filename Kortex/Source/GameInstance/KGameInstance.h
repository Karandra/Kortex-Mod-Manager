#pragma once
#include "stdafx.h"
#include "KVariablesDatabase.h"
#include "KStaticVariablesTable.h"
#include "KGameID.h"
#include "KEvents.h"
#include "KAux.h"
#include <KxFramework/KxINI.h>
#include <KxFramework/KxFileStream.h>
class KActiveGameInstance;
class KProfile;
class KxXMLDocument;

class KGameInstance
{
	public:
		using Vector = std::vector<std::unique_ptr<KGameInstance>>;
		using ProfilesVector = std::vector<std::unique_ptr<KProfile>>;
		using RefVector = std::vector<KGameInstance*>;
		using CRefVector = std::vector<const KGameInstance*>;

		enum CopyOptionsInstance: uint32_t
		{
			Config = 1 << 0,
			ModTags = 1 << 1,
			Programs = 1 << 2,
		};
		enum CopyOptionsProfile: uint32_t
		{
			GameConfig = 1 << 0,
			GameSaves = 1 << 1,
		};

	public:
		static bool IsValidInstanceID(const wxString& id);
		static bool IsValidProfileID(const wxString& id);

	public:
		static KGameInstance* CreateActive(const KGameInstance& instanceTemplate, const wxString& instanceID);

		static wxString GetTemplatesFolder();
		static wxString GetUserTemplatesFolder();
		static void LoadTemplates();

		static size_t GetTemplatesCount();
		static Vector& GetTemplates();
		static KGameInstance* GetTemplate(const KGameID& id);
		static bool HasTemplate(const KGameID& id);

	private:
		static void FindInstanceTemplates(const wxString& path, bool isSystem);

	public:
		static KActiveGameInstance* GetActive();
		static void AssignActive(KActiveGameInstance& instance);
		static void DestroyActive();

		static wxString GetCurrentProfileID();
		static KProfile* GetCurrentProfile();
		static bool IsCurrentProfileID(const wxString& id);

	private:
		KGameID m_GameID;
		wxString m_InstanceID;
		wxString m_TemplateFile;

		wxString m_Name;
		wxString m_NameShort;
		int m_SortOrder = -1;
		bool m_IsSystemTemplate = true;

		KStaticVariablesTable m_Variables;
		Vector m_Instances;
		ProfilesVector m_Profiles;

	private:
		void LoadInstancesList();
		bool InitInstance();

	protected:
		wxString CreateProfileID(const wxString& id) const;
		wxString CreateDefaultProfileID() const;

	protected:
		virtual bool OnLoadInstance(const KxXMLDocument& instanceConfig)
		{
			return true;
		}
		virtual bool ShouldInitProfiles() const
		{
			return false;
		}

	public:
		KGameInstance(const wxString& templateFile, const wxString& instanceID, bool isSystemTemplate);
		virtual ~KGameInstance();

	public:
		bool IsOK() const
		{
			return m_GameID.IsOK();
		}
		bool IsTemplate() const
		{
			return m_InstanceID.IsEmpty();
		}
		wxString GetTemplateFile() const
		{
			return m_TemplateFile;
		}
		
		// Variables
		KIVariablesTable& GetVariables()
		{
			return m_Variables;
		}
		const KIVariablesTable& GetVariables() const
		{
			return m_Variables;
		}
		wxString ExpandVariablesLocally(const wxString& variables) const;
		wxString ExpandVariables(const wxString& variables) const;

		// Properties
		KGameID GetGameID() const
		{
			return m_GameID;
		}
		wxString GetInstanceID() const
		{
			return m_InstanceID;
		}
		wxString GetName() const
		{
			return m_Name.IsEmpty() ? m_NameShort : m_Name;
		}
		wxString GetShortName() const
		{
			return m_NameShort.IsEmpty() ? m_Name : m_NameShort;
		}
		
		int GetSortOrder() const
		{
			return m_SortOrder;
		}
		bool IsSystemTemplate() const
		{
			return m_IsSystemTemplate;
		}
		bool IsActiveInstance() const;

		wxString GetIconLocation() const;
		wxBitmap GetIcon() const;

		wxString GetInstanceTemplateDir() const;
		wxString GetInstanceDir() const;
		wxString GetInstanceRelativePath(const wxString& name) const;

		wxString GetConfigFile() const;
		wxString GetModTagsFile() const;
		wxString GetProgramsFile() const;
		wxString GetModsDir() const;
		wxString GetProfilesDir() const;
		wxString GetGameDir() const;
		wxString GetVirtualGameDir() const;

		// Instances
		bool HasInstances() const
		{
			return !m_Instances.empty();
		}
		size_t GetInstancesCount() const
		{
			return m_Instances.size();
		}
		const Vector& GetActiveInstances() const
		{
			return m_Instances;
		}
		Vector& GetActiveInstances()
		{
			return m_Instances;
		}
		
		bool HasInstance(const wxString& id) const
		{
			return GetInstance(id) != NULL;
		}
		const KGameInstance* GetInstance(const wxString& id) const;
		KGameInstance* GetInstance(const wxString& id);

		const KGameInstance& GetTemplate() const
		{
			// Shouldn't fail
			return *KGameInstance::GetTemplate(m_GameID);
		}
		KGameInstance& GetTemplate()
		{
			// Shouldn't fail
			return *KGameInstance::GetTemplate(m_GameID);
		}

		KGameInstance* AddInstance(const wxString& instanceID);
		KGameInstance* AddInstanceToTemplate(const wxString& instanceID);

		bool Deploy(const KGameInstance* baseInstance = NULL, uint32_t copyOptions = 0);
		bool IsDeployed() const;
		bool WithdrawDeploy();

		// Profiles
		bool HasProfiles() const
		{
			return !m_Profiles.empty();
		}
		size_t GetProfilesCount() const
		{
			return m_Profiles.size();
		}
		const ProfilesVector& GetProfiles() const
		{
			return m_Profiles;
		}
		ProfilesVector& GetProfiles()
		{
			return m_Profiles;
		}

		bool HasProfile(const wxString& id) const
		{
			return GetProfile(id) != NULL;
		}
		const KProfile* GetProfile(const wxString& id) const;
		KProfile* GetProfile(const wxString& id);

		KProfile* CreateProfile(const wxString& profileID, const KProfile* baseProfile = NULL, uint32_t copyOptions = 0);
		KProfile* ShallowCopyProfile(const KProfile& profile, const wxString& nameSuggets = wxEmptyString);
		bool RemoveProfile(KProfile& profile);
		bool RenameProfile(KProfile& profile, const wxString& newID);
		bool ChangeProfileTo(const KProfile& profile);
		void LoadSavedProfileOrDefault();

		// Config
		virtual bool SaveConfig()
		{
			return false;
		}
};

//////////////////////////////////////////////////////////////////////////
class KConfigurableGameInstance: public KGameInstance
{
	private:
		KxINI m_Config;

	protected:
		void LoadVariables(const KxXMLDocument& instanceConfig);
		void DetectGameArchitecture(const KxXMLDocument& instanceConfig);
		void LoadProfiles(const KxXMLDocument& instanceConfig);
		wxString LoadRegistryVariable(const KxXMLNode& node) const;

	protected:
		virtual bool OnLoadInstance(const KxXMLDocument& instanceConfig) override;

	public:
		KConfigurableGameInstance(const KGameInstance& instanceTemplate, const wxString& instanceID);
		virtual ~KConfigurableGameInstance();

	public:
		const KxINI& GetConfig() const
		{
			return m_Config;
		}
		KxINI& GetConfig()
		{
			return m_Config;
		}
		
		virtual bool SaveConfig() override;
};
