#pragma once
#include "Framework.hpp"
#include "IApplication.h"
#include "BroadcastProcessor.h"
#include "GameInstance/GameID.h"
#include <kxf/Application/ICoreApplication.h>
#include <kxf/General/DynamicVariablesCollection.h>
#include <kxf/Localization/LocalizationPackageStack.h>
#include <kxf/FileSystem/NativeFileSystem.h>
#include <kxf/Serialization/XML.h>

namespace Kortex
{
	class DefaultGameDefinition;
}

namespace Kortex
{
	class DefaultApplication: public kxf::RTTI::ImplementInterface<DefaultApplication, IApplication, kxf::Application::ICommandLine>
	{
		private:
			std::unique_ptr<BroadcastReciever> m_BroadcastReciever;
			kxf::LocalizationPackageStack m_LocalizationPackages;
			kxf::DynamicVariablesCollection m_Variables;
			wxCmdLineParser* m_CommandLineParser = nullptr;

			kxf::NativeFileSystem m_UnscopedFS;
			kxf::ScopedNativeFileSystem m_AppRootFS;
			kxf::ScopedNativeFileSystem m_AppResourcesFS;
			kxf::ScopedNativeFileSystem m_AppLogsFS;
			kxf::ScopedNativeFileSystem m_GlobalConfigFS;
			kxf::ScopedNativeFileSystem m_GameDefinitionsFS;
			kxf::ScopedNativeFileSystem m_GameDefinitionsUserFS;
			kxf::ScopedNativeFileSystem m_GameInstancesFS;

			kxf::XMLDocument m_GlobalConfig;
			kxf::FSPath m_GlobalConfigOverride;

			IMainWindow* m_MainWindow = nullptr;
			std::unordered_map<GameID, std::unique_ptr<DefaultGameDefinition>> m_GameDefinitions;
			std::unique_ptr<IGameInstance> m_ActiveGameInstance;

		private:
			void LoadGameDefinitions();

		protected:
			// IApplication
			bool OnCreate() override;
			void OnDestroy() override;
			bool OnInit() override;
			int OnExit() override;
			bool OnException() override;

			void OnGlobalConfigChanged(AppOption& option) override;
			void OnInstanceConfigChanged(AppOption& option, IGameInstance& instance) override;
			void OnProfileConfigChanged(AppOption& option, IGameProfile& profile) override;

		public:
			DefaultApplication();
			~DefaultApplication();

		public:
			kxf::IFileSystem& GetFileSystem(FileSystemOrigin fsOrigin) override;
			kxf::XMLDocument& GetGlobalConfig() override
			{
				return m_GlobalConfig;
			}
			kxf::String GetStartupInstanceID() const override
			{
				return {};
			}
			IMainWindow* GetMainWindow() const override
			{
				return m_MainWindow;
			}

			size_t EnumLoadedModules(std::function<bool(IModule&)> func) override
			{
				return 0;
			}
			size_t EnumLoadedManagers(std::function<bool(IManager&)> func) override
			{
				return 0;
			}

			const kxf::ILocalizationPackage& GetLocalizationPackage() const override
			{
				return m_LocalizationPackages;
			}
			size_t EnumLocalizationPackages(std::function<bool(kxf::Locale, kxf::FileItem)> func) const override
			{
				return 0;
			}

			kxf::IVariablesCollection& GetVariables() override
			{
				return m_Variables;
			}
			kxf::String ExpandVariables(const kxf::String& variables) const override;
			kxf::String ExpandVariablesLocally(const kxf::String& variables) const override;

			size_t EnumGameDefinitions(std::function<bool(IGameDefinition&)> func) override;
			size_t EnumGameInstances(std::function<bool(IGameInstance&)> func) override;
			IGameInstance* GetActiveGameInstance() const override;

			bool OpenInstanceSelectionDialog() override;
			bool Uninstall() override;

		public:
			// kxf::Application::ICommandLine
			void InitializeCommandLine(char** argv, size_t argc) override
			{
			}
			void InitializeCommandLine(wchar_t** argv, size_t argc) override
			{
			}

			size_t EnumCommandLineArgs(std::function<bool(kxf::String)> func) const override;
			void OnCommandLineInit(wxCmdLineParser& parser) override;
			bool OnCommandLineParsed(wxCmdLineParser& parser) override;
			bool OnCommandLineError(wxCmdLineParser& parser) override;
			bool OnCommandLineHelp(wxCmdLineParser& parser) override;
	};
}
