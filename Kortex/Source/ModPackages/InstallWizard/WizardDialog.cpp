#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include "Common.h"
#include "WizardDialog.h"
#include "PackageCreator/PageBase.h"
#include "UI/ImageViewerDialog.h"
#include "UI/TextEditDialog.h"
#include "Utility/KAux.h"
#include "Utility/Log.h"
#include <KxFramework/KxImageView.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxArchiveFileFinder.h>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxString.h>

using namespace Kortex::PackageDesigner;
namespace Kortex::InstallWizard
{
	wxDEFINE_EVENT(KEVT_IW_DONE, wxNotifyEvent);
}
namespace Kortex::InstallWizard
{
	auto GetUIOption(const wxString& option = {})
	{
		return Application::GetAInstanceOptionOf<IPackageManager>(wxS("InstallWizard"), option);
	};
}

namespace Kortex::InstallWizard
{
	void WizardDialog::ShowInvalidPackageDialog(wxWindow* window, const wxString& packagePath)
	{
		KxTaskDialog dialog(window, KxID_NONE, KTrf("InstallWizard.LoadFailed.Caption", packagePath), KTr("InstallWizard.LoadFailed.Message"), KxBTN_OK, KxICON_ERROR);
		dialog.ShowModal();
	}

	bool WizardDialog::CreateUI(wxWindow* parent)
	{
		SetDefaultBackgroundColor();
		if (KxStdDialog::Create(parent, KxID_NONE, "Install Wizard", wxDefaultPosition, IMainWindow::GetInstance()->GetFrame().GetMinSize(), KxBTN_NONE, KxStdDialog::DefaultStyle))
		{
			GetContentWindow()->SetBackgroundColour(GetBackgroundColour());

			m_BackwardButton = AddButton(KxID_BACKWARD, m_BackwardDefaultLabel).As<KxButton>();
			m_BackwardButton->Bind(wxEVT_BUTTON, &WizardDialog::OnGoBackward, this);
			m_BackwardButton->Bind(wxEVT_BUTTON, &ComponentsPage::OnGoStepBackward, &m_PageComponents);

			m_ForwardButton = AddButton(KxID_FORWARD, m_ForwardDefaultLabel).As<KxButton>();
			m_ForwardButton->Bind(wxEVT_BUTTON, &WizardDialog::OnGoForward, this);
			m_ForwardButton->Bind(wxEVT_BUTTON, &ComponentsPage::OnGoStepForward, &m_PageComponents);

			m_CancelButton = AddButton(KxID_CANCEL, m_CancelDefaultLabel).As<KxButton>();
			m_CancelButton->Bind(wxEVT_BUTTON, &WizardDialog::OnCancelButton, this);
			Bind(wxEVT_CLOSE_WINDOW, &WizardDialog::OnClose, this);
			SetCloseIDs({});

			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide(wxBOTH);
			EnableMinimizeButton();
			EnableMaximizeButton();

			m_PageContainer = new wxSimplebook(m_ContentPanel, KxID_NONE, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
			for (WizardPage* page: GetPages())
			{
				m_PageContainer->AddPage(page->Create(), page->GetCaption());
			}

			PostCreate();
			return true;
		}
		return false;
	}
	void WizardDialog::LoadUIOptions()
	{
		GetUIOption().LoadWindowGeometry(this);
		Center();

		for (WizardPage* page: GetPages())
		{
			page->OnLoadUIOptions(GetUIOption(page->GetOptionName()));
		}
	}

	bool WizardDialog::DoSwitchPage(WizardPage& targetPage)
	{
		if (&targetPage != m_CurrentPage && m_CurrentPage->OnClosePage())
		{
			if (targetPage.OnOpenPage())
			{
				auto ProcessButton = [](KxButton* button, const WizardButton& config, const wxString& defaultLabel)
				{
					button->SetLabel(config.HasLabel() ? config.GetLabel() : defaultLabel);
					button->Enable(config.IsEnabled());
				};
				ProcessButton(m_CancelButton, targetPage.GetCancelButton(), m_CancelDefaultLabel);
				ProcessButton(m_BackwardButton, targetPage.GetBackwardButton(), m_BackwardDefaultLabel);
				ProcessButton(m_ForwardButton, targetPage.GetForwardButton(), m_ForwardDefaultLabel);

				m_CurrentPage = &targetPage;
				m_PageContainer->ChangeSelection((size_t)targetPage.GetID());
				return true;
			}
		}
		return false;
	}

	void WizardDialog::OpenPackage(const wxString& packagePath)
	{
		auto thread = new KOperationWithProgressDialog<KxArchiveEvent>(true, GetParent());
		thread->OnRun([this, packagePath = packagePath.Clone()](KOperationWithProgressBase* self)
		{
			self->LinkHandler(&m_Package->GetArchive(), KxEVT_ARCHIVE);
			if (!packagePath.IsEmpty())
			{
				m_Package->Create(packagePath);
			}
			m_Package->LoadResources();
		});
		thread->OnEnd([this](KOperationWithProgressBase* self)
		{
			ProcessLoadPackage();
		});
		thread->SetDialogCaption(packagePath.AfterLast('\\'));
		thread->Run();
	}
	bool WizardDialog::LoadPackage()
	{
		if (m_Package->IsOK() && m_Package->IsTypeSupported())
		{
			const PackageProject::KPackageProjectInfo& info = GetConfig().GetInfo();
			const PackageProject::KPackageProjectInterface& interfaceConfig = GetConfig().GetInterface();

			// Window caption
			SetCaption(KTrf("InstallWizard.WindowCaption", m_Package->GetName()) + ' ' + info.GetVersion());

			// Try to find existing mod for this package
			m_Mod = IModManager::GetInstance()->NewMod();
			FindExistingMod();
			if (m_ExistingMod)
			{
				AcceptExistingMod(*m_ExistingMod);
			}

			// Header
			LoadHeaderImage();

			for (WizardPage* page: GetPages())
			{
				page->OnPackageLoaded();
			}
			LoadUIOptions();
			
			// Manually open info page
			m_CurrentPage = &m_PageInfo;
			return m_CurrentPage->OnOpenPage();
		}
		else
		{
			ShowInvalidPackageDialog(this, m_Package->GetPackageFilePath());
			Close(true);
			return false;
		}
	}
	bool WizardDialog::ProcessLoadPackage()
	{
		if (LoadPackage())
		{
			SwitchPage(WizardPageID::Info);
			Show();
			Raise();

			BroadcastProcessor::Get().ProcessEvent(ModEvent::EvtInstalling, *m_Mod);
			return true;
		}
		return false;
	}

	void WizardDialog::FindExistingMod()
	{
		m_ExistingMod = IModManager::GetInstance()->FindModByID(GetConfig().GetModID());
	}
	void WizardDialog::AcceptExistingMod(const IGameMod& mod)
	{
		// Info
		PackageProject::KPackageProjectInfo& packageInfo = GetConfig().GetInfo();

		// Tags
		packageInfo.GetTagStore() = mod.GetTagStore();

		// Other info
		if (packageInfo.GetName().IsEmpty())
		{
			packageInfo.SetName(mod.GetName());
		}
		if (packageInfo.GetModSourceStore().IsEmpty())
		{
			packageInfo.GetModSourceStore() = mod.GetModSourceStore();
		}

		// Linked mod configuration
		if (mod.IsLinkedMod())
		{
			m_Mod->LinkLocation(mod.GetModFilesDir());
		}
	}
	void WizardDialog::LoadHeaderImage()
	{
		const PackageProject::KPackageProjectInterface& interfaceConfig = m_Package->GetConfig().GetInterface();
		if (const PackageProject::KPPIImageEntry* pHeaderImage = interfaceConfig.GetHeaderImageEntry())
		{
			if (pHeaderImage->HasBitmap())
			{
				KxImageView* pHeaderView = new KxImageView(this, KxID_NONE, KxImageView::DefaultStyle|wxBORDER_NONE);
				wxSize tSize = pHeaderImage->GetSize();
				tSize.SetHeight(pHeaderImage->GetBitmap().GetHeight());
				tSize.DecToIfSpecified(wxSize(wxDefaultCoord, 45));

				pHeaderView->SetMinSize(tSize);
				pHeaderView->SetMaxSize(tSize);
				GetSizer()->Prepend(pHeaderView, 0, wxEXPAND);

				pHeaderView->SetScaleMode(KxIV_SCALE_ASPECT_FILL);
				pHeaderView->SetBitmap(pHeaderImage->GetBitmap());
			}
		}
		SetAutoSize(false);
	}

	bool WizardDialog::AskCancel(bool canCancel)
	{
		if (canCancel)
		{
			if (m_CurrentPage->GetID() != WizardPageID::Components && m_CurrentPage->GetID() != WizardPageID::Installation)
			{
				return true;
			}

			KxTaskDialog dialog(this, KxID_NONE, KTr("InstallWizard.CancelMessage"), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
			return dialog.ShowModal() == KxID_YES;
		}
		return true;
	}
	void WizardDialog::OnClose(wxCloseEvent& event)
	{
		if (!m_PageInstallation.IsCompleted())
		{
			m_PageInstallation.m_ShouldCancel = AskCancel(event.CanVeto());
			if (m_PageInstallation.m_InstallThread)
			{
				m_PageInstallation.m_InstallThread->Stop();
				event.Skip(false);
				event.Veto(true);
			}
			else
			{
				event.Veto(!m_PageInstallation.m_ShouldCancel);
				event.Skip(m_PageInstallation.m_ShouldCancel);
			}
		}
		else
		{
			wxNotifyEvent doneEvent(KEVT_IW_DONE);
			doneEvent.SetEventObject(this);
			HandleWindowEvent(doneEvent);

			event.Skip();
		}
	}
	void WizardDialog::OnCancelButton(wxCommandEvent& event)
	{
		Close();
	}
	void WizardDialog::OnGoBackward(wxCommandEvent& event)
	{
		WizardPageID page = WizardPageID::None;
		switch (m_CurrentPage->GetID())
		{
			case WizardPageID::Requirements:
			{
				page = WizardPageID::Info;
				break;
			}
			case WizardPageID::Components:
			{
				page = m_PageRequirements.HasMainRequirements() ? WizardPageID::Requirements : WizardPageID::Info;
				break;
			}
		};
		SwitchPage(page);
	}
	void WizardDialog::OnGoForward(wxCommandEvent& event)
	{
		if (m_PageInstallation.IsCompleted())
		{
			Close(true);
		}
		else
		{
			WizardPageID page = WizardPageID::None;
			switch (m_CurrentPage->GetID())
			{
				case WizardPageID::Info:
				{
					page = m_PageRequirements.HasMainRequirements() ? WizardPageID::Requirements : WizardPageID::Components;
					if (page == WizardPageID::Components && !m_PageComponents.HasManualComponents())
					{
						page = WizardPageID::Installation;
					}
					break;
				}
				case WizardPageID::Requirements:
				{
					page = m_PageComponents.HasManualComponents() ? WizardPageID::Components : WizardPageID::Installation;
					break;
				}
				case WizardPageID::Components:
				{
					page = WizardPageID::Installation;
					break;
				}
			};
			SwitchPage(page);
		}
	}

	void WizardDialog::SetModData()
	{
		m_Mod->CreateFromProject(GetConfig());
		m_Mod->SetInstallTime(wxDateTime::Now());
		m_Mod->SetPackageFile(m_Package->GetPackageFilePath());
	}
	KxUInt32Vector WizardDialog::GetFilesOfFolder(const PackageProject::KPPFFolderEntry* folder) const
	{
		KxUInt32Vector indexes;
		const wxString path = folder->GetSource();

		KxArchiveFileFinder finder(GetArchive(), path, wxS('*'));
		KxFileItem item = finder.FindNext();
		while (item.IsOK())
		{
			indexes.push_back(item.GetExtraData<size_t>());
			item = finder.FindNext();
		}
		return indexes;
	}
	wxString WizardDialog::GetFinalPath(uint32_t index, const wxString& installLocation, const PackageProject::KPPFFileEntry* fileEntry) const
	{
		// Remove "in archive" source path from final file path
		wxString path = GetArchive().GetItemName(index).Remove(0, fileEntry->GetSource().Length());
		if (!path.IsEmpty() && path[0] == wxS('\\'))
		{
			path.Remove(0, 1);
		}

		// Perpend destination path if needed
		const wxString& destination = fileEntry->GetDestination();
		if (!fileEntry->GetDestination().IsEmpty())
		{
			if (!destination.IsEmpty() && destination[0] != wxS('\\'))
			{
				path.Prepend(wxS('\\'));
			}
			path.Prepend(fileEntry->GetDestination());
		}

		return installLocation + wxS('\\') + path;
	}
	KxStringVector WizardDialog::GetFinalPaths(const KxUInt32Vector& filePaths, const wxString& installLocation, const PackageProject::KPPFFolderEntry* folder) const
	{
		KxStringVector finalPaths;
		for (uint32_t index : filePaths)
		{
			finalPaths.emplace_back(GetFinalPath(index, installLocation, folder));
		}
		return finalPaths;
	}
	void WizardDialog::RunInstall()
	{
		SetModData();

		wxString installLocation = m_Mod->GetModFilesDir();
		if (installLocation.Last() == '\\')
		{
			installLocation.RemoveLast(1);
		}

		auto NotifyMajor = [this](size_t current, size_t max, const wxString& status)
		{
			KxFileOperationEvent* event = new KxFileOperationEvent(KxEVT_ARCHIVE);
			event->SetEventObject(this);
			event->SetSource(status.Clone());
			event->SetMajorProcessed(current);
			event->SetMajorTotal(max);
			QueueEvent(event);
		};

		size_t processed = 0;
		const PackageProject::KPPFFileEntryRefArray& installableFiles = m_PageInstallation.GetInstallableFiles();
		for (const PackageProject::KPPFFileEntry* fileEntry: installableFiles)
		{
			if (m_PageInstallation.m_InstallThread->CanContinue())
			{
				NotifyMajor(processed, installableFiles.size(), fileEntry->GetSource());

				if (const PackageProject::KPPFFolderEntry* folderEntry = fileEntry->ToFolderEntry())
				{
					KxUInt32Vector filesIndexes = GetFilesOfFolder(folderEntry);
					KxStringVector finalPaths = GetFinalPaths(filesIndexes, installLocation, folderEntry);
					GetArchive().ExtractToFiles(filesIndexes, finalPaths);
				}
				else
				{
					KxFileItem item;
					if (GetArchive().FindFile(fileEntry->GetSource(), item))
					{
						wxString path = installLocation + wxS('\\') + fileEntry->GetDestination();
						GetArchive().ExtractToFiles({item.GetExtraData<uint32_t>()}, {path});
					}
				}

				processed++;
				NotifyMajor(processed, installableFiles.size(), fileEntry->GetSource());
			}
			else
			{
				return;
			}
		}
	}

	WizardDialog::WizardDialog()
		:m_PageInfo(*this), m_PageRequirements(*this), m_PageComponents(*this), m_PageInstallation(*this), m_PageCompleted(*this),
		m_CancelDefaultLabel(KTr(KxID_CANCEL)),
		m_BackwardDefaultLabel(wxS("< ") + KTr("InstallWizard.BackwardButton")),
		m_ForwardDefaultLabel(KTr("InstallWizard.ForwardButton") + wxS(" >"))
	{
	}
	WizardDialog::WizardDialog(wxWindow* parent, const wxString& packagePath)
		:WizardDialog()
	{
		Create(parent, packagePath);
	}
	bool WizardDialog::Create(wxWindow* parent, const wxString& packagePath)
	{
		m_Package = std::make_unique<ModPackage>();
		if (CreateUI(parent))
		{
			OpenPackage(packagePath);
			return true;
		}
		return false;
	}
	bool WizardDialog::Create(wxWindow* parent, std::unique_ptr<ModPackage> package)
	{
		m_Package = std::move(package);
		if (CreateUI(parent))
		{
			OpenPackage(wxEmptyString);
			return true;
		}
		return false;
	}

	WizardDialog::~WizardDialog()
	{
		if (IsOptionEnabled(DialogOptions::Cleanup))
		{
			KxFile(m_Package->GetPackageFilePath()).RemoveFile();
		}

		if (m_Package->IsOK())
		{
			GetUIOption().SaveWindowGeometry(this);
			for (const WizardPage* page: GetPages())
			{
				page->OnSaveUIOptions(GetUIOption(page->GetOptionName()));
			}
		}
	}

	bool WizardDialog::SwitchPage(WizardPage& targetPage)
	{
		return DoSwitchPage(targetPage);
	}
	bool WizardDialog::SwitchPage(WizardPageID targetPage)
	{
		if ((int)targetPage >= 0 && (size_t)targetPage < m_PageContainer->GetPageCount())
		{
			switch (targetPage)
			{
				case WizardPageID::Info:
				{
					return DoSwitchPage(m_PageInfo);
				}
				case WizardPageID::Requirements:
				{
					return DoSwitchPage(m_PageRequirements);
				}
				case WizardPageID::Components:
				{
					return DoSwitchPage(m_PageComponents);
				}
				case WizardPageID::Installation:
				{
					return DoSwitchPage(m_PageInstallation);
				}
				case WizardPageID::Completed:
				{
					return DoSwitchPage(m_PageCompleted);
				}
			};
		}
		return false;
	}
}
