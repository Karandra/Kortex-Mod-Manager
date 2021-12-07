#pragma once
#include "Framework.hpp"
#include "IApplication.h"
#include "IResourceManager.h"
#include "IModule.h"
#include "BroadcastProcessor.h"
#include <kxf/Application/ICoreApplication.h>
#include <kxf/General/DynamicVariablesCollection.h>
#include <kxf/Localization/LocalizationPackageStack.h>
#include <kxf/Localization/QtLocalizationPackage.h>
#include <kxf/FileSystem/NativeFileSystem.h>
#include <kxf/Serialization/XML.h>

namespace Kortex
{
	class KORTEX_API DefaultApplication: public kxf::RTTI::DynamicImplementation<DefaultApplication, IApplication, kxf::Application::ICommandLine>
	{
		public:
			std::unique_ptr<BroadcastReceiver> m_BroadcastReceiver;
			kxf::LocalizationPackageStack m_LocalizationPackages;
			kxf::DynamicVariablesCollection m_Variables;
			kxf::CommandLineParser* m_CommandLineParser = nullptr;

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
			bool m_GlobalConfigChanged = false;

			std::weak_ptr<IMainWindow> m_MainWindow;
			std::shared_ptr<IResourceManager> m_ResourceManager;
			std::vector<std::shared_ptr<IModule>> m_LoadedModules;

			std::unordered_map<kxf::String, std::unique_ptr<IGameDefinition>> m_GameDefinitions;
			std::unordered_map<kxf::String, std::unique_ptr<IGameInstance>> m_GameInstances;
			IGameInstance* m_ActiveGameInstance = nullptr;

		private:
			void LoadGlobalConfiguration();
			void LoadLocalizationPackages();

			void LoadGameDefinitions();
			IGameInstance* LoadGameInstances();
			void InitializeActiveInstance(IGameInstance& activeInstance);

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
			const kxf::ILocalizationPackage& GetLocalizationPackage() const override
			{
				return m_LocalizationPackages;
			}
			kxf::IFileSystem& GetFileSystem(FileSystemOrigin fsOrigin) override;
			kxf::XMLDocument& GetGlobalConfig() override
			{
				return m_GlobalConfig;
			}
			kxf::String GetStartupInstanceID() const override
			{
				return {};
			}
			std::shared_ptr<IMainWindow> GetMainWindow() const override
			{
				return m_MainWindow.lock();
			}
			IResourceManager& GetResourceManager() const override
			{
				return *m_ResourceManager;
			}
			kxf::Enumerator<IModule&> EnumModules() override;

			kxf::IVariablesCollection& GetVariables() override
			{
				return m_Variables;
			}
			kxf::String ExpandVariables(const kxf::String& variables) const override;
			kxf::String ExpandVariablesLocally(const kxf::String& variables) const override;

			kxf::Enumerator<IGameDefinition&> EnumGameDefinitions() override;
			kxf::Enumerator<IGameInstance&> EnumGameInstances() override;
			IGameInstance* GetActiveGameInstance() const override;

			IGameInstance* OpenInstanceSelectionDialog(wxWindow* parent = nullptr) override;
			bool Uninstall() override;

		public:
			// kxf::Application::ICommandLine
			void InitializeCommandLine(char** argv, size_t argc) override
			{
			}
			void InitializeCommandLine(wchar_t** argv, size_t argc) override
			{
			}

			kxf::Enumerator<kxf::String> EnumCommandLineArgs() const override;
			void OnCommandLineInit(kxf::CommandLineParser& parser) override;
			bool OnCommandLineParsed(kxf::CommandLineParser& parser) override;
			bool OnCommandLineError(kxf::CommandLineParser& parser) override;
			bool OnCommandLineHelp(kxf::CommandLineParser& parser) override;
	};
}
