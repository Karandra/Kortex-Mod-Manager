#pragma once
#include "Framework.hpp"
#include "IApplication.h"
#include "BroadcastProcessor.h"
#include <kxf/Application/ICoreApplication.h>
#include <kxf/General/DynamicVariablesCollection.h>
#include <kxf/Localization/LocalizationPackageStack.h>

namespace Kortex::Application
{
	class MainWindow;
}

namespace Kortex::Application
{
	class DefaultApplication: public kxf::RTTI::ImplementInterface<DefaultApplication, IApplication, kxf::Application::ICommandLine>
	{
		private:
			std::unique_ptr<BroadcastReciever> m_BroadcastReciever;
			kxf::LocalizationPackageStack m_LocalizationPackages;
			kxf::DynamicVariablesCollection m_Variables;
			wxCmdLineParser* m_CommandLineParser = nullptr;

			kxf::FSPath m_DataDirectory;
			kxf::FSPath m_SettingsDirectory;
			kxf::FSPath m_SettingsFile;
			kxf::FSPath m_LogsDirectory;
			kxf::FSPath m_DefaultInstancesDirectory;
			IMainWindow* m_MainWindow = nullptr;

		protected:
			// IApplication
			void OnCreate() override;
			void OnDestroy() override;
			bool OnInit() override;
			int OnExit() override;
			bool OnException() override;

			void OnGlobalConfigChanged(AppOption& option) override;
			void OnInstanceConfigChanged(AppOption& option, IGameInstance& instance) override;
			void OnProfileConfigChanged(AppOption& option, IGameProfile& profile) override;

		public:
			kxf::FSPath GetDataDirectory() const override
			{
				return m_DataDirectory;
			}
			kxf::FSPath GetLogsDirectory() const override
			{
				return m_LogsDirectory;
			}
			kxf::FSPath GetSettingsDirectory() const override
			{
				return m_SettingsDirectory;
			}
			kxf::FSPath GetSettingsFile() const override
			{
				return m_SettingsFile;
			}
			kxf::FSPath GetInstancesDirectory() const override
			{
				return m_DefaultInstancesDirectory;
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
