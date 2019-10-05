#include "stdafx.h"
#include "InstallationPage.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include <Kortex/InstallWizard.hpp>
#include <Kortex/ModManager.hpp>
#include <KxFramework/KxTaskDialog.h>

using namespace Kortex::PackageDesigner;
namespace Kortex::InstallWizard
{
	void InstallationPage::OnLoadUIOptions(const Application::ActiveInstanceOption& option)
	{
	}
	void InstallationPage::OnSaveUIOptions(Application::ActiveInstanceOption& option) const
	{
	}
	void InstallationPage::OnPackageLoaded()
	{
	}

	bool InstallationPage::OnOpenPage()
	{
		WizardDialog& wizard = GetWizard();

		if (wizard.IsOptionEnabled(DialogOptions::Debug))
		{
			CollectAllInstallableEntries();
			ShowInstallableFilesPreview();

			// Switch back to info page to reset components state.
			wizard.SwitchPage(WizardPageID::Info);
			return false;
		}
		else if (!OnBeginInstall())
		{
			wizard.Close(true);
			return false;
		}
		return true;
	}
	bool InstallationPage::OnClosePage()
	{
		m_IsComplete = true;
		return true;
	}

	bool InstallationPage::OnBeginInstall()
	{
		CollectAllInstallableEntries();
		if (!m_InstallableFiles.empty())
		{
			m_InstallThread = new InstallOperation(*this);
			m_InstallThread->OnRun([this](KOperationWithProgressBase* self)
			{
				self->LinkHandler(&GetWizard().GetArchive(), KxEVT_ARCHIVE);
				GetWizard().RunInstall();
			});
			m_InstallThread->OnEnd([this](KOperationWithProgressBase* self)
			{
				m_InstallThread = nullptr;
				OnEndInstall();
			});

			m_InstallThread->Run();
			return true;
		}
		else
		{
			KxTaskDialog dialog(&GetWizard(), KxID_NONE, KTr("InstallWizard.NoFiles.Caption"), KTr("InstallWizard.NoFiles.Message"), KxBTN_OK, KxICON_ERROR);
			dialog.SetOptionEnabled(KxTD_HYPERLINKS_ENABLED);
			dialog.ShowModal();
			return false;
		}
	}
	bool InstallationPage::OnEndInstall()
	{
		auto mod = GetWizard().TakeMod();
		if (mod->Save())
		{
			// Save main or header image
			const KPackageProjectInterface& interfaceConfig = GetPackageConfig().GetInterface();
			const KPPIImageEntry* imageEntry = interfaceConfig.GetMainImageEntry();
			imageEntry = imageEntry ? imageEntry : interfaceConfig.GetHeaderImageEntry();
			if (imageEntry && imageEntry->HasBitmap())
			{
				const wxBitmap& bitmap = imageEntry->GetBitmap();
				bitmap.SaveFile(mod->GetImageFile(), bitmap.HasAlpha() ? wxBITMAP_TYPE_PNG : wxBITMAP_TYPE_JPEG);
			}

			// Add mod to manager
			IGameMod& modRef = IModManager::GetInstance()->EmplaceMod(std::move(mod));

			BroadcastProcessor::Get().QueueEvent(ModEvent::EvtInstalled, modRef);
			if (ShouldCancel())
			{
				// We were canceled, but mod is partially installed.
				// Just exit immediately, user can uninstall this.
				GetWizard().Destroy();
			}
			else
			{
				GetWizard().SwitchPage(WizardPageID::Completed);
			}
			return true;
		}
		return false;
	}
	void InstallationPage::OnMinorProgress(KxFileOperationEvent& event)
	{
		if (m_ShouldCancel)
		{
			event.Veto();
			return;
		}

		m_MinorStatus->SetLabel(event.GetCurrent());
		if (!event.GetSource().IsEmpty())
		{
			m_MajorStatus->SetLabel(event.GetSource());
		}

		int64_t minorMin = event.GetMinorProcessed();
		int64_t minorMax = event.GetMinorTotal();
		if (event.IsMinorKnown())
		{
			m_MinorProgress->SetValue(minorMin, minorMax);
		}
		else if (minorMin != -2 && minorMax != -2)
		{
			m_MinorProgress->Pulse();
		}

		if (event.IsMajorKnown())
		{
			m_MajorProgress->SetValue(event.GetMajorProcessed(), event.GetMajorTotal());
		}
		else
		{
			m_MajorProgress->Pulse();
		}
	}
	void InstallationPage::OnMajorProgress(KxFileOperationEvent& event)
	{
		if (m_ShouldCancel)
		{
			event.Veto();
			return;
		}

		int64_t current = event.GetMajorProcessed();
		int64_t max = event.GetMajorTotal();
		m_MajorStatus->SetLabel(KTrf("InstallWizard.InstalledXOfY", current, max) + wxS(". ") + event.GetSource());
	}

