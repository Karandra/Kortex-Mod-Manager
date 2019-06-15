#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Events.hpp>
#include "Common.h"
#include "WizardDialog.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include "UI/KMainWindow.h"
#include "UI/KImageViewerDialog.h"
#include "UI/KTextEditorDialog.h"
#include "Utility/KAux.h"
#include "Utility/Log.h"
#include <KxFramework/KxImageView.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxArchiveFileFinder.h>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxString.h>

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
		if (KxStdDialog::Create(parent, KxID_NONE, "Install Wizard", wxDefaultPosition, KMainWindow::GetInstance()->GetMinSize(), KxBTN_NONE, KxStdDialog::DefaultStyle))
		{
			GetContentWindow()->SetBackgroundColour(GetBackgroundColour());

			m_BackwardButton = AddButton(KxID_BACKWARD, "< " + KTr("InstallWizard.BackwardButton")).As<KxButton>();
			m_BackwardButton->Bind(wxEVT_BUTTON, &WizardDialog::OnGoBackward, this);
			m_BackwardButton->Bind(wxEVT_BUTTON, &ComponentsPage::OnGoStepBackward, &m_PageComponents);

			m_ForwardButton = AddButton(KxID_FORWARD, KTr("InstallWizard.ForwardButton") + " >").As<KxButton>();
			m_ForwardButton->Bind(wxEVT_BUTTON, &WizardDialog::OnGoForward, this);
			m_ForwardButton->Bind(wxEVT_BUTTON, &ComponentsPage::OnGoStepForward, &m_PageComponents);

			m_CancelButton = AddButton(KxID_CANCEL).As<KxButton>();
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
			m_PageContainer->AddPage(CreateUI_Done(), wxEmptyString);

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

	wxWindow* WizardDialog::CreateUI_Done()
	{
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_Done_Pane = new KxPanel(m_PageContainer, KxID_NONE);
		m_Done_Pane->SetSizer(sizer);

		m_Done_Label = KPackageCreatorPageBase::CreateCaptionLabel(m_Done_Pane, wxEmptyString);
		m_Done_Label->ToggleWindowStyle(KxLABEL_LINE);
		m_Done_Label->SetForegroundColour(m_Done_Pane->GetForegroundColour());

		sizer->AddSpacer(25);
		sizer->Add(m_Done_Label, 0, wxEXPAND|wxLEFT, 50);
		return m_Done_Pane;
	}

	void WizardDialog::SetLabelByCurrentPage()
	{
		switch (m_CurrentPage)
		{
			case WizardPageID::Info:
			{
				SetLabel(KTr("InstallWizard.Page.Info"));
				break;
			}
			case WizardPageID::Requirements:
			{
				SetLabel(KTr("InstallWizard.Page.Requirements"));
				break;
			}
			case WizardPageID::Components:
			{
				SetLabel(KTr("InstallWizard.Page.Components"));
				break;
			}
			case WizardPageID::Installation:
			{
				SetLabel(KTr("InstallWizard.Page.Installing"));
				break;
			}
			case WizardPageID::Done:
			{
				SetLabel(KTr("InstallWizard.Page.Done"));
				break;
			}
		};
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
			const KPackageProjectInfo& info = GetConfig().GetInfo();
			const KPackageProjectInterface& interfaceConfig = GetConfig().GetInterface();

			// Window caption
			SetCaption(KTrf("InstallWizard.WindowCaption", m_Package->GetName()) + ' ' + info.GetVersion());

			// Try to find existing mod for this package
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

			return true;
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
		bool ret = LoadPackage();
		SwitchPage(WizardPageID::Info);

		Show();
		Raise();

		ModManager::ModEvent(Events::ModInstalling, m_ModEntry).Send();

		return ret;
	}

	void WizardDialog::FindExistingMod()
	{
		m_ExistingMod = Kortex::IModManager::GetInstance()->FindModByID(GetConfig().GetModID());
	}
	void WizardDialog::AcceptExistingMod(const IGameMod& mod)
	{
		// Info
		KPackageProjectInfo& packageInfo = GetConfig().GetInfo();

		// Tags
		packageInfo.GetTagStore() = mod.GetTagStore();
		m_ModEntry.SetPriorityGroupTag(mod.GetPriorityGroupTag());

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
			m_ModEntry.LinkLocation(mod.GetModFilesDir());
		}
	}
	void WizardDialog::LoadHeaderImage()
	{
		const KPackageProjectInterface& interfaceConfig = m_Package->GetConfig().GetInterface();
		if (const KPPIImageEntry* pHeaderImage = interfaceConfig.GetHeaderImageEntry())
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
			if (m_CurrentPage != WizardPageID::Components && m_CurrentPage != WizardPageID::Installation)
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
		switch (m_CurrentPage)
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
			switch (m_CurrentPage)
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
		m_ModEntry.CreateFromProject(GetConfig());
		m_ModEntry.SetInstallTime(wxDateTime::Now());
		m_ModEntry.SetPackageFile(m_Package->GetPackageFilePath());
		m_ModEntry.CreateAllFolders();
	}
	KxUInt32Vector WizardDialog::GetFilesOfFolder(const KPPFFolderEntry* folder) const
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
	wxString WizardDialog::GetFinalPath(uint32_t index, const wxString& installLocation, const KPPFFileEntry* fileEntry) const
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
	KxStringVector WizardDialog::GetFinalPaths(const KxUInt32Vector& filePaths, const wxString& installLocation, const KPPFFolderEntry* folder) const
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

		wxString installLocation = m_ModEntry.GetModFilesDir();
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
		const KPPFFileEntryRefArray& installableFiles = m_PageInstallation.GetInstallableFiles();
		for (const KPPFFileEntry* fileEntry: installableFiles)
		{
			if (m_PageInstallation.m_InstallThread->CanContinue())
			{
				NotifyMajor(processed, installableFiles.size(), fileEntry->GetSource());

				if (const KPPFFolderEntry* folderEntry = fileEntry->ToFolderEntry())
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
		:m_PageInfo(*this),
		m_PageRequirements(*this),
		m_PageComponents(*this),
		m_PageInstallation(*this)
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
	bool WizardDialog::Create(wxWindow* parent, std::unique_ptr<ModPackage>&& package)
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

	void WizardDialog::SwitchPage(WizardPageID page)
	{
		if (page != m_CurrentPage && page != WizardPageID::None && page < (WizardPageID)m_PageContainer->GetPageCount())
		{
			if (!OnLeavingPage(m_CurrentPage))
			{
				return;
			}

			switch (page)
			{
				case WizardPageID::Info:
				{
					m_BackwardButton->Disable();
					m_ForwardButton->Enable();

					m_CurrentPage = page;
					SetLabelByCurrentPage();
					m_PageContainer->ChangeSelection((size_t)page);
					break;
				}
				case WizardPageID::Requirements:
				{
					m_BackwardButton->Enable();
					m_ForwardButton->Enable();

					m_CurrentPage = page;
					SetLabelByCurrentPage();
					m_PageContainer->ChangeSelection((size_t)page);
					break;
				}
				case WizardPageID::Components:
				{
					m_BackwardButton->Enable();
					m_ForwardButton->Enable();

					m_CurrentPage = page;
					SetLabelByCurrentPage();

					if (!m_PageComponents.BeginComponents())
					{
						Close(true);
						break;
					}

					m_PageContainer->ChangeSelection((size_t)page);
					break;
				}
				case WizardPageID::Installation:
				{
					if (IsOptionEnabled(DialogOptions::Debug))
					{
						m_PageInstallation.CollectAllInstallableEntries();
						m_PageInstallation.ShowInstallableFilesPreview();

						// Switch back to info page to reset components state.
						SwitchPage(WizardPageID::Info);
						break;
					}

					if (!m_PageInstallation.OnBeginInstall())
					{
						Close(true);
						break;
					}

					m_BackwardButton->Disable();
					m_ForwardButton->Disable();

					m_CurrentPage = page;
					SetLabelByCurrentPage();
					m_PageContainer->ChangeSelection((size_t)page);
					break;
				}
				case WizardPageID::Done:
				{
					m_PageInstallation.m_IsComplete = true;

					m_CancelButton->Disable();
					m_BackwardButton->Disable();
					m_ForwardButton->Enable();

					m_ForwardButton->SetLabel(KTr(KxID_CLOSE));
					m_Done_Label->SetLabel(KTrf("InstallWizard.InstallationComplete", m_Package->GetName()));

					m_CurrentPage = page;
					SetLabelByCurrentPage();
					m_PageContainer->ChangeSelection((size_t)page);
					break;
				}
			};

			if (page != WizardPageID::Info)
			{
				m_InfoPageLeft = true;
			}
		}
	}
	bool WizardDialog::OnLeavingPage(WizardPageID page)
	{
		switch (page)
		{
			case WizardPageID::Info:
			{
				// Show this only if this is the first time user leaving info page
				if (!IsInfoPageLeft())
				{
					// Reinstall confirmation
					if (!IsOptionEnabled(DialogOptions::Debug) && m_ExistingMod && m_ExistingMod->IsInstalled())
					{
						KxTaskDialog dialog(this, KxID_NONE, KTr("InstallWizard.Reinstall.Caption"), KTr("InstallWizard.Reinstall.Message"), KxBTN_YES|KxBTN_NO);
						dialog.SetMainIcon(KxICON_WARNING);
						return dialog.ShowModal() == KxID_YES;
					}
				}
				break;
			}
		};
		return true;
	}
}
