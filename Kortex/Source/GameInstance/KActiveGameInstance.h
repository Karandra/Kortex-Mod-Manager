#pragma once
#include "stdafx.h"
#include "KGameInstance.h"
#include "KProgramOptions.h"
class KProfile;

class KLocationsManagerConfig;
class KConfigManagerConfig;
class KVirtualizationConfig;
class KPackageManagerConfig;
class KProgramManagerConfig;
class KPluginManagerConfig;
class KScreenshotsGalleryConfig;
class KSaveManagerConfig;
class KNetworkConfig;

class KActiveGameInstance: public KConfigurableGameInstance
{
	private:
		KxFileStream m_DirectoryLock;
		wxString m_CurrentProfileID;
		
		std::unique_ptr<KLocationsManagerConfig> m_LocationsConfig;
		std::unique_ptr<KConfigManagerConfig> m_GameConfig;
		std::unique_ptr<KVirtualizationConfig> m_VirtualizationConfig;
		std::unique_ptr<KPackageManagerConfig> m_PackageManagerConfig;
		std::unique_ptr<KProgramManagerConfig> m_ProgramConfig;
		std::unique_ptr<KPluginManagerConfig> m_PluginManagerConfig;
		std::unique_ptr<KScreenshotsGalleryConfig> m_ScreenshotsGallery;
		std::unique_ptr<KSaveManagerConfig> m_SaveManagerConfig;
		std::unique_ptr<KNetworkConfig> m_NetworkConfig;

	protected:
		void InitModulesConfig(const KxXMLDocument& instanceConfig);
		void InitVariables(const KProfile& profile);
		
		virtual bool OnLoadInstance(const KxXMLDocument& instanceConfig) override;
		virtual bool ShouldInitProfiles() const override;

	public:
		KActiveGameInstance(const KGameInstance& instanceTemplate, const wxString& instanceID);
		virtual ~KActiveGameInstance();

	public:
		const wxString& GetActiveProfileID() const;
		void SetCurrentProfileID(const wxString& id);

		const KProfile* GetActiveProfile() const
		{
			return GetProfile(m_CurrentProfileID);
		}
		KProfile* GetActiveProfile()
		{
			return GetProfile(m_CurrentProfileID);
		}

		void ChangeProfileTo(const KProfile& profile);
		void LoadSavedProfileOrDefault();
};
