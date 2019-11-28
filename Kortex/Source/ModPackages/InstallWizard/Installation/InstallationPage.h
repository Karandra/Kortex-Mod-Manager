#pragma once
#include "stdafx.h"
#include "../WizardPage.h"
#include "Utility/KOperationWithProgress.h"
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxProgressBar.h>
#include <KxFramework/KxArchiveEvent.h>

namespace Kortex::InstallWizard
{
	class InstallOperation;

	class InstallationPage: public WizardPage
	{
		friend class WizardDialog;
		friend class InstallOperation;

		private:
			KxPanel* m_PagePanel = nullptr;

			KxProgressBar* m_MinorProgress = nullptr;
			KxProgressBar* m_MajorProgress = nullptr;
			KxLabel* m_MinorStatus = nullptr;
			KxLabel* m_MajorStatus = nullptr;

			PackageProject::FileItem::RefVector m_InstallableFiles;
			InstallOperation* m_InstallThread = nullptr;

			bool m_ShouldCancel = false;
			bool m_IsComplete = false;

		private:
			bool OnBeginInstall();
			bool OnEndInstall();
			void OnMinorProgress(KxFileOperationEvent& event);
			void OnMajorProgress(KxFileOperationEvent& event);

		protected:
			void OnLoadUIOptions(const Application::ActiveInstanceOption& option) override;
			void OnSaveUIOptions(Application::ActiveInstanceOption& option) const override;
			void OnPackageLoaded() override;

			bool OnOpenPage() override;
			bool OnClosePage() override;

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
			InstallationPage(WizardDialog& wizard)
				:WizardPage(wizard)
			{
			}

		public:
			wxWindow* Create() override;
			wxWindow* GetWindow() override
			{
				return m_PagePanel;
			}
			
			WizardPageID GetID() const override
			{
				return WizardPageID::Installation;
			}
			wxString GetCaption() const override
			{
				return KTr("InstallWizard.Page.Installing");
			}
			wxString GetOptionName() const override
			{
				return wxS("Page/Installation");
			}
	
		public:
			void CollectAllInstallableEntries();
			void SortInstallableFiles();
			void ShowInstallableFilesPreview();
			const PackageProject::FileItem::RefVector& GetInstallableFiles() const
			{
				return  m_InstallableFiles;
			}

			bool ShouldCancel() const
			{
				return m_ShouldCancel;
			}
			bool IsCompleted() const
			{
				return m_IsComplete;
			}
	};
}

namespace Kortex::InstallWizard
{
	class InstallOperation: public KOperationWithProgressBase
	{
		private:
			InstallationPage& m_InstallPage;

		public:
			InstallOperation(InstallationPage& page)
				:KOperationWithProgressBase(true), m_InstallPage(page)
			{
			}

		public:
			void LinkHandler(wxEvtHandler* eventHandler, wxEventType type) override
			{
				using T = wxEventTypeTag<KxFileOperationEvent>;
				GetEventHandler()->Bind(static_cast<T>(type), &InstallationPage::OnMinorProgress, &m_InstallPage);
				KOperationWithProgressBase::LinkHandler(eventHandler, type);
			}
	};
}
