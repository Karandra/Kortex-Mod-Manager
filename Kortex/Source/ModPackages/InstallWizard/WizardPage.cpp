#include "stdafx.h"
#include "WizardPage.h"
#include <Kortex/InstallWizard.hpp>

namespace Kortex::InstallWizard
{
	wxWindow* WizardPage::GetPageContainer() const
	{
		return m_Wizard.m_PageContainer;
	}
	KPackageProject& WizardPage::GetPackageConfig() const
	{
		return m_Wizard.GetConfig();
	}
}
