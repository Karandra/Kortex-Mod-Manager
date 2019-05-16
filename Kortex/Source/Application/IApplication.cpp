#include "stdafx.h"
#include "IApplication.h"
#include "SystemApplication.h"
#include <Kortex/Application.hpp>
#include <Kortex/Events.hpp>
#include "UI/KMainWindow.h"
#include "Archive/KArchive.h"
#include "Utility/Log.h"
#include <KxFramework/KxSystem.h>

namespace Kortex
{
	SystemApplication* IApplication::GetSystemApp()
	{
		return SystemApplication::GetInstance();
	}

	wxString IApplication::RethrowCatchAndGetExceptionInfo() const
	{
		return GetSystemApp()->RethrowCatchAndGetExceptionInfo();
	}

	wxString IApplication::GetRootFolder() const
	{
		return GetSystemApp()->GetRootFolder();
	}
	wxString IApplication::GetExecutablePath() const
	{
		return GetSystemApp()->GetExecutablePath();
	}
	wxString IApplication::GetExecutableName() const
	{
		return GetSystemApp()->GetExecutableName();
	}

	bool IApplication::Is64Bit() const
	{
		#if defined _WIN64
		return true;
		#else
		return false;
		#endif
	}
	bool IApplication::IsSystem64Bit() const
	{
		return KxSystem::Is64Bit();
	}

	bool IApplication::IsAnotherRunning() const
	{
		return GetSystemApp()->IsAnotherRunning();
	}
	bool IApplication::QueueDownloadToMainProcess(const wxString& link)
	{
		return GetSystemApp()->QueueDownloadToMainProcess(link);
	}

	void IApplication::EnableIE10Support()
	{
		GetSystemApp()->ConfigureForInternetExplorer10(true);
	}
	void IApplication::DisableIE10Support()
	{
		GetSystemApp()->ConfigureForInternetExplorer10(false);
	}

	wxString IApplication::GetID() const
	{
		return GetSystemApp()->GetAppName();
	}
	wxString IApplication::GetName() const
	{
		return GetSystemApp()->GetAppDisplayName();
	}
	wxString IApplication::GetShortName() const
	{
		return GetSystemApp()->GetShortName();
	}
	wxString IApplication::GetDeveloper() const
	{
		return GetSystemApp()->GetVendorName();
	}
	KxVersion IApplication::GetVersion() const
	{
		return GetSystemApp()->GetAppVersion();
	}
	KxVersion IApplication::GetWxWidgetsVersion() const
	{
		return wxGetLibraryVersionInfo();
	}

	KxXMLDocument& IApplication::GetGlobalConfig() const
	{
		return GetSystemApp()->GetGlobalConfig();
	}

	wxCmdLineParser& IApplication::GetCmdLineParser() const
	{
		return GetSystemApp()->GetCmdLineParser();
	}
	bool IApplication::ParseCommandLine()
	{
		return GetSystemApp()->ParseCommandLine();
	}

	wxWindow* IApplication::GetActiveWindow() const
	{
		return ::wxGetActiveWindow();
	}
	wxWindow* IApplication::GetTopWindow() const
	{
		return GetSystemApp()->GetTopWindow();
	}
	void IApplication::SetTopWindow(wxWindow* window)
	{
		return GetSystemApp()->SetTopWindow(window);
	}
	bool IApplication::IsActive() const
	{
		const KMainWindow* mainWindow = KMainWindow::GetInstance();
		return GetSystemApp()->IsActive() && mainWindow && mainWindow == GetActiveWindow() && mainWindow->HasFocus();
	}

	void IApplication::ExitApp(int exitCode)
	{
		GetSystemApp()->ExitApp(exitCode);
	}
	wxLog& IApplication::GetLogger()
	{
		return *GetSystemApp()->GetLogger();
	}
	LoadTranslationStatus IApplication::TryLoadTranslation(KxTranslation& translation,
														   const KxTranslation::AvailableMap& availableTranslations,
														   const wxString& component,
														   const wxString& desiredLocale
	) const
	{
		auto LoadLang = [&translation, &component, &availableTranslations](const wxString& name, bool isFullName = false) -> bool
		{
			auto it = availableTranslations.find(isFullName ? name : name + '.' + component);
			if (it != availableTranslations.end())
			{
				Utility::Log::LogInfo("Trying to load translation from file \"%1\" for \"%2\" component", it->second, name);
				if (translation.LoadFromFile(it->second))
				{
					translation.SetLocale(name.BeforeFirst('.'));
					return true;
				}
			}
			return false;
		};

		if (!availableTranslations.empty())
		{
			// Try load translation for desired locale
			if (LoadLang(desiredLocale))
			{
				return LoadTranslationStatus::Success;
			}

			// Try default locales
			if (LoadLang(KxTranslation::GetUserDefaultLocale()) ||
				LoadLang(KxTranslation::GetSystemPreferredLocale()) ||
				LoadLang(KxTranslation::GetSystemDefaultLocale()) ||
				LoadLang("en-US"))
			{
				return LoadTranslationStatus::Success;
			}

			// Try first available
			const auto& first = *availableTranslations.begin();
			if (LoadLang(first.first, true))
			{
				return LoadTranslationStatus::Success;
			}
			return LoadTranslationStatus::LoadingError;
		}
		return LoadTranslationStatus::NoTranslations;
	}
}
