#include "stdafx.h"
#include "RequirementsPage.h"
#include "DisplayModel.h"
#include <Kortex/InstallWizard.hpp>

namespace Kortex::InstallWizard
{
	void RequirementsPage::OnLoadUIOptions(const Application::ActiveInstanceOption& option)
	{
		option.LoadDataViewLayout(m_DisplayModel->GetView());
	}
	void RequirementsPage::OnSaveUIOptions(Application::ActiveInstanceOption& option) const
	{
		option.SaveDataViewLayout(m_DisplayModel->GetView());
	}
	void RequirementsPage::OnPackageLoaded()
	{
		GetWizard().GetComponentsPage().StoreRequirementsFlags();

		const KPackageProject& package = GetPackageConfig();
		m_DisplayModel->ShowGroups(package.GetRequirements().GetDefaultGroup());
	}

	RequirementsPage::RequirementsPage(WizardDialog& wizard)
		:WizardPage(wizard)
	{
	}
	
	wxWindow* RequirementsPage::Create()
	{
		m_DisplayModel = new RequirementsPageNS::DisplayModel(*this);
		m_DisplayModel->CreateView(GetPageContainer());

		return m_DisplayModel->GetView();
	}
	wxWindow* RequirementsPage::GetWindow()
	{
		return m_DisplayModel->GetView();
	}

	bool RequirementsPage::HasMainRequirements() const
	{
		return !GetPackageConfig().GetRequirements().IsDefaultGroupEmpty();
	}
}
