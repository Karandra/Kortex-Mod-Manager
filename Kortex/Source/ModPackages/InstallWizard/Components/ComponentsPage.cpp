#include "stdafx.h"
#include "ComponentsPage.h"
#include "DisplayModel.h"
#include "../Requirements/DisplayModel.h"
#include "UI/ImageViewerDialog.h"
#include "Utility/UI.h"
#include <Kortex/Application.hpp>
#include <Kortex/InstallWizard.hpp>
#include <KxFramework/KxTaskDialog.h>

namespace
{
	enum class TabIndex
	{
		Description = 0,
		Requirements = 1,
	};
	namespace OName
	{
		KortexDefOption(Splitter);
		KortexDefOption(RequirementsView);
		KortexDefOption(ComponentsView);
	}
}

namespace Kortex::InstallWizard
{
	void ComponentsPage::OnLoadUIOptions(const Application::ActiveInstanceOption& option)
	{
		option.QueryElement(OName::Splitter).LoadSplitterLayout(m_SplitterV);
		option.QueryElement(OName::Splitter).LoadSplitterLayout(m_SplitterHRight);
		option.QueryElement(OName::ComponentsView).LoadDataViewLayout(m_ComponentsModel->GetView());
		option.QueryElement(OName::RequirementsView).LoadDataViewLayout(m_RequirementsModel->GetView());
	}
	void ComponentsPage::OnSaveUIOptions(Application::ActiveInstanceOption& option) const
	{
		option.ConstructElement(OName::Splitter).SaveSplitterLayout(m_SplitterV);
		option.ConstructElement(OName::Splitter).SaveSplitterLayout(m_SplitterHRight);
		option.ConstructElement(OName::ComponentsView).SaveDataViewLayout(m_ComponentsModel->GetView());
		option.ConstructElement(OName::RequirementsView).SaveDataViewLayout(m_RequirementsModel->GetView());
	}
	void ComponentsPage::OnPackageLoaded()
	{
		m_HasManualComponents = CheckIsManualComponentsAvailable();
	}

	bool ComponentsPage::OnOpenPage()
	{
		if (!BeginComponents())
		{
			GetWizard().Close(true);
			return false;
		}
		return true;
	}

	void ComponentsPage::OnSelectComponent(KxDataViewEvent& event)
	{
		DisplayModelNode* node = m_ComponentsModel->GetNode(event.GetItem());
		if (event.GetEventType() == KxEVT_DATAVIEW_ITEM_HOVERED && (!event.GetItem().IsOK() || !node || node->IsGroup()))
		{
			return;
		}
		ClearComponentsViewInfo();

		if (node)
		{
			if (const PackageProject::ComponentItem* entry = node->GetEntry())
			{
				const wxString& description = entry->GetDescription();
				const bool isDescriptionEmpty = description.IsEmpty();
				if (!isDescriptionEmpty)
				{
					m_Description->SetValue(description);
					m_Description->Enable();
				}
				else
				{
					m_Description->SetValue(Utility::UI::MakeHTMLWindowPlaceholder(KTr("InstallWizard.NoDescriptionHint"), m_Description));
				}
				m_TabView->SetPageImage((size_t)TabIndex::Description, (int)(!isDescriptionEmpty ? ImageResourceID::InformationFrame : ImageResourceID::InformationFrameEmpty));

				bool bRequirementsEmpty = entry->GetRequirements().empty();
				if (!bRequirementsEmpty)
				{
					const bool isOK = GetPackageConfig().GetRequirements().CalcOverallStatus(entry->GetRequirements());

					m_TabView->SetPageImage((size_t)TabIndex::Requirements, (int)(isOK ? ImageResourceID::TickCircleFrame : ImageResourceID::CrossCircleFrame));
					//m_Components_Requirements->SetDataVector(&GetConfig().GetRequirements(), entry->GetRequirements());
				}
				else
				{
					m_TabView->SetPageImage((size_t)TabIndex::Requirements, (int)ImageResourceID::InformationFrameEmpty);
				}

				const PackageProject::ImageItem* imageEntry = GetPackageConfig().GetInterface().FindImageByPath(entry->GetImage());
				if (imageEntry && imageEntry->HasBitmap())
				{
					m_ImageView->SetBitmap(imageEntry->GetBitmap());
					m_ImageView->Enable();
				}

				// Tab switch
				if (!isDescriptionEmpty)
				{
					m_TabView->ChangeSelection((size_t)TabIndex::Description);
				}
				else
				{
					m_TabView->ChangeSelection((size_t)TabIndex::Requirements);
				}
				m_ComponentsModel->GetView()->SetFocus();
			}
		}
		event.Skip();
	}
	void ComponentsPage::ClearComponentsViewInfo()
	{
		m_TabView->SetPageImage((size_t)TabIndex::Description, (int)ImageResourceID::InformationFrameEmpty);
		m_TabView->SetPageImage((size_t)TabIndex::Requirements, (int)ImageResourceID::InformationFrameEmpty);

		m_Description->SetValue(Utility::UI::MakeHTMLWindowPlaceholder(KTr("InstallWizard.SelectComponentHint"), m_Description));
		m_Description->Disable();

		m_ImageView->SetBitmap(wxNullBitmap);
		m_ImageView->Disable();

		m_RequirementsModel->ClearDisplay();
	}

