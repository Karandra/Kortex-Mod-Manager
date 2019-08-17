#pragma once
#include "stdafx.h"
#include "GameInstance/GameID.h"
#include "Resources/IImageProvider.h"
#include "Options/Option.h"
#include <KxFramework/KxXML.h>
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxTranslation.h>
#include <KxFramework/KxVersion.h>
#include <KxFramework/KxURI.h>
class KxImageList;
class KxImageSet;

namespace Kortex
{
	class LogEvent;
	class SystemApplication;
	class BroadcastProcessor;
	class IVariableTable;
	class IGameInstance;
	class IGameProfile;
	class ITranslator;
}
namespace Kortex
{
	struct CmdLineParameters;
}

namespace Kortex
{
	enum class LoadTranslationStatus
	{
		Success = 0,
		NoTranslations,
		LoadingError
	};
}

namespace Kortex
{
	class IApplication:
		public KxSingletonPtr<IApplication>,
		public Application::WithOptions<IApplication>
	{
		friend class SystemApplication;

		public:
			static SystemApplication* GetSystemApp();

		private:
			void OnConfigureCommandLine();

		protected:
			virtual void OnCreate() = 0;
			virtual void OnDestroy() = 0;
			virtual bool OnInit() = 0;
			virtual int OnExit() = 0;

			virtual void OnError(LogEvent& event) = 0;
			virtual bool OnException() = 0;
			wxString RethrowCatchAndGetExceptionInfo() const;

			virtual void OnGlobalConfigChanged(AppOption& option) = 0;
			virtual void OnInstanceConfigChanged(AppOption& option, IGameInstance& instance) = 0;
			virtual void OnProfileConfigChanged(AppOption& option, IGameProfile& profile) = 0;

		public:
			wxString GetRootFolder() const;
			wxString GetExecutablePath() const;
			wxString GetExecutableName() const;

			virtual wxString GetDataFolder() const = 0;
			virtual wxString GetLogsFolder() const = 0;
			virtual wxString GetUserSettingsFolder() const = 0;
			virtual wxString GetUserSettingsFile() const = 0;
			virtual wxString GetInstancesFolder() const = 0;
			virtual wxString GetStartupInstanceID() const = 0;

			virtual bool IsTranslationLoaded() const = 0;
			virtual const KxTranslation& GetTranslation() const = 0;
			virtual const ITranslator& GetTranslator() const = 0;
			virtual KxTranslation::AvailableMap GetAvailableTranslations() const = 0;

			virtual const IImageProvider& GetImageProvider() const = 0;

			virtual IVariableTable& GetVariables() = 0;
			virtual wxString ExpandVariablesLocally(const wxString& variables) const = 0;
			virtual wxString ExpandVariables(const wxString& variables) const = 0;
			
			virtual bool OpenInstanceSelectionDialog() = 0;
			virtual bool Uninstall() = 0;

		public:
			bool Is64Bit() const;
			bool IsSystem64Bit() const;
			bool IsAnotherRunning() const;

			bool QueueDownloadToMainProcess(const wxString& link);
			std::optional<wxString> GetLinkFromCommandLine() const;

			wxCmdLineParser& GetCmdLineParser() const;
			wxString FormatCommandLine(const CmdLineParameters& parameters);
			bool ScheduleRestart(const wxString& commandLine = {}, std::optional<wxTimeSpan> timeout = {});

			void EnableIE10Support();
			void DisableIE10Support();

			wxString GetID() const;
			wxString GetName() const;
			wxString GetShortName() const;
			wxString GetDeveloper() const;
			KxVersion GetVersion() const;
			KxVersion GetWxWidgetsVersion() const;
			KxXMLDocument& GetGlobalConfig() const;
			IModule& GetModule() const;

			wxWindow* GetActiveWindow() const;
			wxWindow* GetTopWindow() const;
			void SetTopWindow(wxWindow* window);
			bool IsActive() const;
			bool IsMainWindowActive() const;

			void ExitApp(int exitCode = 0);
			wxLog& GetLogger();
			BroadcastProcessor& GetBroadcastProcessor();
			LoadTranslationStatus TryLoadTranslation(KxTranslation& translation,
													 const KxTranslation::AvailableMap& availableTranslations,
													 const wxString& component,
													 const wxString& desiredLocale = wxEmptyString
			) const;
	};
}
