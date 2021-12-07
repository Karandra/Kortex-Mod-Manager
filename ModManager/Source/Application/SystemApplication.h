#pragma once
#include "Framework.hpp"
#include "BroadcastProcessor.h"
#include <kxf/Application/GUIApplication.h>
#include <kxf/Localization/AndroidLocalizationPackage.h>
#include <wx/snglinst.h>

namespace Kortex
{
	class IApplication;
}

namespace Kortex
{
	class KORTEX_API SystemApplication: public kxf::RTTI::Implementation<SystemApplication, kxf::GUIApplication>
	{
		friend class IApplication;

		public:
			static SystemApplication& GetInstance() noexcept
			{
				return static_cast<SystemApplication&>(*kxf::ICoreApplication::GetInstance());
			}

		private:
			BroadcastProcessor m_BroadcastProcessor;
			kxf::AndroidLocalizationPackage m_EmptyLocalizationPackage;

			std::shared_ptr<IApplication> m_Application;
			wxSingleInstanceChecker m_SingleInstance;
			kxf::FSPath m_RootDirectory;

		private:
			// SystemApplication
			kxf::String ExamineCaughtException() const;
			bool OnException();

			void InitializeLogging();
			void InitializeFramework();

		public:
			SystemApplication();
			~SystemApplication();

		public:
			// ICoreApplication
			bool OnCreate() override;
			void OnDestroy() override;
			bool OnInit() override;

			void OnFatalException() override;
			bool OnMainLoopException() override;
			void OnUnhandledException() override;
			void OnAssertFailure(const kxf::String& file, int line, const kxf::String& function, const kxf::String& condition, const kxf::String& message) override;

			const kxf::ILocalizationPackage& GetLocalizationPackage() const override;
			
			// ICoreApplication -> ICommandLine
			kxf::Enumerator<kxf::String> EnumCommandLineArgs() const override;
			void OnCommandLineInit(kxf::CommandLineParser& parser) override;
			bool OnCommandLineParsed(kxf::CommandLineParser& parser) override;
			bool OnCommandLineError(kxf::CommandLineParser& parser) override;
			bool OnCommandLineHelp(kxf::CommandLineParser& parser) override;

		public:
			// SystemApplication
			bool IsAnotherInstanceRunning() const
			{
				return m_SingleInstance.IsAnotherRunning();
			}
			kxf::FSPath GetRootDirectory() const
			{
				return m_RootDirectory;
			}
			kxf::String GetShortName() const;

			IApplication* GetApplication() const
			{
				return m_Application.get();
			}
			BroadcastProcessor& GetBroadcastProcessor()
			{
				return m_BroadcastProcessor;
			}
			wxLog& GetLogger() const
			{
				return *wxLog::GetActiveTarget();
			}
	};
}
