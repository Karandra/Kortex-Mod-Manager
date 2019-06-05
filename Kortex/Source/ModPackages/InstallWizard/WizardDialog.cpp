#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Events.hpp>
#include "Common.h"
#include "WizardDialog.h"
#include "InfoDisplayModel.h"
#include "RequirementsDisplayModel.h"
#include "ComponentsDisplayModel.h"
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

namespace
{
	enum class ComponentsTabIndex
	{
		Description = 0,
		Requirements = 1,
	};
	namespace OName
	{
		KortexDefOption(MainUI);
		KortexDefOption(InfoView);
		KortexDefOption(RequirementsView);
		KortexDefOption(ComponentRequirementsView);
		KortexDefOption(ComponentsView);
	}
}

namespace Kortex::InstallWizard
{
	wxDEFINE_EVENT(KEVT_IW_DONE, wxNotifyEvent);
}

namespace Kortex::InstallWizard
{
	namespace
	{
		auto GetUIOption(const wxString& option = {})
		{
			return Application::GetAInstanceOptionOf<IPackageManager>(wxS("InstallWizard"), option);
		};
	}

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
			m_BackwardButton->Bind(wxEVT_BUTTON, &WizardDialog::OnGoStepBackward, this);

			m_ForwardButton = AddButton(KxID_FORWARD, KTr("InstallWizard.ForwardButton") + " >").As<KxButton>();
			m_ForwardButton->Bind(wxEVT_BUTTON, &WizardDialog::OnGoForward, this);
			m_ForwardButton->Bind(wxEVT_BUTTON, &WizardDialog::OnGoStepForward, this);

			m_CancelButton = AddButton(KxID_CANCEL).As<KxButton>();
			m_CancelButton->Bind(wxEVT_BUTTON, &WizardDialog::OnCancelButton, this);
			Bind(wxEVT_CLOSE_WINDOW, &WizardDialog::OnClose, this);
			SetCloseIDs({});

			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide(wxBOTH);
			EnableMinimizeButton();
			EnableMaximizeButton();

