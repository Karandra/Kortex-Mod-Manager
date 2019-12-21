#pragma once
#include "stdafx.h"
#include "IGameInstance.h"
#include "Application/VariablesTable/VariablesDatabase.h"
#include "Application/VariablesTable/StaticVariableTable.h"
#include "GameID.h"
#include "Utility/KAux.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxFileStream.h>
class KActiveGameInstance;
class IVariableTable;
class IGameProfile;
class KxXMLDocument;

namespace Kortex::GameInstance
{
	class DefaultGameInstance: public KxRTTI::ExtendInterface<DefaultGameInstance, IGameInstance>
	{
		KxDecalreIID(DefaultGameInstance, {0x73a46d8d, 0x1a8b, 0x4adc, {0xa5, 0x50, 0xbb, 0xfd, 0x36, 0x2c, 0x31, 0x83}});

		friend class IGameInstance;

		protected:
			GameID m_GameID;
			wxString m_InstanceID;
			wxString m_DefinitionFile;

			wxString m_GameName;
			wxString m_GameShortName;
			int m_SortOrder = -1;
			bool m_IsSystemTemplate = true;

			StaticVariableTable m_Variables;
			ProfilesVector m_Profiles;

		protected:
			wxString CreateProfileID(const wxString& id) const override;
			wxString CreateDefaultProfileID() const override;

			void LoadVariables(const KxXMLDocument& instanceConfig, const KxXMLDocument* userConfig = nullptr);
			wxString LoadRegistryVariable(const KxXMLNode& node) const;
			void DetectGameArchitecture(const KxXMLDocument& instanceConfig);

			virtual bool OnLoadInstance(const KxXMLDocument& templateConfig)
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
			void Create(const wxString& definitionFile, const wxString& instanceID, bool isSystemTemplate)
			{
				m_DefinitionFile = definitionFile;
				m_InstanceID = instanceID;
				m_IsSystemTemplate = isSystemTemplate;
			}

		public:
			DefaultGameInstance(const wxString& templateFile, const wxString& instanceID, bool isSystemTemplate)
				:m_DefinitionFile(templateFile), m_InstanceID(instanceID), m_IsSystemTemplate(isSystemTemplate)
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
			wxString GetDefinitionFile() const override
			{
				return m_DefinitionFile;
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
			wxString GetGameName() const override
			{
				return m_GameName.IsEmpty() ? m_GameShortName : m_GameName;
			}
			wxString GetGameShortName() const override
			{
				return m_GameShortName.IsEmpty() ? m_GameName : m_GameShortName;
			}
		
			int GetSortOrder() const override
			{
				return m_SortOrder;
			}
			bool IsSystemTemplate() const override
			{
				return m_IsSystemTemplate;
			}
			
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
			bool ChangeProfileTo(IGameProfile& profile) override;
			void LoadSavedProfileOrDefault() override;
	};
}

namespace Kortex::GameInstance
{
	class ConfigurableGameInstance: public KxRTTI::ImplementInterface<ConfigurableGameInstance, DefaultGameInstance, IConfigurableGameInstance>
	{
		private:
			KxXMLDocument m_Config;
			const bool m_WasCreatedUsingOnlyInstanceID = false;

		protected:
			void LoadProfiles(const KxXMLDocument& instanceConfig);
			void LoadConfigFile();
			bool InitInstance() override;
			bool OnLoadInstance(const KxXMLDocument& templateConfig) override;

		public:
			ConfigurableGameInstance(const wxString& instanceID);
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
			
			void OnConfigChanged(AppOption& option) override;
			void SaveConfig() override;
			void OnExit() override;
	};
}
