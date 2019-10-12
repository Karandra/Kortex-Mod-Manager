#pragma once
#include "stdafx.h"
#include "ApplicationModule.h"
#include "BroadcastProcessor.h"
#include <KxFramework/KxApp.h>
#include <KxFramework/KxXML.h>
#include <KxFramework/KxURI.h>
#include <wx/apptrait.h>
#include <wx/snglinst.h>

namespace Kortex
{
	class LogEvent;
	class GameID;
	class IGameInstance;
	class IGameProfile;
	class IApplication;
	class AppOption;
	class IThemeManager;
	class INotificationCenter;
	class BroadcastProcessor;
}
namespace Kortex::GameInstance
{
	class TemplateLoader;
}

namespace Kortex
{
	class SystemApplication;
	class SystemApplicationTraits: public wxGUIAppTraits
	{
		friend class SystemApplication;

		private:
			SystemApplication& m_SystemApp;

			FILE* m_LogTargetFILE = nullptr;
			wxLogStderr* m_LogTarget = nullptr;

		private:
			FILE* CreateLogFile() const;

		public:
			SystemApplicationTraits(SystemApplication& systemApp);

		public:
			wxLog* CreateLogTarget() override;
	};
}

namespace Kortex
{
	class SystemApplication final: public KxApp<wxApp, SystemApplication>
	{
		friend class SystemApplicationTraits;
		friend class IApplication;
		friend class AppOption;

		friend class IGameInstance;
		friend class GameInstance::TemplateLoader;
		friend class GameID;

		private:
			BroadcastProcessor m_BroadcastProcessor;
			BroadcastReciever m_BroadcastReciever;

			wxString m_RootFolder;
			wxString m_ExecutableName;
			wxString m_ExecutablePath;

			Application::ApplicationModule m_ApplicationModule;
			std::unique_ptr<IApplication> m_Application;
			std::unique_ptr<IThemeManager> m_ThemeManager;
			std::unique_ptr<INotificationCenter> m_NotificationCenter;
			bool m_IsApplicationInitialized = false;
			int m_ExitCode = 0;

			std::unique_ptr<IGameInstance> m_ActiveGameInstance;
			std::vector<std::unique_ptr<IGameInstance>> m_GameInstanceTemplates;
			std::vector<std::unique_ptr<IGameInstance>> m_ShallowGameInstances;

			SystemApplicationTraits* m_AppTraits = nullptr;
			wxSingleInstanceChecker m_SingleInstanceChecker;
			KxXMLDocument m_GlobalConfig;

		private:
			void InitLogging();
			void UninitLogging();

			void InitComponents();
			void UninitComponents();
			void SetPostCreateVariables();

			void LoadGlobalConfig();
			void SaveGlobalConfig();
			void TerminateActiveInstance();

		public:
			SystemApplication();
			~SystemApplication() override;

		private:
			bool OnInit() override;
			int OnExit() override;
			void OnError(LogEvent& event);

			void OnGlobalConfigChanged(AppOption& option);
			void OnInstanceConfigChanged(AppOption& option, IGameInstance& instance);
			void OnProfileConfigChanged(AppOption& option, IGameProfile& profile);

			auto& GetGameInstanceTemplates()
			{
				return m_GameInstanceTemplates;
			}
			auto& GetShallowGameInstances()
			{
				return m_ShallowGameInstances;
			}
			IGameInstance* GetActiveGameInstance();
			void AssignActiveGameInstance(std::unique_ptr<IGameInstance> instance);

			bool OnException();
			bool OnExceptionInMainLoop() override;
			void OnUnhandledException() override;
			wxString RethrowCatchAndGetExceptionInfo() const;

			wxAppTraits* CreateTraits() override;
			void ExitApp(int exitCode) override;

		public:
			bool IsAnotherRunning() const;
			void ConfigureForInternetExplorer10(bool init) const;
			bool QueueDownloadToMainProcess(const wxString& link) const;
			
			wxString GetShortName() const;
			wxString GetRootFolder() const
			{
				return m_RootFolder;
			}
			wxString GetExecutablePath() const
			{
				return m_ExecutablePath;
			}
			wxString GetExecutableName() const
			{
				return m_ExecutableName;
			}

			KxXMLDocument& GetGlobalConfig()
			{
				return m_GlobalConfig;
			}
			IThemeManager& GetThemeManager() const
			{
				return *m_ThemeManager;
			}
			BroadcastProcessor& GetBroadcastProcessor()
			{
				return m_BroadcastProcessor;
			}
			wxLog* GetLogger() const;
			void CleanupLogs();
	};
}