			m_TabView = new wxSimplebook(m_ContentPanel, KxID_NONE, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
			m_TabView->AddPage(CreateUI_Info(), wxEmptyString);
			m_TabView->AddPage(CreateUI_Requirements(), wxEmptyString);
			m_TabView->AddPage(CreateUI_Components(), wxEmptyString);
			m_TabView->AddPage(CreateUI_Installing(), wxEmptyString);
			m_TabView->AddPage(CreateUI_Done(), wxEmptyString);

			PostCreate();
			return true;
		}
		return false;
	}
	void WizardDialog::LoadUIOptions()
	{
		GetUIOption().LoadWindowLayout(this);
		Center();

		GetUIOption(OName::MainUI).LoadSplitterLayout(m_Info_DocumentsSplitter);
		GetUIOption(OName::MainUI).LoadSplitterLayout(m_Components_SplitterV);
		GetUIOption(OName::MainUI).LoadSplitterLayout(m_Components_SplitterHRight);

		GetUIOption(OName::InfoView).LoadDataViewLayout(m_Info_PackageInfoList->GetView());
		GetUIOption(OName::RequirementsView).LoadDataViewLayout(m_Requirements_Main->GetView());
		GetUIOption(OName::ComponentRequirementsView).LoadDataViewLayout(m_Components_ItemList->GetView());
		GetUIOption(OName::ComponentsView).LoadDataViewLayout(m_Components_Requirements->GetView());
	}

	wxWindow* WizardDialog::CreateUI_Info()
	{
		m_Info_Tabs = new KxAuiNotebook(m_TabView, KxID_NONE, KxAuiNotebook::DefaultStyle|wxBORDER_NONE);

		/* Info */
		m_Info_PackageInfoList = new InfoDisplayModel(*this, GetConfig());
		m_Info_PackageInfoList->CreateView(m_Info_Tabs);
		m_Info_Tabs->AddPage(m_Info_PackageInfoList->GetView(), KTr("InstallWizard.Page.Info"), true);

		/* Description */
		m_Info_Description = new KxHTMLWindow(m_Info_Tabs, KxID_NONE);
		m_Info_Description->Bind(wxEVT_HTML_LINK_CLICKED, [this](wxHtmlLinkEvent& event)
		{
			KAux::AskOpenURL(event.GetLinkInfo().GetHref(), this);
		});
		m_Info_Tabs->AddPage(m_Info_Description, KTr("Generic.Description"));

		/* Documents */
		// Pane and sizer
		m_Info_DocumentsSplitter = new KxSplitterWindow(m_Info_Tabs, KxID_NONE);
		m_Info_DocumentsSplitter->SetName("DocumentsListSize");
		m_Info_DocumentsSplitter->SetMinimumPaneSize(200);
		m_Info_DocumentsSplitter->SetSashColor(IThemeManager::GetActive().GetColor(IThemeManager::ColorIndex::WindowBG));

		// List
		m_Info_DocumentsList = new KxListBox(m_Info_DocumentsSplitter, KxID_NONE);
		m_Info_DocumentsList->Bind(wxEVT_LISTBOX, [this](wxCommandEvent& event)
		{
			OnSelectDocument(event.GetInt());
		});

		m_Info_DocumentSimple = new KxHTMLWindow(m_Info_DocumentsSplitter, KxID_NONE, wxEmptyString, KxThumbView::DefaultStyle|wxBORDER_THEME);
		m_Info_DocumentAdvanced = wxWebView::New(m_Info_DocumentsSplitter, KxID_NONE);

		m_Info_DocumentsSplitter->SplitVertically(m_Info_DocumentsList, m_Info_DocumentSimple, m_Info_DocumentsSplitter->GetMinimumPaneSize());
		m_Info_Tabs->AddPage(m_Info_DocumentsSplitter, KTr("InstallWizard.Page.Documnets"));

		/* Screenshots */
		m_Info_Screenshots = new KxThumbView(m_Info_Tabs, KxID_NONE, KxThumbView::DefaultStyle|wxBORDER_NONE);
		m_Info_Screenshots->Bind(KxEVT_THUMBVIEW_ACTIVATED, [this](wxCommandEvent& event)
		{
			m_CurrentImageIndex = event.GetInt();
			if (m_ImagesMap.count(m_CurrentImageIndex))
			{
				KImageViewerDialog dialog(this);
				dialog.Bind(KEVT_IMAGEVIEWER_PREV_IMAGE, &WizardDialog::OnNavigateImageViewer, this);
				dialog.Bind(KEVT_IMAGEVIEWER_NEXT_IMAGE, &WizardDialog::OnNavigateImageViewer, this);

				const KPPIImageEntry* entry = m_ImagesMap.at(m_CurrentImageIndex);
				KImageViewerEvent evt;
				evt.SetBitmap(entry->GetBitmap());
				evt.SetDescription(entry->GetDescription());
				SetImageViewerNavigationInfo(evt);
				dialog.Navigate(evt);

				dialog.ShowModal();
				m_CurrentImageIndex = -1;
			}
		});
		IThemeManager::GetActive().ProcessWindow(m_Info_Screenshots);

		m_Info_Tabs->AddPage(m_Info_Screenshots, KTr("InstallWizard.Page.Screenshots"));

		// Return page
		return m_Info_Tabs;
	}
	wxWindow* WizardDialog::CreateUI_Requirements()
	{
		m_Requirements_Main = new RequirementsDisplayModel();
		m_Requirements_Main->Create(m_TabView);

		return m_Requirements_Main->GetView();
	}
	wxWindow* WizardDialog::CreateUI_Components()
	{
		/* Create splitters */
		m_Components_SplitterV = new KxSplitterWindow(m_TabView, KxID_NONE);
		m_Components_SplitterV->SetMinimumPaneSize(150);
		m_Components_SplitterV->SetName("ComponentsPaneSize");
		IThemeManager::GetActive().ProcessWindow(m_Components_SplitterV);

		m_Components_SplitterHRight = new KxSplitterWindow(m_Components_SplitterV, KxID_NONE);
		m_Components_SplitterHRight->SetName("ComponentImageHeight");
		m_Components_SplitterHRight->SetMinimumPaneSize(150);
		IThemeManager::GetActive().ProcessWindow(m_Components_SplitterHRight);

		/* Controls */
		// Item list
		m_Components_ItemList = new ComponentsDisplayModel();
		m_Components_ItemList->Create(m_Components_SplitterV);
		m_Components_ItemList->SetDataVector();
		m_Components_ItemList->GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &WizardDialog::OnSelectComponent, this);
		m_Components_ItemList->GetView()->Bind(KxEVT_DATAVIEW_ITEM_HOVERED, &WizardDialog::OnSelectComponent, this);

		// Image view
		m_Components_ImageView = new KxImageView(m_Components_SplitterHRight, KxID_NONE, wxBORDER_THEME);
		m_Components_ImageView->SetScaleMode(KxIV_SCALE_ASPECT_FIT);
		m_Components_ImageView->Bind(wxEVT_LEFT_DCLICK, [this](wxMouseEvent& event)
		{
			event.Skip();
			if (const KPPCEntry* pComponent = m_Components_ItemList->GetHotTrackedEntry())
			{
				const KPPIImageEntry* entry = GetConfig().GetInterface().FindEntryWithValue(pComponent->GetImage());
				if (entry)
				{
					KImageViewerEvent evt;
					evt.SetBitmap(entry->GetBitmap());
					evt.SetDescription(entry->GetDescription());

					KImageViewerDialog dialog(this);
					dialog.Navigate(evt);
					dialog.ShowModal();
				}
			}
		});

		// Tabs
		m_Components_Tabs = new KxAuiNotebook(m_Components_SplitterHRight, KxID_NONE);
		m_Components_Tabs->SetImageList(&ImageProvider::GetImageList());

		// Description
		m_Components_Description = new KxHTMLWindow(m_Components_Tabs, KxID_NONE, wxEmptyString, KxHTMLWindow::DefaultStyle|wxBORDER_NONE);
		m_Components_Description->Bind(wxEVT_HTML_LINK_CLICKED, [this](wxHtmlLinkEvent& event)
		{
			KAux::AskOpenURL(event.GetLinkInfo().GetHref(), this);
		});
		m_Components_Tabs->InsertPage((size_t)ComponentsTabIndex::Description, m_Components_Description, KTr("Generic.Description"), true);

		// Requirements
		m_Components_Requirements = new RequirementsDisplayModel();
		m_Components_Requirements->SetDataViewFlags(KxDataViewCtrl::DefaultStyle|(KxDataViewCtrlStyles)wxBORDER_NONE);
		m_Components_Requirements->Create(m_Components_Tabs);
		m_Components_Requirements->SetDataVector();
		m_Components_Tabs->InsertPage((size_t)ComponentsTabIndex::Requirements, m_Components_Requirements->GetView(), KTr("InstallWizard.Page.Requirements"));

		/* Split */
		m_Components_SplitterHRight->SplitHorizontally(m_Components_ImageView, m_Components_Tabs, -m_Components_SplitterHRight->GetMinimumPaneSize());
		m_Components_SplitterV->SplitVertically(m_Components_ItemList->GetView(), m_Components_SplitterHRight, m_Components_SplitterV->GetMinimumPaneSize());

		// Return main splitter
		return m_Components_SplitterV;
	}
	wxWindow* WizardDialog::CreateUI_Installing()
	{
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_Installing_Pane = new KxPanel(m_TabView, KxID_NONE);
		m_Installing_Pane->SetSizer(sizer);

		m_Installing_MinorProgress = new KxProgressBar(m_Installing_Pane, KxID_NONE, 100);
		m_Installing_MajorProgress = new KxProgressBar(m_Installing_Pane, KxID_NONE, 100);
		m_Installing_MinorStatus = KPackageCreatorPageBase::CreateNormalLabel(m_Installing_Pane, wxEmptyString, false);
		m_Installing_MajorStatus = KPackageCreatorPageBase::CreateNormalLabel(m_Installing_Pane, wxEmptyString, false);

		sizer->Add(m_Installing_MinorStatus, 0, wxEXPAND);
		sizer->Add(m_Installing_MinorProgress, 0, wxEXPAND);
		sizer->AddSpacer(15);
		sizer->Add(m_Installing_MajorStatus, 0, wxEXPAND);
		sizer->Add(m_Installing_MajorProgress, 0, wxEXPAND);

		Bind(KxEVT_ARCHIVE, &WizardDialog::OnMajorProgress, this);
		return m_Installing_Pane;
	}
	wxWindow* WizardDialog::CreateUI_Done()
	{
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_Done_Pane = new KxPanel(m_TabView, KxID_NONE);
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
			case PageIndex::Info:
			{
				SetLabel(KTr("InstallWizard.Page.Info"));
				break;
			}
			case PageIndex::Requirements:
			{
				SetLabel(KTr("InstallWizard.Page.Requirements"));
				break;
			}
			case PageIndex::Components:
			{
				SetLabel(KTr("InstallWizard.Page.Components"));
				break;
			}
			case PageIndex::Installation:
			{
				SetLabel(KTr("InstallWizard.Page.Installing"));
				break;
			}
			case PageIndex::Done:
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

			/* Info and misc */

			// Window caption
			SetCaption(KTrf("InstallWizard.WindowCaption", m_Package->GetName()) + ' ' + info.GetVersion());

			// Try to find existing mod for this package
			FindExistingMod();
			if (m_ExistingMod)
			{
				AcceptExistingMod(*m_ExistingMod);
			}

			// Info
			LoadInfoList();

			// Description
			m_Info_Description->SetTextValue(info.GetDescription());

			// Documents
			for (const KLabeledValue& entry: info.GetDocuments())
			{
				m_Info_DocumentsList->AddItem(entry.GetLabel());
			}

			wxCommandEvent event(wxEVT_LISTBOX, m_Info_DocumentsList->GetId());
			event.SetEventObject(m_Info_DocumentsList);
			event.SetInt(-1);
			m_Info_DocumentsList->HandleWindowEvent(event);

			// Screenshots
			for (const KPPIImageEntry& entry: interfaceConfig.GetImages())
			{
				if (entry.IsVisible() && entry.HasBitmap())
				{
					size_t index = m_Info_Screenshots->AddThumb(entry.GetBitmap());
					m_ImagesMap.insert_or_assign(index, &entry);
				}
			}

			// Header
			LoadHeaderImage();

			/* Requirements */
			StoreRequirementsFlags();
			LoadMainRequirements();

			/* Components */
			m_HasManualComponents = CheckIsManualComponentsAvailable();

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
		SwitchPage(PageIndex::Info);

		Show();
		Raise();

		Kortex::ModManager::ModEvent(Kortex::Events::ModInstalling, m_ModEntry).Send();

		return ret;
	}

	void WizardDialog::FindExistingMod()
	{
		m_ExistingMod = Kortex::IModManager::GetInstance()->FindModByID(GetConfig().GetModID());
	}
	void WizardDialog::AcceptExistingMod(const Kortex::IGameMod& mod)
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
	void WizardDialog::LoadInfoList()
	{
		const KPackageProject& project = m_Package->GetConfig();
		const KPackageProjectInfo& info = project.GetInfo();
		auto AddString = [this](const wxString& name, const wxString& value, InfoKind type = InfoKind::None, bool isRequired = false, ResourceID image = {})
		{
			if (isRequired || !value.IsEmpty())
			{
				m_Info_PackageInfoList->AddItem(KLabeledValue(value, name), image, type);
			}
		};
		auto AddSites = [this, &info, &AddString]()
		{
			info.GetModSourceStore().Visit([this](const ModSourceItem& item)
			{
				IModNetwork* modNetwork = nullptr;
				ResourceID icon = item.TryGetModNetwork(modNetwork) ? modNetwork->GetIcon() : IModNetwork::GetGenericIcon();
				m_Info_PackageInfoList->AddItem(KLabeledValue(item.GetURL(), item.GetName()), icon, InfoKind::ModSource);
				return true;
			});
		};
		auto AddUserData = [this, &info, &AddString]()
		{
			for (const KLabeledValue& entry: info.GetCustomFields())
			{
				m_Info_PackageInfoList->AddItem(entry);
			}
		};

		AddString(KTr("PackageCreator.PageInfo.BasicInfo.ID"), wxEmptyString, InfoKind::ID, true);
		AddString(KTr("PackageCreator.PageInfo.BasicInfo.Name"), wxEmptyString, InfoKind::Name, true);
		AddString(KTr("PackageCreator.PageInfo.BasicInfo.TranslatedName"), info.GetTranslatedName());
		AddString(KTr("PackageCreator.PageInfo.BasicInfo.Translatior"), info.GetTranslator());
		AddString(KTr("PackageCreator.PageInfo.BasicInfo.Version"), info.GetVersion(), InfoKind::None, true);
		AddString(KTr("PackageCreator.PageInfo.BasicInfo.Author"), info.GetAuthor(), InfoKind::None, true);
		AddSites();
		AddUserData();
		AddString(KTr("PackageCreator.PageInfo.BasicInfo.Tags"), wxEmptyString, InfoKind::Tags, true, ImageResourceID::Tags);

		m_Info_PackageInfoList->ItemsChanged();
	}
	void WizardDialog::LoadMainRequirements()
	{
		const KPackageProjectRequirements& tReqs = m_Package->GetConfig().GetRequirements();
		m_Requirements_Main->SetDataVector(&tReqs, tReqs.GetDefaultGroup());
	}

	void WizardDialog::OnNavigateImageViewer(KImageViewerEvent& event)
	{
		int oldIndex = m_CurrentImageIndex;
		if (event.GetEventType() == KEVT_IMAGEVIEWER_NEXT_IMAGE)
		{
			m_CurrentImageIndex++;
		}
		else
		{
			m_CurrentImageIndex--;
		}

		SetImageViewerNavigationInfo(event);
		if (m_CurrentImageIndex >= 0 && (size_t)m_CurrentImageIndex < m_ImagesMap.size())
		{
			const KPPIImageEntry* entry = m_ImagesMap.at(m_CurrentImageIndex);
			event.SetBitmap(entry->GetBitmap());
			event.SetDescription(entry->GetDescription());
		}
		else
		{
			m_CurrentImageIndex = oldIndex;
			event.Veto();
		}
	}
	void WizardDialog::SetImageViewerNavigationInfo(KImageViewerEvent& event)
	{
		event.SetHasPrevNext(m_CurrentImageIndex > 0, (size_t)(m_CurrentImageIndex + 1) < m_ImagesMap.size());
	}
	void WizardDialog::OnSelectDocument(int index, bool useAdvancedEditor)
	{
		wxWindowUpdateLocker lock(m_Info_DocumentsSplitter);
		m_Info_DocumentSimple->Show();
		m_Info_DocumentAdvanced->Hide();
		m_Info_DocumentAdvanced->LoadURL(wxWebViewDefaultURLStr);

		auto UnSplit = [this]()
		{
			m_Info_DocumentsSplitter->Unsplit(m_Info_DocumentsList);
			m_Info_DocumentsSplitter->Unsplit(m_Info_DocumentSimple);
			m_Info_DocumentsSplitter->Unsplit(m_Info_DocumentAdvanced);
			m_Info_DocumentSimple->Hide();
			m_Info_DocumentAdvanced->Hide();
		};
		auto SwitchSimple = [this, &UnSplit]()
		{
			UnSplit();
			m_Info_DocumentSimple->Show();
			m_Info_DocumentSimple->Enable();
			m_Info_DocumentsSplitter->SplitVertically(m_Info_DocumentsList, m_Info_DocumentSimple, m_Info_DocumentsSplitter->GetSashPosition());
		};
		auto SwitchAdvanced = [this, &UnSplit]()
		{
			UnSplit();
			m_Info_DocumentAdvanced->Show();
			m_Info_DocumentAdvanced->Enable();
			m_Info_DocumentsSplitter->SplitVertically(m_Info_DocumentsList, m_Info_DocumentAdvanced, m_Info_DocumentsSplitter->GetSashPosition());
		};

		const KLabeledValue::Vector& documents = m_Package->GetConfig().GetInfo().GetDocuments();
		if (index != -1 && (size_t)index < documents.size())
		{
			try
			{
				const KLabeledValue& entry = documents[index];
				if (useAdvancedEditor || KAux::IsFileExtensionMatches(entry.GetValue(), {"pdf", "xml", "htm", "html", "doc", "docx"}))
				{
					KxFileStream file(CreateTempFile(entry.GetValue().AfterLast('\\')), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Everything);
					const KArchive::Buffer& fileBuffer = m_Package->GetDocumentBuffer(entry);
					file.Write(fileBuffer.data(), fileBuffer.size());

					m_Info_DocumentAdvanced->LoadURL(file.GetFileName());
					SwitchAdvanced();
				}
				else
				{
					const KArchive::Buffer& buffer = m_Package->GetDocumentBuffer(entry);
					wxString text = m_Package->ReadString(buffer);
					if (text.IsEmpty() && !buffer.empty())
					{
						// Couldn't load the document, maybe unsupported encoding. Try WebView.
						OnSelectDocument(index, true);
					}
					else
					{
						m_Info_DocumentSimple->SetTextValue(text);
						SwitchSimple();
					}
				}
			}
			catch (const std::out_of_range& exc)
			{
				// Getting the document can throw
				Utility::Log::LogInfo("Can't load document at index %1: %2", index, exc.what());
			}
		}
		else
		{
			SwitchSimple();
			m_Info_DocumentSimple->SetTextValue(KAux::MakeHTMLWindowPlaceholder(KTr("InstallWizard.SelectDocumentHint"), m_Info_DocumentSimple));
			m_Info_DocumentSimple->Disable();
		}
	}

	bool WizardDialog::AskCancel(bool canCancel)
	{
		if (canCancel)
		{
			if (m_CurrentPage != PageIndex::Components && m_CurrentPage != PageIndex::Installation)
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
		if (!IsCompleted())
		{
			m_ShouldCancel = AskCancel(event.CanVeto());
			if (m_InstallThread)
			{
				m_InstallThread->Stop();
				event.Skip(false);
				event.Veto(true);
			}
			else
			{
				event.Veto(!m_ShouldCancel);
				event.Skip(m_ShouldCancel);
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
		PageIndex page = PageIndex::None;
		switch (m_CurrentPage)
		{
			case PageIndex::Requirements:
			{
				page = PageIndex::Info;
				break;
			}
			case PageIndex::Components:
			{
				page = HasMainRequirements() ? PageIndex::Requirements : PageIndex::Info;
				break;
			}
		};
		SwitchPage(page);
	}
	void WizardDialog::OnGoForward(wxCommandEvent& event)
	{
		if (IsCompleted())
		{
			Close(true);
		}
		else
		{
			PageIndex page = PageIndex::None;
			switch (m_CurrentPage)
			{
				case PageIndex::Info:
				{
					page = HasMainRequirements() ? PageIndex::Requirements : PageIndex::Components;
					if (page == PageIndex::Components && !HasManualComponents())
					{
						page = PageIndex::Installation;
					}
					break;
				}
				case PageIndex::Requirements:
				{
					page = HasManualComponents() ? PageIndex::Components : PageIndex::Installation;
					break;
				}
				case PageIndex::Components:
				{
					page = PageIndex::Installation;
					break;
				}
			};
			SwitchPage(page);
		}
	}

	void WizardDialog::OnGoStepBackward(wxCommandEvent& event)
	{
		if (m_InstallSteps.GetSize() > 1)
		{
			m_InstallSteps.PopItem();

			RestoreStepFlagsUpToThis(*m_InstallSteps.GetTopStep());
			LoadManualStep(*m_InstallSteps.GetTopStep());
		}
		else
		{
			// Now stack has only first step. Clear it.
			m_InstallSteps.Clear();
			event.Skip();
		}
	}
	void WizardDialog::OnGoStepForward(wxCommandEvent& event)
	{
		if (GetCurrentStep())
		{
			KPPCEntry::RefVector checkedEntries;
			if (m_Components_ItemList->OnLeaveStep(checkedEntries))
			{
				// Store flags from checked entries and save list of checked entries to current step.
				// This is needed for reverting checked status and correctly resetting flags when going step back.
				StoreStepFlags(checkedEntries);
				m_InstallSteps.GetTopItem()->GetChecked() = checkedEntries;

				KPPCStep* step = GetFirstStepSatisfiesConditions(GetCurrentStep());
				if (step)
				{
					m_InstallSteps.PushStep(step);
					LoadManualStep(*step);
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
	void WizardDialog::OnSelectComponent(KxDataViewEvent& event)
	{
		wxWindowUpdateLocker lock1(m_Components_Tabs);
		wxWindowUpdateLocker lock2(m_Components_ImageView);
		wxWindowUpdateLocker lock3(m_Components_Description);
		wxWindowUpdateLocker lock4(m_Components_ItemList->GetView());
		wxWindowUpdateLocker lock5(m_Components_Requirements->GetView());

		ComponentsDisplayModelNode* node = m_Components_ItemList->GetNode(event.GetItem());
		if (event.GetEventType() == KxEVT_DATAVIEW_ITEM_HOVERED && (!event.GetItem().IsOK() || !node || node->IsGroup()))
		{
			return;
		}
		ClearComponentsViewInfo();

		if (node)
		{
			if (const KPPCEntry* entry = node->GetEntry())
			{
				const wxString& description = entry->GetDescription();
				const bool isDescriptionEmpty = description.IsEmpty();
				if (!isDescriptionEmpty)
				{
					m_Components_Description->SetTextValue(description);
					m_Components_Description->Enable();
				}
				else
				{
					m_Components_Description->SetTextValue(KAux::MakeHTMLWindowPlaceholder(KTr("InstallWizard.NoDescriptionHint"), m_Components_Description));
				}
				m_Components_Tabs->SetPageImage((size_t)ComponentsTabIndex::Description, (int)(!isDescriptionEmpty ? ImageResourceID::InformationFrame : ImageResourceID::InformationFrameEmpty));

				bool bRequirementsEmpty = entry->GetRequirements().empty();
				if (!bRequirementsEmpty)
				{
					const bool isOK = GetConfig().GetRequirements().CalcOverallStatus(entry->GetRequirements());

					m_Components_Tabs->SetPageImage((size_t)ComponentsTabIndex::Requirements, (int)(isOK ? ImageResourceID::TickCircleFrame : ImageResourceID::CrossCircleFrame));
					m_Components_Requirements->SetDataVector(&GetConfig().GetRequirements(), entry->GetRequirements());
				}
				else
				{
					m_Components_Tabs->SetPageImage((size_t)ComponentsTabIndex::Requirements, (int)ImageResourceID::InformationFrameEmpty);
				}

				const KPPIImageEntry* pImageEntry = GetConfig().GetInterface().FindEntryWithValue(entry->GetImage());
				if (pImageEntry && pImageEntry->HasBitmap())
				{
					m_Components_ImageView->SetBitmap(pImageEntry->GetBitmap());
					m_Components_ImageView->Enable();
				}

				// Tab switch
				if (!isDescriptionEmpty)
				{
					m_Components_Tabs->ChangeSelection((size_t)ComponentsTabIndex::Description);
				}
				else
				{
					m_Components_Tabs->ChangeSelection((size_t)ComponentsTabIndex::Requirements);
				}
				m_Components_ItemList->GetView()->SetFocus();
			}
		}
		event.Skip();
	}

	void WizardDialog::StoreRequirementsFlags()
	{
		for (const auto& group: GetConfig().GetRequirements().GetGroups())
		{
			m_FlagsStorage.insert_or_assign(group->GetFlagName(), group->CalcGroupStatus() ? "true" : "false");
		}
	}
	void WizardDialog::StoreStepFlags(const KPPCEntry::RefVector& checkedEntries)
	{
		for (KPPCEntry* entry: checkedEntries)
		{
			for (const KPPCFlagEntry& flagEntry: entry->GetConditionalFlags().GetFlags())
			{
				m_FlagsStorage.insert_or_assign(flagEntry.GetName(), flagEntry.GetValue());
			}
		}
	}
	void WizardDialog::RestoreStepFlagsUpToThis(const KPPCStep& step)
	{
		m_FlagsStorage.clear();
		StoreRequirementsFlags();

		for (const StepStackItem& tStepItem: m_InstallSteps)
		{
			if (tStepItem.GetStep() != &step)
			{
				StoreStepFlags(tStepItem.GetChecked());
			}
			else
			{
				return;
			}
		}
	}

	bool WizardDialog::IsConditionSatisfied(const KPPCFlagEntry& flagEntry) const
	{
		auto it = m_FlagsStorage.find(flagEntry.GetName());
		if (it != m_FlagsStorage.end())
		{
			return it->second == flagEntry.GetValue();
		}
		return !flagEntry.HasValue();
	}
	bool WizardDialog::IsConditionsSatisfied(const KPPCConditionGroup& conditionGroup) const
	{
		if (conditionGroup.HasConditions())
		{
			KPackageProjectConditionChecker groupChecker;
			for (const KPPCCondition& condition: conditionGroup.GetConditions())
			{
				// Evaluate each condition
				KPackageProjectConditionChecker conditionChecker;
				for (const KPPCFlagEntry& flag: condition.GetFlags())
				{
					conditionChecker(IsConditionSatisfied(flag), condition.GetOperator());
					if (condition.GetOperator() == KPP_OPERATOR_AND && !conditionChecker.GetResult())
					{
						break;
					}
				}

				// Then combine it
				groupChecker(conditionChecker.GetResult(), conditionGroup.GetOperator());
				if (conditionGroup.GetOperator() == KPP_OPERATOR_AND && !groupChecker.GetResult())
				{
					break;
				}
			}
			return groupChecker.GetResult();
		}
		return true;
	}
	bool WizardDialog::IsStepSatisfiesConditions(const KPPCStep& step) const
	{
		return IsConditionsSatisfied(step.GetConditionGroup());
	}
	bool WizardDialog::CheckIsManualComponentsAvailable() const
	{
		const KPackageProjectComponents& components = GetConfig().GetComponents();
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
	KPPCStep* WizardDialog::GetFirstStepSatisfiesConditions() const
	{
		const KPPCStep::Vector& steps = GetConfig().GetComponents().GetSteps();
		for (size_t i = 0; i < steps.size(); i++)
		{
			KPPCStep* pCurrentStep = steps[i].get();
			if (IsStepSatisfiesConditions(*pCurrentStep))
			{
				return pCurrentStep;
			}
		}
		return nullptr;
	}
	KPPCStep* WizardDialog::GetFirstStepSatisfiesConditions(const KPPCStep* afterThis) const
	{
		const KPPCStep::Vector& steps = GetConfig().GetComponents().GetSteps();
		const auto itAfterThis = std::find_if(steps.begin(), steps.end(), [afterThis](const auto& step)
		{
			return step.get() == afterThis;
		});

		auto itNextAfterThis = itAfterThis + 1;
		if (itAfterThis != steps.end() && itNextAfterThis != steps.end())
		{
			for (auto it = itNextAfterThis; it != steps.end(); ++it)
			{
				KPPCStep& currentStep = **it;
				if (IsStepSatisfiesConditions(currentStep))
				{
					return &currentStep;
				}
			}
		}
		return nullptr;
	}

	void WizardDialog::ClearComponentsViewInfo()
	{
		m_Components_Tabs->SetPageImage((size_t)ComponentsTabIndex::Description, (int)ImageResourceID::InformationFrameEmpty);
		m_Components_Tabs->SetPageImage((size_t)ComponentsTabIndex::Requirements, (int)ImageResourceID::InformationFrameEmpty);

		m_Components_Description->SetTextValue(KAux::MakeHTMLWindowPlaceholder(KTr("InstallWizard.SelectComponentHint"), m_Components_Description));
		m_Components_Description->Disable();

		m_Components_ImageView->SetBitmap(wxNullBitmap);
		m_Components_ImageView->Disable();

		m_Components_Requirements->SetDataVector();
	}
	void WizardDialog::ResetComponents()
	{
		// Reset steps stack
		m_InstallSteps.Clear();

		// Reset flags
		m_FlagsStorage.clear();

		// Reset entries conditional type descriptors
		for (auto& step: GetConfig().GetComponents().GetSteps())
		{
			for (auto& group: step->GetGroups())
			{
				for (auto& entry: group->GetEntries())
				{
					entry->SetTDCurrentValue(KPPC_DESCRIPTOR_INVALID);
				}
			}
		}

		// Reset view
		ClearComponentsViewInfo();
		m_Components_ItemList->SetDataVector();
	}
	bool WizardDialog::BeginComponents()
	{
		ResetComponents();
		StoreRequirementsFlags();

		KPPCStep* step = GetFirstStepSatisfiesConditions();
		if (step)
		{
			m_InstallSteps.PushStep(step);
			LoadManualStep(*step);
			return true;
		}
		else
		{
			KxTaskDialog(this, KxID_NONE, KTr("InstallWizard.InvalidStepsConfig.Caption"), KTr("InstallWizard.InvalidStepsConfig.Message"), KxBTN_OK, KxICON_ERROR).ShowModal();
			return false;
		}
	}
	void WizardDialog::LoadManualStep(KPPCStep& step)
	{
		// Set step name if any
		if (!step.GetName().IsEmpty())
		{
			SetLabel(step.GetName());
		}
		else
		{
			SetLabelByCurrentPage();
		}

		// Check entries conditions and set its type descriptors current value
		for (auto& group: step.GetGroups())
		{
			for (auto& entry: group->GetEntries())
			{
				entry->SetTDCurrentValue(KPPC_DESCRIPTOR_INVALID);
				if (IsConditionsSatisfied(entry->GetTDConditionGroup()))
				{
					entry->SetTDCurrentValue(entry->GetTDConditionalValue());
				}
			}
		}

		m_Components_ItemList->SetDataVector(&GetConfig().GetComponents(), &step, GetCurrentStepItem()->GetChecked());
		m_Components_ItemList->GetView()->SetFocus();
	}
	void WizardDialog::CollectAllInstallableEntries()
	{
		auto AddFilesFromList = [this](const KxStringVector& list, bool pushBack = true)
		{
			for (const wxString& id: list)
			{
				KPPFFileEntry* entry = GetConfig().GetFileData().FindEntryWithID(id);
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
		if (!HasManualComponents() && !HasConditionalInstall())
		{
			for (const auto& entry: GetConfig().GetFileData().GetData())
			{
				m_InstallableFiles.push_back(entry.get());
			}
			SortInstallableFiles();
		}
		else
		{
			// Manual steps present, get files from checked entries
			if (HasManualComponents())
			{
				for (const StepStackItem& step: m_InstallSteps)
				{
					for (const KPPCEntry* entry: step.GetChecked())
					{
						AddFilesFromList(entry->GetFileData());
					}
				}
			}

			// Conditional install present. Run it and store files.
			if (HasConditionalInstall())
			{
				for (const auto& step: GetConfig().GetComponents().GetConditionalSteps())
				{
					if (IsConditionsSatisfied(step->GetConditionGroup()))
					{
						AddFilesFromList(step->GetEntries());
					}
				}
			}

			// Sort all files excluding required
			SortInstallableFiles();

			// Add required files to the beginning
			AddFilesFromList(GetConfig().GetComponents().GetRequiredFileData(), false);
		}

		// Remove duplicates
		UniqueFiles();
	}
	void WizardDialog::SortInstallableFiles()
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
	void WizardDialog::ShowInstallableFilesPreview()
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

		KxTaskDialog dialog(this, KxID_NONE, KTr("InstallWizard.CollectedFiles.Caption"), KxString::Join(files, "\r\n"), KxBTN_OK, KxICON_INFORMATION);
		dialog.ShowModal();
	}

	bool WizardDialog::OnBeginInstall()
	{
		CollectAllInstallableEntries();
		if (!m_InstallableFiles.empty())
		{
			m_InstallThread = new InstallationOperation(this);
			m_InstallThread->OnRun([this](KOperationWithProgressBase* self)
			{
				self->LinkHandler(&m_Package->GetArchive(), KxEVT_ARCHIVE);
				RunInstall();
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
			KxTaskDialog dialog(this, KxID_NONE, KTr("InstallWizard.NoFiles.Caption"), KTr("InstallWizard.NoFiles.Message"), KxBTN_OK, KxICON_ERROR);
			dialog.SetOptionEnabled(KxTD_HYPERLINKS_ENABLED);
			dialog.ShowModal();
			return false;
		}
	}
	bool WizardDialog::OnEndInstall()
	{
		if (m_ModEntry.Save())
		{
			// Save main or header image
			const KPackageProjectInterface& interfaceConfig = m_Package->GetConfig().GetInterface();
			const KPPIImageEntry* imageEntry = interfaceConfig.GetMainImageEntry();
			imageEntry = imageEntry ? imageEntry : interfaceConfig.GetHeaderImageEntry();
			if (imageEntry && imageEntry->HasBitmap())
			{
				const wxBitmap& bitmap = imageEntry->GetBitmap();
				bitmap.SaveFile(m_ModEntry.GetImageFile(), bitmap.HasAlpha() ? wxBITMAP_TYPE_PNG : wxBITMAP_TYPE_JPEG);
			}

			Kortex::IModManager::GetInstance()->NotifyModInstalled(m_ModEntry);
			if (ShouldCancel())
			{
				// We were canceled, but mod is partially installed.
				// Just exit immediately, user can uninstall this.
				Destroy();
			}
			else
			{
				SwitchPage(PageIndex::Done);
			}
			return true;
		}
		return false;
	}
	void WizardDialog::OnMinorProgress(KxFileOperationEvent& event)
	{
		if (m_ShouldCancel)
		{
			event.Veto();
			return;
		}

		m_Installing_MinorStatus->SetLabel(event.GetCurrent());
		if (!event.GetSource().IsEmpty())
		{
			m_Installing_MajorStatus->SetLabel(event.GetSource());
		}

		int64_t minorMin = event.GetMinorProcessed();
		int64_t minorMax = event.GetMinorTotal();
		if (event.IsMinorKnown())
		{
			m_Installing_MinorProgress->SetValue(minorMin, minorMax);
		}
		else if (minorMin != -2 && minorMax != -2)
		{
			m_Installing_MinorProgress->Pulse();
		}

		if (event.IsMajorKnown())
		{
			m_Installing_MajorProgress->SetValue(event.GetMajorProcessed(), event.GetMajorTotal());
		}
		else
		{
			m_Installing_MajorProgress->Pulse();
		}
	}
	void WizardDialog::OnMajorProgress(KxFileOperationEvent& event)
	{
		if (m_ShouldCancel)
		{
			event.Veto();
			return;
		}

		int64_t current = event.GetMajorProcessed();
		int64_t max = event.GetMajorTotal();
		m_Installing_MajorStatus->SetLabel(KTrf("InstallWizard.InstalledXOfY", current, max) + wxS(". ") + event.GetSource());
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
		for (const KPPFFileEntry* fileEntry: m_InstallableFiles)
		{
			if (m_InstallThread->CanContinue())
			{
				NotifyMajor(processed, m_InstallableFiles.size(), fileEntry->GetSource());

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
				NotifyMajor(processed, m_InstallableFiles.size(), fileEntry->GetSource());
			}
			else
			{
				return;
			}
		}
	}

	WizardDialog::WizardDialog()
		//:m_Option_Window(this, "Window"),
		//m_Option_MainUI(this, "MainUI"),
		//m_Option_InfoView(this, "InfoView"),
		//m_Option_RequirementsView(this, "RequirementsView"),
		//m_Option_ComponentsView(this, "ComponentsView"),
		//m_Option_ComponentRequirementsView(this, "ComponentRequirementsView")
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
			GetUIOption().SaveWindowLayout(this);

			GetUIOption(OName::MainUI).SaveSplitterLayout(m_Info_DocumentsSplitter);
			GetUIOption(OName::MainUI).SaveSplitterLayout(m_Components_SplitterV);
			GetUIOption(OName::MainUI).SaveSplitterLayout(m_Components_SplitterHRight);

			GetUIOption(OName::InfoView).SaveDataViewLayout(m_Info_PackageInfoList->GetView());
			GetUIOption(OName::RequirementsView).SaveDataViewLayout(m_Requirements_Main->GetView());
			GetUIOption(OName::ComponentRequirementsView).SaveDataViewLayout(m_Components_ItemList->GetView());
			GetUIOption(OName::ComponentsView).SaveDataViewLayout(m_Components_Requirements->GetView());
		}
	}

	void WizardDialog::SwitchPage(PageIndex page)
	{
		if (page != m_CurrentPage && page != PageIndex::None && page < (PageIndex)m_TabView->GetPageCount())
		{
			if (!OnLeavingPage(m_CurrentPage))
			{
				return;
			}

			switch (page)
			{
				case PageIndex::Info:
				{
					m_BackwardButton->Disable();
					m_ForwardButton->Enable();

					m_CurrentPage = page;
					SetLabelByCurrentPage();
					m_TabView->ChangeSelection((size_t)page);
					break;
				}
				case PageIndex::Requirements:
				{
					m_BackwardButton->Enable();
					m_ForwardButton->Enable();

					m_CurrentPage = page;
					SetLabelByCurrentPage();
					m_TabView->ChangeSelection((size_t)page);
					break;
				}
				case PageIndex::Components:
				{
					m_BackwardButton->Enable();
					m_ForwardButton->Enable();

					m_CurrentPage = page;
					SetLabelByCurrentPage();

					if (!BeginComponents())
					{
						Close(true);
						break;
					}

					m_TabView->ChangeSelection((size_t)page);
					break;
				}
				case PageIndex::Installation:
				{
					if (IsOptionEnabled(DialogOptions::Debug))
					{
						CollectAllInstallableEntries();
						ShowInstallableFilesPreview();

						// Switch back to info page to reset components state.
						SwitchPage(PageIndex::Info);
						break;
					}

					if (!OnBeginInstall())
					{
						Close(true);
						break;
					}

					m_BackwardButton->Disable();
					m_ForwardButton->Disable();

					m_CurrentPage = page;
					SetLabelByCurrentPage();
					m_TabView->ChangeSelection((size_t)page);
					break;
				}
				case PageIndex::Done:
				{
					m_IsComplete = true;

					m_CancelButton->Disable();
					m_BackwardButton->Disable();
					m_ForwardButton->Enable();

					m_ForwardButton->SetLabel(KTr(KxID_CLOSE));
					m_Done_Label->SetLabel(KTrf("InstallWizard.InstallationComplete", m_Package->GetName()));

					m_CurrentPage = page;
					SetLabelByCurrentPage();
					m_TabView->ChangeSelection((size_t)page);
					break;
				}
			};

			if (page != PageIndex::Info)
			{
				m_InfoPageLeft = true;
			}
		}
	}
	bool WizardDialog::OnLeavingPage(PageIndex page)
	{
		switch (page)
		{
			case PageIndex::Info:
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
	bool WizardDialog::HasMainRequirements() const
	{
		return !GetConfig().GetRequirements().IsDefaultGroupEmpty();
	}
	bool WizardDialog::HasManualComponents() const
	{
		return m_HasManualComponents;
	}
	bool WizardDialog::HasConditionalInstall() const
	{
		return !GetConfig().GetComponents().GetConditionalSteps().empty();
	}
}
