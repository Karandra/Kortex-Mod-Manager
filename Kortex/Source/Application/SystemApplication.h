#pragma once
#include "stdafx.h"
#include <KxFramework/KxApp.h>
#include <KxFramework/KxXML.h>
#include <wx/snglinst.h>

namespace Kortex
{
	class LogEvent;
	class IApplication;
	class IAppOption;
	class IThemeManager;
	class INotificationCenter;

	class SystemApplicationTraits;
	class SystemApplication final: public KxApp<wxApp, SystemApplication>
	{
		friend class SystemApplicationTraits;
		friend class IApplication;
		friend class IAppOption;

		private:
			std::unique_ptr<IApplication> m_Application;
			std::unique_ptr<IThemeManager> m_ThemeManager;
			std::unique_ptr<INotificationCenter> m_NotificationCenter;
			bool m_IsApplicationInitialized = false;
			int m_ExitCode = 0;

			SystemApplicationTraits* m_AppTraits = nullptr;
			wxSingleInstanceChecker m_SingleInstanceChecker;
			KxXMLDocument m_GlobalConfig;

		private:
			void InitLogging();
			void UninitLogging();

			void InitComponents();
			void UninitComponents();

			void LoadGlobalSettings();
			void SaveGlobalSettings();
			void SaveActiveInstanceSettings();

		public:
			SystemApplication();
			~SystemApplication() override;

		private:
			bool OnInit() override;
			int OnExit() override;
			void OnError(LogEvent& event);
			void OnGlobalConfigChanged(IAppOption& option);

			bool OnException();
			bool OnExceptionInMainLoop() override;
			void OnUnhandledException() override;
			wxString RethrowCatchAndGetExceptionInfo() const;

			wxAppTraits* CreateTraits() override;
			void ExitApp(int exitCode);

		public:
			bool IsAnotherRunning() const;
			void ConfigureForInternetExplorer10(bool init) const;
			bool QueueDownloadToMainProcess(const wxString& link) const;
			
			KxXMLDocument& GetGlobalConfig()
			{
				return m_GlobalConfig;
			}
			IThemeManager& GetThemeManager() const
			{
				return *m_ThemeManager;
			}
			wxLog* GetLogger() const;
			void CleanupLogs();
	};
}
