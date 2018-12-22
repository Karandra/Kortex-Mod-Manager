#pragma once
#include "stdafx.h"
#include "IApplication.h"
#include "Application/VariablesTable/DynamicVariableTable.h"
#include "KImageProvider.h"
#include <KxFramework/KxApp.h>
#include <KxFramework/KxImageList.h>
#include <KxFramework/KxImageSet.h>
class KIPCClient;
class KVFSService;

namespace Kortex
{
	class GameModsModule;
	class KPackageModule;
	class KProgramModule;
	class NetworkModule;
}

namespace Kortex::Application
{
	class DefaultApplication: public IApplication
	{
		private:
			KxTranslation m_Translation;
			KxStringToStringUMap m_AvailableTranslations;
			DynamicVariableTable m_Variables;

			wxString m_RootFolder;
			wxString m_DataFolder;
			wxString m_LogsFolder;
			wxString m_UserSettingsFolder;
			wxString m_UserSettingsFile;
			wxString m_InstancesFolder;

			wxWindow* m_InitProgressDialog = nullptr;
			KxImageList m_ImageList;
			KxImageSet m_ImageSet;
		
			KIPCClient* m_VFSServiceClient = nullptr;
			KVFSService* m_VFSService = nullptr;

			GameID m_StartupGameID;
			wxString m_StartupInstanceID;
			bool m_IsCmdStartupGameID = false;
			bool m_IsCmdStartupInstanceID = false;

			std::unique_ptr<GameModsModule> m_GameModsModule;
			std::unique_ptr<KPackageModule> m_PackagesModule;
			std::unique_ptr<KProgramModule> m_ProgramModule;
			std::unique_ptr<NetworkModule> m_NetworkModule;

		public:
			DefaultApplication();

		protected:
			void OnCreate() override;
			void OnDestroy() override;
			bool OnInit() override;
			int OnExit() override;

			void OnError(LogEvent& event) override;
			bool OnException() override;

			bool OnGlobalConfigChanged(IAppOption& option) override;
			bool OnInstanceConfigChanged(IAppOption& option, IGameInstance& instance) override;
			bool OnProfileConfigChanged(IAppOption& option, IGameProfile& profile) override;

		public:
			wxString GetRootFolder() const override
			{
				return m_RootFolder;
			}
			wxString GetDataFolder() const override
			{
				return m_DataFolder;
			}
			wxString GetLogsFolder() const override
			{
				return m_LogsFolder;
			}
			wxString GetUserSettingsFolder() const override
			{
				return m_UserSettingsFolder;
			}
			wxString GetUserSettingsFile() const override
			{
				return m_UserSettingsFile;
			}
			wxString GetInstancesFolder() const override
			{
				return m_InstancesFolder;
			}

			GameID GetStartupGameID() const override
			{
				return m_StartupGameID;
			}
			wxString GetStartupInstanceID() const override
			{
				return m_StartupInstanceID;
			}

			const KxImageList& GetImageList() const override
			{
				return m_ImageList;
			}
			const KxImageSet& GetImageSet() const override
			{
				return m_ImageSet;
			}

			IVariableTable& GetVariables() override
			{
				return m_Variables;
			}
			wxString ExpandVariablesLocally(const wxString& variables) const override;
			wxString ExpandVariables(const wxString& variables) const override;
		
			bool IsTranslationLoaded() const override
			{
				return m_Translation.IsOK();
			}
			const KxTranslation& GetTranslation() const override
			{
				return m_Translation;
			}
			KxTranslation::AvailableMap GetAvailableTranslations() const override
			{
				return m_AvailableTranslations;
			}

			bool ScheduleRestart() override;
			bool Uninstall() override;

			void LoadTranslation();
			void LoadImages();
			void ShowWorkspace();
			bool ShowChageInstanceDialog();

			void InitSettings();
			bool IsPreStartConfigNeeded();
			bool ShowFirstTimeConfigDialog(wxWindow* parent);
			void InitInstancesData(wxWindow* parent);
			bool LoadInstance();
			void LoadStartupGameIDAndInstanceID();

			void InitVFS();
			void UnInitVFS();
	};
}
