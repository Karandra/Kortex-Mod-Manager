#pragma once
#include "stdafx.h"
#include "IGameInstance.h"
#include "Application/VariablesTable/VariablesDatabase.h"
#include "Application/VariablesTable/StaticVariableTable.h"
#include <Kortex/Events.hpp>
#include "GameID.h"
#include "KAux.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxFileStream.h>
class KActiveGameInstance;
class IVariableTable;
class IGameProfile;
class KxXMLDocument;

namespace Kortex::GameInstance
{
	class DefaultGameInstance: public RTTI::IImplementation<IGameInstance>
	{
		private:
			GameID m_GameID;
			wxString m_InstanceID;
			wxString m_TemplateFile;

			wxString m_Name;
			wxString m_NameShort;
			int m_SortOrder = -1;
			bool m_IsSystemTemplate = true;

			StaticVariableTable m_Variables;
			Vector m_Instances;
			ProfilesVector m_Profiles;

		private:
			void LoadInstancesList();

		protected:
			wxString CreateProfileID(const wxString& id) const override;
			wxString CreateDefaultProfileID() const override;

			bool OnLoadInstance(const KxXMLDocument& instanceConfig) override
			{
				return true;
			}
			bool ShouldInitProfiles() const override
			{
				return false;
			}
			bool InitInstance() override;

		protected:
			DefaultGameInstance() = default;
			void Create(const wxString& templateFile, const wxString& instanceID, bool isSystemTemplate)
			{
				m_TemplateFile = templateFile;
				m_InstanceID = instanceID;
				m_IsSystemTemplate = isSystemTemplate;
			}

		public:
			DefaultGameInstance(const wxString& templateFile, const wxString& instanceID, bool isSystemTemplate)
				:m_TemplateFile(templateFile), m_InstanceID(instanceID), m_IsSystemTemplate(isSystemTemplate)
			{
			}

		public:
			bool IsOK() const override
			{
				return m_GameID.IsOK();
			}
			bool IsTemplate() const override
			{
				return m_InstanceID.IsEmpty();
			}
			wxString GetTemplateFile() const override
			{
				return m_TemplateFile;
			}
		
			// Variables
			IVariableTable& GetVariables() override
			{
				return m_Variables;
			}
			const IVariableTable& GetVariables() const override
			{
				return m_Variables;
			}
			wxString ExpandVariablesLocally(const wxString& variables) const override;
			wxString ExpandVariables(const wxString& variables) const override;

			// Properties
			GameID GetGameID() const override
			{
				return m_GameID;
			}
			wxString GetInstanceID() const override
			{
				return m_InstanceID;
			}
			wxString GetName() const override
			{
				return m_Name.IsEmpty() ? m_NameShort : m_Name;
			}
			wxString GetShortName() const override
			{
				return m_NameShort.IsEmpty() ? m_Name : m_NameShort;
			}
		
			int GetSortOrder() const override
			{
				return m_SortOrder;
			}
			bool IsSystemTemplate() const override
			{
				return m_IsSystemTemplate;
			}
			bool IsActiveInstance() const override;

			wxString GetIconLocation() const override;
			wxBitmap GetIcon() const override;

			wxString GetInstanceTemplateDir() const override;
			wxString GetInstanceDir() const override;
			wxString GetInstanceRelativePath(const wxString& name) const override;

			wxString GetConfigFile() const override;
			wxString GetModsDir() const override;
			wxString GetProfilesDir() const override;
			wxString GetGameDir() const override;
			wxString GetVirtualGameDir() const override;

			// Instances
			const Vector& GetActiveInstances() const override
			{
				return m_Instances;
			}
			Vector& GetActiveInstances() override
			{
				return m_Instances;
			}
		
			const IGameInstance* GetInstance(const wxString& id) const override;
			IGameInstance* GetInstance(const wxString& id) override;

			const IGameInstance& GetTemplate() const override
			{
				// Shouldn't fail
				return *IGameInstance::GetTemplate(m_GameID);
			}
			IGameInstance& GetTemplate() override
			{
				// Shouldn't fail
				return *IGameInstance::GetTemplate(m_GameID);
			}

			IGameInstance* AddInstance(const wxString& instanceID) override;
			IGameInstance* AddInstanceToTemplate(const wxString& instanceID) override;

			bool Deploy(const IGameInstance* baseInstance = nullptr, uint32_t copyOptions = 0) override;
			bool IsDeployed() const override;
			bool WithdrawDeploy() override;

			// Profiles
			const ProfilesVector& GetProfiles() const override
			{
				return m_Profiles;
			}
			ProfilesVector& GetProfiles() override
			{
				return m_Profiles;
			}

			const IGameProfile* GetProfile(const wxString& id) const override;
			IGameProfile* GetProfile(const wxString& id) override;

			std::unique_ptr<IGameProfile> NewProfile() override;
			IGameProfile* CreateProfile(const wxString& profileID, const IGameProfile* baseProfile = nullptr, uint32_t copyOptions = 0) override;
			IGameProfile* ShallowCopyProfile(const IGameProfile& profile, const wxString& nameSuggets = wxEmptyString) override;
			bool RemoveProfile(IGameProfile& profile) override;
			bool RenameProfile(IGameProfile& profile, const wxString& newID) override;
			bool ChangeProfileTo(const IGameProfile& profile) override;
			void LoadSavedProfileOrDefault() override;
	};
}

namespace Kortex::GameInstance
{
	class ConfigurableGameInstance: public RTTI::IImplementation<DefaultGameInstance, IConfigurableGameInstance>
	{
		private:
			KxXMLDocument m_Config;

		protected:
			void LoadVariables(const KxXMLDocument& instanceConfig);
			void DetectGameArchitecture(const KxXMLDocument& instanceConfig);
			void LoadProfiles(const KxXMLDocument& instanceConfig);
			wxString LoadRegistryVariable(const KxXMLNode& node) const;

			bool OnLoadInstance(const KxXMLDocument& instanceConfig) override;

		public:
			ConfigurableGameInstance(const IGameInstance& instanceTemplate, const wxString& instanceID);

		public:
			const KxXMLDocument& GetConfig() const override
			{
				return m_Config;
			}
			KxXMLDocument& GetConfig() override
			{
				return m_Config;
			}
			
			void OnConfigChanged(IAppOption& option) override;
			void SaveConfig() override;
			void OnExit() override;
	};
}
