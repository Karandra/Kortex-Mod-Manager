#pragma once
#include "Framework.hpp"
#include "Options/Option.h"
#include <kxf/General/Version.h>
#include <kxf/General/IVariablesCollection.h>
#include <kxf/FileSystem/IFileSystem.h>
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
	class IModule;
	class IGameInstance;
	class IGameDefinition;
	class IGameProfile;
	class IMainWindow;
	class BroadcastProcessor;

	enum class FileSystemOrigin
	{
		None = -1,
		Unscoped = 0,

		AppRoot,
		AppLogs,
		AppResources,
		GlobalConfig,
		GameDefinitions,
		GameDefinitionsUser,
		GameInstances
	};
}

namespace Kortex
{
	class IApplication: public kxf::RTTI::Interface<IApplication>, public Application::WithOptions<IApplication>
	{
		KxRTTI_DeclareIID(IApplication, {0xb5e8047c, 0x9239, 0x45c4, {0x86, 0xf6, 0x6c, 0x83, 0xa8, 0x42, 0x06, 0x3e}});

		friend class SystemApplication;
		friend class AppOption;

		public:
			static IApplication& GetInstance() noexcept;

		protected:
			virtual bool OnCreate() = 0;
			virtual void OnDestroy() = 0;
			virtual bool OnInit() = 0;
			virtual int OnExit() = 0;
			virtual bool OnException() = 0;
			kxf::String ExamineCaughtException() const;

			virtual void OnGlobalConfigChanged(AppOption& option) = 0;
			virtual void OnInstanceConfigChanged(AppOption& option, IGameInstance& instance) = 0;
			virtual void OnProfileConfigChanged(AppOption& option, IGameProfile& profile) = 0;

		public:
			virtual const kxf::ILocalizationPackage& GetLocalizationPackage() const = 0;
			virtual kxf::IFileSystem& GetFileSystem(FileSystemOrigin fsOrigin) = 0;
			virtual kxf::XMLDocument& GetGlobalConfig() = 0;
			virtual kxf::String GetStartupInstanceID() const = 0;
			virtual IMainWindow* GetMainWindow() const = 0;
			virtual kxf::Enumerator<IModule&> EnumModules() = 0;

			virtual kxf::IVariablesCollection& GetVariables() = 0;
			virtual kxf::String ExpandVariables(const kxf::String& variables) const = 0;
			virtual kxf::String ExpandVariablesLocally(const kxf::String& variables) const = 0;

			virtual kxf::Enumerator<IGameDefinition&> EnumGameDefinitions() = 0;
			virtual kxf::Enumerator<IGameInstance&> EnumGameInstances() = 0;
			virtual IGameInstance* GetActiveGameInstance() const = 0;
			IGameDefinition* GetGameDefinitionByName(const kxf::String& name);
			IGameInstance* GetGameInstanceByName(const kxf::String& name);
			
			virtual IGameInstance* OpenInstanceSelectionDialog(wxWindow* parent = nullptr) = 0;
			virtual bool Uninstall() = 0;

		public:
			bool Is64Bit() const;
			bool IsSystem64Bit() const;
			bool IsAnotherInstanceRunning() const;

			kxf::String FormatCommandLine(const std::unordered_map<kxf::String, kxf::String>& parameters);
			bool ScheduleRestart(const kxf::String& commandLine = {}, std::optional<kxf::TimeSpan> timeout = {});
			void Exit(int exitCode = 0);

			kxf::String GetID() const;
			kxf::String GetName() const;
			kxf::String GetShortName() const;
			kxf::String GetDeveloper() const;
			kxf::Version GetVersion() const;
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
