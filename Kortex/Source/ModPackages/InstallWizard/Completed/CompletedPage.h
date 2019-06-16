#pragma once
#include "stdafx.h"
#include "../WizardPage.h"
#include "Utility/KOperationWithProgress.h"
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxLabel.h>

namespace Kortex::InstallWizard
{
	class CompletedPage: public WizardPage
	{
		private:
			KxPanel* m_Panel = nullptr;
			KxLabel* m_Label = nullptr;

		protected:
			void OnLoadUIOptions(const Application::ActiveInstanceOption& option) override;
			void OnSaveUIOptions(Application::ActiveInstanceOption& option) const override;
			void OnPackageLoaded() override;

			bool OnOpenPage() override;

			WizardButton GetCancelButton() override
			{
				return WizardButton({}, false);
			}
			WizardButton GetBackwardButton() override
			{
				return WizardButton({}, false);
			}
			WizardButton GetForwardButton() override
			{
				return WizardButton(KTr(KxID_CLOSE), true);
			}

		public:
			CompletedPage(WizardDialog& wizard)
				:WizardPage(wizard)
			{
			}

		public:
			wxWindow* Create() override;
			wxWindow* GetWindow() override
			{
				return m_Panel;
			}
			
			WizardPageID GetID() const override
			{
				return WizardPageID::Completed;
			}
			wxString GetCaption() const override
			{
				return KTr("InstallWizard.Page.Done");
			}
			wxString GetOptionName() const override
			{
				return wxS("Page/Completed");
			}
	};
}
