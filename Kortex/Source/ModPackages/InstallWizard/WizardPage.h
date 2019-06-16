#pragma once
#include "stdafx.h"
#include "Application/Resources/ImageResourceID.h"
#include "PackageProject/KPackageProject.h"

namespace Kortex
{
	class ModPackage;
}
namespace Kortex::Application
{
	class ActiveInstanceOption;
}
namespace Kortex::InstallWizard
{
	class WizardDialog;

	enum class WizardPageID
	{
		None = -1,
		Info,
		Requirements,
		Components,
		Installation,
		Completed,
	};
}

namespace Kortex::InstallWizard
{
	class WizardPage: public wxEvtHandler
	{
		friend class WizardDialog;

		private:
			WizardDialog& m_Wizard;

		protected:
			virtual void OnLoadUIOptions(const Application::ActiveInstanceOption& option) = 0;
			virtual void OnSaveUIOptions(Application::ActiveInstanceOption& option) const = 0;
			virtual void OnPackageLoaded() = 0;

			virtual void OnOpenPage() {}
			virtual void OnClosePage() {}

		public:
			WizardPage(WizardDialog& wizard)
				:m_Wizard(wizard)
			{
			}

		public:
			const WizardDialog& GetWizard() const
			{
				return m_Wizard;
			}
			WizardDialog& GetWizard()
			{
				return m_Wizard;
			}

			wxWindow* GetPageContainer() const;
			ModPackage& GetPackage() const;
			KPackageProject& GetPackageConfig() const;

		public:
			virtual wxWindow* Create() = 0;
			virtual wxWindow* GetWindow() = 0;

			virtual WizardPageID GetID() const = 0;
			virtual wxString GetCaption() const = 0;
			virtual wxString GetOptionName() const = 0;
	};
}
