#pragma once
#include "Framework.hpp"
#include "BroadcastProcessor.h"
#include <kxf/Application/GUIApplication.h>
#include <kxf/Localization/LocalizationPackageStack.h>
#include <wx/snglinst.h>

namespace Kortex
{
	class IApplication;
}

namespace Kortex
{
	class SystemApplication: public kxf::RTTI::ImplementInterface<SystemApplication, kxf::GUIApplication>
	{
		friend class IApplication;

		public:
			static SystemApplication& GetInstance() noexcept
			{
				return static_cast<SystemApplication&>(*kxf::ICoreApplication::GetInstance());
			}

		private:
			kxf::LocalizationPackageStack m_LocalizationPackages;
			BroadcastProcessor m_BroadcastProcessor;

			kxf::FSPath m_RootDirectory;
			kxf::FSPath m_ExecutablePath;

			std::unique_ptr<IApplication> m_Application;
			wxSingleInstanceChecker m_SingleInstance;
			wxCmdLineParser* m_CommandLineParser;

		private:
			// SystemApplication
			kxf::String ExamineCaughtException() const;
			bool OnException();

		public:
			// ICoreApplication
			bool OnCreate() override;
			bool OnInit() override;

			void OnFatalException() override;
			bool OnMainLoopException() override;
			void OnUnhandledException() override;
			void OnAssertFailure(kxf::String file, int line, kxf::String function, kxf::String condition, kxf::String message) override;

			const kxf::ILocalizationPackage& GetLocalizationPackage() const override
			{
				return m_LocalizationPackages;
			}

		public:
			// SystemApplication
			bool IsAnotherInstanceRunning() const
			{
				return m_SingleInstance.IsAnotherRunning();
			}
			kxf::String GetShortName() const;

			BroadcastProcessor& GetBroadcastProcessor()
			{
				return m_BroadcastProcessor;
			}
			IApplication* GetApplication() const
			{
				return m_Application.get();
			}
			wxCmdLineParser& GetCommandLineParser() const
			{
				return *m_CommandLineParser;
			}
			wxLog& GetLogger() const
			{
				return *wxLog::GetActiveTarget();
			}
	};
}