	void ComponentsPage::OnGoStepBackward(wxCommandEvent& event)
	{
		if (m_InstallSteps.GetSize() > 1)
		{
			m_InstallSteps.PopItem();

			PackageProject::ComponentStep& previousStep = *m_InstallSteps.GetTopStep();
			RestoreStepFlagsUpToThis(previousStep);
			LoadManualStep(previousStep);
		}
		else
		{
			// Now stack has only first step. Clear it.
			m_InstallSteps.Clear();
			event.Skip();
		}
	}
	void ComponentsPage::OnGoStepForward(wxCommandEvent& event)
	{
		if (PackageProject::ComponentStep* currentStep = GetCurrentStep())
		{
			PackageProject::ComponentItem::RefVector checkedEntries;
			if (m_ComponentsModel->OnLeaveStep(checkedEntries))
			{
				// Store flags from checked entries and save list of checked entries to current step.
				// This is needed for reverting checked status and correctly resetting flags when going step back.
				StoreStepFlags(checkedEntries);

				// Save checked items to restore check state when going step back
				m_InstallSteps.GetTopItem()->GetChecked() = checkedEntries;

				if (PackageProject::ComponentStep* nextStep = GetFirstStepSatisfiesConditions(*currentStep))
				{
					m_InstallSteps.PushStep(*nextStep);
					LoadManualStep(*nextStep);
				}
				else
				{
					// No steps left, go to install now
					event.Skip();
				}
			}
			return;
		}
		event.Skip();
	}
	
	wxWindow* ComponentsPage::Create()
	{
		/* Create splitters */
		m_SplitterV = new KxSplitterWindow(GetPageContainer(), KxID_NONE);
		m_SplitterV->SetMinimumPaneSize(150);
		m_SplitterV->SetName("VSplitter");
		IThemeManager::GetActive().Apply(m_SplitterV);

		m_SplitterHRight = new KxSplitterWindow(m_SplitterV, KxID_NONE);
		m_SplitterHRight->SetName("HSplitterRight");
		m_SplitterHRight->SetMinimumPaneSize(150);
		IThemeManager::GetActive().Apply(m_SplitterHRight);

		/* Controls */
		// Item list
		m_ComponentsModel = new ComponentsPageNS::DisplayModel();
		m_ComponentsModel->Create(m_SplitterV);
		m_ComponentsModel->SetDataVector();
		m_ComponentsModel->GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &ComponentsPage::OnSelectComponent, this);
		m_ComponentsModel->GetView()->Bind(KxEVT_DATAVIEW_ITEM_HOVERED, &ComponentsPage::OnSelectComponent, this);

		// Image view
		m_ImageView = new KxImageView(m_SplitterHRight, KxID_NONE, wxBORDER_THEME);
		m_ImageView->SetScaleMode(KxIV_SCALE_ASPECT_FIT);
		m_ImageView->Bind(wxEVT_LEFT_DCLICK, [this](wxMouseEvent& event)
		{
			event.Skip();
			if (const PackageProject::ComponentItem* pComponent = m_ComponentsModel->GetHotTrackedEntry())
			{
				const PackageProject::ImageItem* entry = GetPackageConfig().GetInterface().FindImageByPath(pComponent->GetImage());
				if (entry)
				{
					UI::ImageViewerEvent evt;
					evt.SetBitmap(entry->GetBitmap());
					evt.SetDescription(entry->GetDescription());

					UI::ImageViewerDialog dialog(&GetWizard());
					dialog.Navigate(evt);
					dialog.ShowModal();
				}
			}
		});

