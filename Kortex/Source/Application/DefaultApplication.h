#pragma once
#include "stdafx.h"
#include "IApplication.h"
#include "RefTranslator.h"
#include "Resources/DefaultImageProvider.h"
#include "VariablesTable/DynamicVariableTable.h"
#include <KxFramework/KxApp.h>
#include <KxFramework/KxImageList.h>
#include <KxFramework/KxImageSet.h>
#include <KxFramework/KxDPIAwareness.h>

namespace Kortex
{
	class IVFSService;

	class GameModsModule;
	class ModPackagesModule;
	class KProgramModule;
	class NetworkModule;
}

namespace Kortex::Application
{
	class MainWindow;

	class DefaultApplication: public IApplication
	{
		private:
			KxTranslation m_Translation;
			RefTranslator m_Translator;
			KxStringToStringUMap m_AvailableTranslations;
			DynamicVariableTable m_Variables;

			wxString m_DataFolder;
			wxString m_LogsFolder;
			wxString m_UserSettingsFolder;
			wxString m_UserSettingsFile;
			wxString m_InstancesFolder;
			wxString m_DefaultInstancesFolder;

			wxWindow* m_InitProgressDialog = nullptr;
			MainWindow* m_MainWindow = nullptr;
			DefaultImageProvider m_ImageProvider;
			BroadcastReciever m_BroadcastReciever;

			wxString m_StartupInstanceID;
			bool m_IsCmdStartupInstanceID = false;

			std::unique_ptr<GameModsModule> m_GameModsModule;
			std::unique_ptr<ModPackagesModule> m_PackagesModule;
			std::unique_ptr<KProgramModule> m_ProgramModule;
			std::unique_ptr<NetworkModule> m_NetworkModule;
			std::unique_ptr<IVFSService> m_VFSService;

			std::unique_ptr<KxDPIAwareness::ThreadContextChanger> m_ThreadDPIContext;

		public:
			DefaultApplication();

		protected:
			void OnCreate() override;
			void OnDestroy() override;
			bool OnInit() override;
			int OnExit() override;

			void OnError(LogEvent& event) override;
			bool OnException() override;

			void OnGlobalConfigChanged(AppOption& option) override;
			void OnInstanceConfigChanged(AppOption& option, IGameInstance& instance) override;
			void OnProfileConfigChanged(AppOption& option, IGameProfile& profile) override;

		public:
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
			wxString GetStartupInstanceID() const override
			{
				return m_StartupInstanceID;
			}

			const IImageProvider& GetImageProvider() const override
			{
				return m_ImageProvider;
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
			const ITranslator& GetTranslator() const override
			{
				return m_Translator;
			}
			KxTranslation::AvailableMap GetAvailableTranslations() const override
			{
				return m_AvailableTranslations;
			}

			bool OpenInstanceSelectionDialog() override;
			bool Uninstall() override;

			void LoadTranslation();
			void LoadImages();
			void ShowWorkspace();

			bool InitSettings(bool downloadLinkPresent = false);
			bool IsPreStartConfigNeeded();
			bool ShowFirstTimeConfigDialog(wxWindow* parent);
			
			void LoadStartupInstanceID();
			bool BeginLoadCurrentInstance(wxWindow* parent = nullptr, bool downloadLinkPresent = false);
			bool LoadInstance();
			bool DispatchDownloadLink(const wxString& link, bool* canContinue = nullptr);
			bool FinalizeInitialization();

			void InitVFS();
			void UnInitVFS();
	};
}
