#pragma once
#include "stdafx.h"
#include "ModPackages/InstallWizard/WizardPage.h"
#include <KxFramework/KxNotebook.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxListBox.h>
#include <KxFramework/KxImageView.h>
#include <KxFramework/KxThumbView.h>
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxSplitterWindow.h>
class KImageViewerEvent;

namespace Kortex::InstallWizard
{
	namespace InfoPageNS
	{
		class InfoDisplayModel;
	}

	class InfoPage: public WizardPage
	{
		private:
			KxAuiNotebook* m_TabsContainer = nullptr;
			InfoPageNS::InfoDisplayModel* m_InfoDisplayModel = nullptr;
			KxHTMLWindow* m_DescriptionView = nullptr;

			KxSplitterWindow* m_DocumentsContainer = nullptr;
			KxListBox* m_DocumentsList = nullptr;
			KxHTMLWindow* m_DocumentSimple = nullptr;
			wxWebView* m_DocumentAdvanced = nullptr;

			KxThumbView* m_ScreenshotsView = nullptr;
			std::unordered_map<int, const KPPIImageEntry*> m_ImagesMap;
			int m_CurrentImageIndex = -1;

		private:
			wxWindow* CreateInfoTab();
			wxWindow* CreateDescriptionTab();
			wxWindow* CreateDocumentsTab();
			wxWindow* CreateScreenshotsTab();

			void LoadInfoTab(const KPackageProject& package);
			void LoadDescriptionTab(const KPackageProject& package);
			void LoadDocumentsTab(const KPackageProject& package);
			void LoadScreenshotsTab(const KPackageProject& package);
			
			void OnSelectDocument(int index, bool useAdvancedEditor = false);
			void SetImageViewerNavigationInfo(KImageViewerEvent& event) const;
			void OnNavigateImageViewer(KImageViewerEvent& event);

		protected:
			void OnLoadUIOptions(const Application::ActiveInstanceOption& option) override;
			void OnSaveUIOptions(Application::ActiveInstanceOption& option) const override;
			void OnPackageLoaded() override;

		public:
			InfoPage(WizardDialog& wizard);

		public:
			wxWindow* Create() override;
			wxWindow* GetWindow() override
			{
				return m_TabsContainer;
			}
			
			WizardPageID GetID() const override
			{
				return WizardPageID::Info;
			}
			wxString GetCaption() const override
			{
				return KTr("InstallWizard.Page.Info");
			}
			wxString GetOptionName() const override
			{
				return wxS("Page/Information");
			}
	};
}