	wxWindow* InstallationPage::Create()
	{
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_PagePanel = new KxPanel(GetPageContainer(), KxID_NONE);
		m_PagePanel->SetSizer(sizer);

		m_MinorProgress = new KxProgressBar(m_PagePanel, KxID_NONE, 100);
		m_MajorProgress = new KxProgressBar(m_PagePanel, KxID_NONE, 100);
		m_MinorStatus = KPackageCreatorPageBase::CreateNormalLabel(m_PagePanel, wxEmptyString, false);
		m_MajorStatus = KPackageCreatorPageBase::CreateNormalLabel(m_PagePanel, wxEmptyString, false);

		sizer->Add(m_MinorStatus, 0, wxEXPAND);
		sizer->Add(m_MinorProgress, 0, wxEXPAND);
		sizer->AddSpacer(15);
		sizer->Add(m_MajorStatus, 0, wxEXPAND);
		sizer->Add(m_MajorProgress, 0, wxEXPAND);

		Bind(KxEVT_ARCHIVE, &InstallationPage::OnMajorProgress, this);
		return m_PagePanel;
	}

	void InstallationPage::CollectAllInstallableEntries()
	{
		auto AddFilesFromList = [this](const KxStringVector& list, bool pushBack = true)
		{
			for (const wxString& id: list)
			{
				KPPFFileEntry* entry = GetPackageConfig().GetFileData().FindEntryWithID(id);
				if (entry)
				{
					if (pushBack)
					{
						m_InstallableFiles.push_back(entry);
					}
					else
					{
						m_InstallableFiles.insert(m_InstallableFiles.begin(), entry);
					}
				}
			}
		};
		auto UniqueFiles = [this]()
		{
			m_InstallableFiles.erase(std::unique(m_InstallableFiles.begin(), m_InstallableFiles.end()), m_InstallableFiles.end());
		};

		m_InstallableFiles.clear();

		// No manual install steps and no conditional install.
		// Saves list of all files in the package.
		const ComponentsPage& componentsPage = GetWizard().GetComponentsPage();
		if (!componentsPage.HasManualComponents() && !componentsPage.HasConditionalInstall())
		{
			for (const auto& entry: GetPackageConfig().GetFileData().GetData())
			{
				m_InstallableFiles.push_back(entry.get());
			}
			SortInstallableFiles();
		}
		else
		{
			// Manual steps present, get files from checked entries
			if (componentsPage.HasManualComponents())
			{
				for (const StepStackItem& step: componentsPage.GetInstallSteps())
				{
					for (const KPPCEntry* entry: step.GetChecked())
					{
						AddFilesFromList(entry->GetFileData());
					}
				}
			}

			// Conditional install present. Run it and store files.
			if (componentsPage.HasConditionalInstall())
			{
				for (const auto& step: GetPackageConfig().GetComponents().GetConditionalSteps())
				{
					if (componentsPage.IsConditionsSatisfied(step->GetConditionGroup()))
					{
						AddFilesFromList(step->GetEntries());
					}
				}
			}

			// Sort all files excluding required
			SortInstallableFiles();

			// Add required files to the beginning
			AddFilesFromList(GetPackageConfig().GetComponents().GetRequiredFileData(), false);
		}

		// Remove duplicates
		UniqueFiles();
	}
	void InstallationPage::SortInstallableFiles()
	{
		// Sort all files by its priorities.
		// Leave all files with default priority (-1) at end.
		// So split all files into two arrays: first with assigned priorities and second with default.
		// Sort arrays with non-default priorities.
		// Merge the two arrays. First sorted, then unsorted.

		KPPFFileEntryRefArray defaultPriority;
		KPPFFileEntryRefArray nonDefaultPriority;
		for (KPPFFileEntry* entry: m_InstallableFiles)
		{
			if (entry->IsDefaultPriority())
			{
				defaultPriority.push_back(entry);
			}
			else
			{
				nonDefaultPriority.push_back(entry);
			}
		}

		// Sort non-default
		std::sort(nonDefaultPriority.begin(), nonDefaultPriority.end(), [](const KPPFFileEntry* entry1, const KPPFFileEntry* entry2)
		{
			return entry1->GetPriority() < entry2->GetPriority();
		});

		// Merge back
		m_InstallableFiles.clear();
		for (KPPFFileEntry* entry: nonDefaultPriority)
		{
			m_InstallableFiles.push_back(entry);
		}
		for (KPPFFileEntry* entry: defaultPriority)
		{
			m_InstallableFiles.push_back(entry);
		}
	}
	void InstallationPage::ShowInstallableFilesPreview()
	{
		KxStringVector files;
		files.reserve(m_InstallableFiles.size());

		for (const auto& fileEntry: m_InstallableFiles)
		{
			if (fileEntry->GetID() != fileEntry->GetSource())
			{
				files.push_back(wxString::Format("%s (%s)", fileEntry->GetID(), fileEntry->GetSource()));
			}
			else
			{
				files.push_back(fileEntry->GetID());
			}
		}

		KxTaskDialog dialog(&GetWizard(), KxID_NONE, KTr("InstallWizard.CollectedFiles.Caption"), KxString::Join(files, "\r\n"), KxBTN_OK, KxICON_INFORMATION);
		dialog.ShowModal();
	}
}