		// Tabs
		m_TabView = new KxAuiNotebook(m_SplitterHRight, KxID_NONE);
		m_TabView->SetImageList(&ImageProvider::GetImageList());

		// Description
		m_Description = new KxHTMLWindow(m_TabView, KxID_NONE, wxEmptyString, KxHTMLWindow::DefaultStyle|wxBORDER_NONE);
		m_Description->Bind(wxEVT_HTML_LINK_CLICKED, [this](wxHtmlLinkEvent& event)
		{
			Utility::UI::AskOpenURL(event.GetLinkInfo().GetHref(), &GetWizard());
		});
		m_TabView->InsertPage((size_t)TabIndex::Description, m_Description, KTr("Generic.Description"), true);

		// Requirements
		m_RequirementsModel = new RequirementsPageNS::DisplayModel(GetWizard().GetRequirementsPage());
		m_RequirementsModel->CreateView(m_TabView, true);
		m_TabView->InsertPage((size_t)TabIndex::Requirements, m_RequirementsModel->GetView(), KTr("InstallWizard.Page.Requirements"));

		/* Split */
		m_SplitterHRight->SplitHorizontally(m_ImageView, m_TabView, -m_SplitterHRight->GetMinimumPaneSize());
		m_SplitterV->SplitVertically(m_ComponentsModel->GetView(), m_SplitterHRight, m_SplitterV->GetMinimumPaneSize());

