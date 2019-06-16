#pragma once
#include "stdafx.h"
#include "../WizardPage.h"
#include "../StepStack.h"
#include <KxFramework/KxImageView.h>
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxSplitterWindow.h>

namespace Kortex::InstallWizard
{
	namespace RequirementsPageNS
	{
		class DisplayModel;
	}
	namespace ComponentsPageNS
	{
		class DisplayModel;
		class DisplayModelNode;
	}
}

namespace Kortex::InstallWizard
{
	class ComponentsPage: public WizardPage
	{
		friend class WizardDialog;

		private:
			using DisplayModelNode = ComponentsPageNS::DisplayModelNode;

		private:
			KxSplitterWindow* m_SplitterV = nullptr;
			KxSplitterWindow* m_SplitterHRight = nullptr;

			ComponentsPageNS::DisplayModel* m_ComponentsModel = nullptr;
			RequirementsPageNS::DisplayModel* m_RequirementsModel = nullptr;
			KxAuiNotebook* m_TabView = nullptr;
			KxImageView* m_ImageView = nullptr;
			KxHTMLWindow* m_Description = nullptr;

			StepStack m_InstallSteps;
			std::unordered_map<wxString, wxString> m_FlagsStorage;
			bool m_HasManualComponents = false;

		protected:
			void OnLoadUIOptions(const Application::ActiveInstanceOption& option) override;
			void OnSaveUIOptions(Application::ActiveInstanceOption& option) const override;
			void OnPackageLoaded() override;

			bool OnOpenPage() override;

		private:
			void OnSelectComponent(KxDataViewEvent& event);
			void ClearComponentsViewInfo();

			void OnGoStepBackward(wxCommandEvent& event);
			void OnGoStepForward(wxCommandEvent& event);

		public:
			ComponentsPage(WizardDialog& wizard)
				:WizardPage(wizard)
			{
			}

		public:
			wxWindow* Create() override;
			wxWindow* GetWindow() override
			{
				return nullptr;
			}
			
			WizardPageID GetID() const override
			{
				return WizardPageID::Components;
			}
			wxString GetCaption() const override
			{
				return KTr("InstallWizard.Page.Components");
			}
			wxString GetOptionName() const override
			{
				return wxS("Page/Components");
			}

		public:
			void ClearDisplay();
			void ResetComponents();
			bool BeginComponents();

			void StoreRequirementsFlags();
			void StoreStepFlags(const KPPCEntry::RefVector& checkedEntries);
			void RestoreStepFlagsUpToThis(const KPPCStep& step);

			bool IsConditionSatisfied(const KPPCFlagEntry& flagEntry) const;
			bool IsConditionsSatisfied(const KPPCConditionGroup& conditionGroup) const;
			bool IsStepSatisfiesConditions(const KPPCStep& step) const;
			bool CheckIsManualComponentsAvailable() const;
			KPPCStep* GetFirstStepSatisfiesConditions() const;
			KPPCStep* GetFirstStepSatisfiesConditions(const KPPCStep* afterThis) const;
			
			const StepStack& GetInstallSteps() const
			{
				return m_InstallSteps;
			}
			KPPCStep* GetCurrentStep() const;
			const StepStackItem* GetCurrentStepItem() const;
			void LoadManualStep(KPPCStep& step);

			bool HasManualComponents() const
			{
				return m_HasManualComponents;
			}
			bool HasConditionalInstall() const
			{
				return !GetPackageConfig().GetComponents().GetConditionalSteps().empty();
			}
	};
}
