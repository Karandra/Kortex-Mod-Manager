#pragma once
#include "Framework.hpp"
#include <kxf/General/Singleton.h>
#include <kxf/General/Version.h>
#include <kxf/General/IVariablesCollection.h>
#include <kxf/Localization/Locale.h>
#include <kxf/Localization/ILocalizationPackage.h>
class wxLog;
class wxCmdLineParser;

namespace kxf
{
	class XMLDocument;
}

namespace Kortex
{
	class LogEvent;
	class SystemApplication;
	class BroadcastProcessor;
	class IGameInstance;
	class IGameProfile;
	class IMainWindow;
	struct CmdLineParameters;
}

namespace Kortex
{
	class IApplication: public kxf::SingletonPtr<IApplication>//, public Application::WithOptions<IApplication>
	{
		friend class SystemApplication;

		public:
			static SystemApplication& GetSystemApp() noexcept;
			static IApplication& GetInstance() noexcept;

		private:
			void OnConfigureCommandLine();

		protected:
			virtual void OnCreate() = 0;
			virtual void OnDestroy() = 0;
			virtual bool OnInit() = 0;
			virtual int OnExit() = 0;

			virtual void OnError(LogEvent& event) = 0;
			virtual bool OnException() = 0;
			kxf::String ExamineCaughtException() const;

			//virtual void OnGlobalConfigChanged(AppOption& option) = 0;
			//virtual void OnInstanceConfigChanged(AppOption& option, IGameInstance& instance) = 0;
			//virtual void OnProfileConfigChanged(AppOption& option, IGameProfile& profile) = 0;

		public:
			kxf::String GetRootFolder() const;
			kxf::String GetExecutablePath() const;
			kxf::String GetExecutableName() const;

			virtual kxf::String GetDataFolder() const = 0;
			virtual kxf::String GetLogsFolder() const = 0;
			virtual kxf::String GetUserSettingsFolder() const = 0;
			virtual kxf::String GetUserSettingsFile() const = 0;
			virtual kxf::String GetInstancesFolder() const = 0;
			virtual kxf::String GetStartupInstanceID() const = 0;

			virtual const kxf::ILocalizationPackage& GetLocalizationPackage() const = 0;
			virtual size_t EnumLocalizationPackages(std::function<bool(kxf::Locale, kxf::FileItem)> func) const = 0;

			virtual kxf::IVariablesCollection& GetVariables() = 0;
			virtual kxf::String ExpandVariablesLocally(const kxf::String& variables) const = 0;
			virtual kxf::String ExpandVariables(const kxf::String& variables) const = 0;
			
			virtual bool OpenInstanceSelectionDialog() = 0;
			virtual bool Uninstall() = 0;

		public:
			bool Is64Bit() const;
			bool IsSystem64Bit() const;
			bool IsAnotherInstanceRunning() const;

			bool QueueDownloadToMainProcess(const kxf::String& link);
			std::optional<kxf::String> GetLinkFromCommandLine() const;

			wxCmdLineParser& GetCommandLineParser() const;
			kxf::String FormatCommandLine(const CmdLineParameters& parameters);
			bool ScheduleRestart(const kxf::String& commandLine = {}, std::optional<kxf::TimeSpan> timeout = {});
			void Exit(int exitCode = 0);

			kxf::String GetID() const;
			kxf::String GetName() const;
			kxf::String GetShortName() const;
			kxf::String GetDeveloper() const;
			kxf::Version GetVersion() const;
			kxf::Version GetWxWidgetsVersion() const;
			kxf::XMLDocument& GetGlobalConfig() const;

			wxWindow* GetActiveWindow() const;
			wxWindow* GetTopWindow() const;
			void SetTopWindow(wxWindow* window);
			bool IsActive() const;
			bool IsMainWindowActive() const;

			wxLog& GetLogger();
			BroadcastProcessor& GetBroadcastProcessor();
	};
}