		// Return main splitter
		return m_SplitterV;
	}

	void ComponentsPage::ClearDisplay()
	{
		ClearComponentsViewInfo();
		m_ComponentsModel->SetDataVector();
	}
	void ComponentsPage::ResetComponents()
	{
		// Reset steps stack
		m_InstallSteps.Clear();

		// Reset flags
		m_FlagsStorage.clear();

		// Reset entries conditional type descriptors
		for (auto& step: GetPackageConfig().GetComponents().GetSteps())
		{
			for (auto& group: step->GetGroups())
			{
				for (auto& entry: group->GetItems())
				{
					entry->SetTDCurrentValue(PackageProject::TypeDescriptor::Invalid);
				}
			}
		}

		// Clear view
		ClearDisplay();
	}
	bool ComponentsPage::BeginComponents()
	{
		ResetComponents();
		StoreRequirementsFlags();

		if (PackageProject::ComponentStep* firstStep = GetFirstStepSatisfiesConditions())
		{
			m_InstallSteps.PushStep(*firstStep);
			LoadManualStep(*firstStep);
			return true;
		}
		else
		{
			KxTaskDialog(&GetWizard(), KxID_NONE, KTr("InstallWizard.InvalidStepsConfig.Caption"), KTr("InstallWizard.InvalidStepsConfig.Message"), KxBTN_OK, KxICON_ERROR).ShowModal();
			return false;
		}
	}
	void ComponentsPage::StoreRequirementsFlags()
	{
		for (const auto& group: GetPackageConfig().GetRequirements().GetGroups())
		{
			m_FlagsStorage.insert_or_assign(group->GetFlagName(), group->CalcGroupStatus() ? wxS("true") : wxS("false"));
		}
	}

	void ComponentsPage::StoreStepFlags(const PackageProject::ComponentItem::RefVector& checkedEntries)
	{
		for (PackageProject::ComponentItem* entry: checkedEntries)
		{
			for (const PackageProject::FlagItem& flagEntry: entry->GetConditionFlags().GetFlags())
			{
				m_FlagsStorage.insert_or_assign(flagEntry.GetName(), flagEntry.GetValue());
			}
		}
	}
	void ComponentsPage::RestoreStepFlagsUpToThis(const PackageProject::ComponentStep& step)
	{
		m_FlagsStorage.clear();
		StoreRequirementsFlags();

		for (const StepStackItem& stepItem: m_InstallSteps)
		{
			if (stepItem.GetStep() != &step)
			{
				StoreStepFlags(stepItem.GetChecked());
			}
			else
			{
				return;
			}
		}
	}

	bool ComponentsPage::IsConditionSatisfied(const PackageProject::FlagItem& flagEntry) const
	{
		auto it = m_FlagsStorage.find(flagEntry.GetName());
		if (it != m_FlagsStorage.end())
		{
			return it->second == flagEntry.GetValue();
		}
		return !flagEntry.HasValue();
	}
	bool ComponentsPage::IsConditionsSatisfied(const PackageProject::ConditionGroup& conditionGroup) const
	{
		if (conditionGroup.HasConditions())
		{
			PackageProject::ConditionChecker groupChecker;
			for (const PackageProject::Condition& condition: conditionGroup.GetConditions())
			{
				// Evaluate each condition
				PackageProject::ConditionChecker conditionChecker;
				for (const PackageProject::FlagItem& flag: condition.GetFlags())
				{
					conditionChecker(IsConditionSatisfied(flag), condition.GetOperator());
					if (condition.GetOperator() == PackageProject::Operator::And && !conditionChecker.GetResult())
					{
						break;
					}
				}

				// Then combine it
				groupChecker(conditionChecker.GetResult(), conditionGroup.GetOperator());
				if (conditionGroup.GetOperator() == PackageProject::Operator::And && !groupChecker.GetResult())
				{
					break;
				}
			}
			return groupChecker.GetResult();
		}
		return true;
	}
	bool ComponentsPage::IsStepSatisfiesConditions(const PackageProject::ComponentStep& step) const
	{
		return IsConditionsSatisfied(step.GetConditionGroup());
	}
	bool ComponentsPage::CheckIsManualComponentsAvailable() const
	{
		const PackageProject::ComponentsSection& components = GetPackageConfig().GetComponents();
		if (!components.GetSteps().empty())
		{
			// Iterate over all steps from the beginning
			// and if at least one step satisfies conditions return true.
			for (const auto& step: components.GetSteps())
			{
				if (IsStepSatisfiesConditions(*step))
				{
					return true;
				}
			}
		}
		return false;
	}
	PackageProject::ComponentStep* ComponentsPage::GetFirstStepSatisfiesConditions() const
	{
		for (const auto& step: GetPackageConfig().GetComponents().GetSteps())
		{
			if (IsStepSatisfiesConditions(*step))
			{
				return step.get();
			}
		}
		return nullptr;
	}
	PackageProject::ComponentStep* ComponentsPage::GetFirstStepSatisfiesConditions(const PackageProject::ComponentStep& afterThis) const
	{
		const PackageProject::ComponentStep::Vector& steps = GetPackageConfig().GetComponents().GetSteps();
		const auto itAfterThis = std::find_if(steps.begin(), steps.end(), [&afterThis](const auto& step)
		{
			return step.get() == &afterThis;
		});

		const auto itNextAfterThis = itAfterThis + 1;
		if (itAfterThis != steps.end() && itNextAfterThis != steps.end())
		{
			for (auto it = itNextAfterThis; it != steps.end(); ++it)
			{
				PackageProject::ComponentStep& currentStep = **it;
				if (IsStepSatisfiesConditions(currentStep))
				{
					return &currentStep;
				}
			}
		}
		return nullptr;
	}

	PackageProject::ComponentStep* ComponentsPage::GetCurrentStep() const
	{
		return m_InstallSteps.GetTopStep();
	}
	const StepStackItem* ComponentsPage::GetCurrentStepItem() const
	{
		return m_InstallSteps.GetTopItem();
	}
	void ComponentsPage::LoadManualStep(PackageProject::ComponentStep& step)
	{
		// Set step name if any
		if (!step.GetName().IsEmpty())
		{
			GetWizard().SetLabel(step.GetName());
		}
		else
		{
			GetWizard().SetLabel(GetCaption());
		}

		// Check entries conditions and set its type descriptors current value
		for (auto& group: step.GetGroups())
		{
			for (auto& item: group->GetItems())
			{
				item->SetTDCurrentValue(PackageProject::TypeDescriptor::Invalid);
				if (IsConditionsSatisfied(item->GetTDConditionGroup()))
				{
					item->SetTDCurrentValue(item->GetTDConditionalValue());
				}
			}
		}

		m_ComponentsModel->SetDataVector(&GetPackageConfig().GetComponents(), &step, GetCurrentStepItem()->GetChecked());
		m_ComponentsModel->GetView()->SetFocus();
	}
}
