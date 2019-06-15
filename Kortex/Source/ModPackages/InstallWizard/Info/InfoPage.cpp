#include "stdafx.h"
#include "InfoPage.h"
#include "InfoDisplayModel.h"
#include "UI/KImageViewerDialog.h"
#include <Kortex/Application.hpp>
#include <Kortex/InstallWizard.hpp>
#include "Utility/Log.h"

namespace
{
	namespace OName
	{
		KortexDefOption(GeneralInfo);
		KortexDefOption(Documents);
	}
}

namespace Kortex::InstallWizard
{
	void InfoPage::OnSelectDocument(int index, bool useAdvancedEditor)
	{
		wxWindowUpdateLocker lock(m_DocumentsContainer);
		m_DocumentSimple->Show();
		m_DocumentAdvanced->Hide();
		m_DocumentAdvanced->LoadURL(wxWebViewDefaultURLStr);

		auto UnSplit = [this]()
		{
			m_DocumentsContainer->Unsplit(m_DocumentsList);
			m_DocumentsContainer->Unsplit(m_DocumentSimple);
			m_DocumentsContainer->Unsplit(m_DocumentAdvanced);
			m_DocumentSimple->Hide();
			m_DocumentAdvanced->Hide();
		};
		auto SwitchSimple = [this, &UnSplit]()
		{
			UnSplit();
			m_DocumentSimple->Show();
			m_DocumentSimple->Enable();
			m_DocumentsContainer->SplitVertically(m_DocumentsList, m_DocumentSimple, m_DocumentsContainer->GetSashPosition());
		};
		auto SwitchAdvanced = [this, &UnSplit]()
		{
			UnSplit();
			m_DocumentAdvanced->Show();
			m_DocumentAdvanced->Enable();
			m_DocumentsContainer->SplitVertically(m_DocumentsList, m_DocumentAdvanced, m_DocumentsContainer->GetSashPosition());
		};

		const KLabeledValue::Vector& documents = GetPackageConfig().GetInfo().GetDocuments();
		if (index != -1 && (size_t)index < documents.size())
		{
			try
			{
				const ModPackage& package = GetWizard().GetPackage();
				const KLabeledValue& entry = documents[index];

				if (useAdvancedEditor || KAux::IsFileExtensionMatches(entry.GetValue(), {"pdf", "xml", "htm", "html", "doc", "docx"}))
				{
					KxFileStream file(GetWizard().CreateTempFile(entry.GetValue().AfterLast('\\')), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Everything);
					const KArchive::Buffer& fileBuffer = package.GetDocumentBuffer(entry);
					file.Write(fileBuffer.data(), fileBuffer.size());

					m_DocumentAdvanced->LoadURL(file.GetFileName());
					SwitchAdvanced();
				}
				else
				{
					const KArchive::Buffer& buffer = package.GetDocumentBuffer(entry);
					wxString text = package.ReadString(buffer);
					if (text.IsEmpty() && !buffer.empty())
					{
						// Couldn't load the document, maybe unsupported encoding. Try WebView.
						OnSelectDocument(index, true);
					}
					else
					{
						m_DocumentSimple->SetTextValue(text);
						SwitchSimple();
					}
				}
			}
			catch (const std::out_of_range& e)
			{
				// Getting the document can throw
				Utility::Log::LogInfo("Can't load document at index %1: %2", index, e.what());
			}
		}
		else
		{
			SwitchSimple();
			m_DocumentSimple->SetTextValue(KAux::MakeHTMLWindowPlaceholder(KTr("InstallWizard.SelectDocumentHint"), m_DocumentSimple));
			m_DocumentSimple->Disable();
		}
	}
	void InfoPage::SetImageViewerNavigationInfo(KImageViewerEvent& event) const
	{
		event.SetHasPrevNext(m_CurrentImageIndex > 0, (size_t)(m_CurrentImageIndex + 1) < m_ImagesMap.size());
	}
	void InfoPage::OnNavigateImageViewer(KImageViewerEvent& event)
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

	wxWindow* InfoPage::CreateInfoTab()
	{
		m_InfoDisplayModel = new InfoPageNS::InfoDisplayModel(*this, 10);
		m_InfoDisplayModel->CreateView(m_TabsContainer);
		return m_InfoDisplayModel->GetView();
	}
	wxWindow* InfoPage::CreateDescriptionTab()
	{
		m_DescriptionView = new KxHTMLWindow(m_TabsContainer, KxID_NONE);
		m_DescriptionView->Bind(wxEVT_HTML_LINK_CLICKED, [this](wxHtmlLinkEvent& event)
		{
			KAux::AskOpenURL(event.GetLinkInfo().GetHref(), &GetWizard());
		});
		return m_DescriptionView;
	}
	wxWindow* InfoPage::CreateDocumentsTab()
	{
		m_DocumentsContainer = new KxSplitterWindow(m_TabsContainer, KxID_NONE);
		m_DocumentsContainer->SetName("DocumentsListSize");
		m_DocumentsContainer->SetMinimumPaneSize(200);
		m_DocumentsContainer->SetSashColor(IThemeManager::GetActive().GetColor(IThemeManager::ColorIndex::WindowBG));

		// List
		m_DocumentsList = new KxListBox(m_DocumentsContainer, KxID_NONE);
		m_DocumentsList->Bind(wxEVT_LISTBOX, [this](wxCommandEvent& event)
		{
			OnSelectDocument(event.GetInt());
		});

		m_DocumentSimple = new KxHTMLWindow(m_DocumentsContainer, KxID_NONE, wxEmptyString, KxThumbView::DefaultStyle|wxBORDER_THEME);
		m_DocumentAdvanced = wxWebView::New(m_DocumentsContainer, KxID_NONE);

		m_DocumentsContainer->SplitVertically(m_DocumentsList, m_DocumentSimple, m_DocumentsContainer->GetMinimumPaneSize());
		

		return m_DocumentsContainer;
	}
	wxWindow* InfoPage::CreateScreenshotsTab()
	{
		m_ScreenshotsView = new KxThumbView(m_TabsContainer, KxID_NONE, KxThumbView::DefaultStyle|wxBORDER_NONE);
		m_ScreenshotsView->Bind(KxEVT_THUMBVIEW_ACTIVATED, [this](wxCommandEvent& event)
		{
			m_CurrentImageIndex = event.GetInt();
			if (m_ImagesMap.count(m_CurrentImageIndex))
			{
				KImageViewerDialog dialog(&GetWizard());
				dialog.Bind(KEVT_IMAGEVIEWER_PREV_IMAGE, &InfoPage::OnNavigateImageViewer, this);
				dialog.Bind(KEVT_IMAGEVIEWER_NEXT_IMAGE, &InfoPage::OnNavigateImageViewer, this);

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
		IThemeManager::GetActive().ProcessWindow(m_ScreenshotsView);

		return m_ScreenshotsView;
	}

	void InfoPage::LoadInfoTab(const KPackageProject& package)
	{
		using InfoPageNS::InfoKind;
		const KPackageProjectInfo& info = package.GetInfo();

		auto AddString = [this](const wxString& name, const wxString& value, InfoKind type = InfoKind::None, bool isRequired = false, ResourceID image = {})
		{
			if (isRequired || !value.IsEmpty())
			{
				m_InfoDisplayModel->AddItem(KLabeledValue(value, name), image, type);
			}
		};
		auto AddSites = [this, &info, &AddString]()
		{
			info.GetModSourceStore().Visit([this](const ModSourceItem& item)
			{
				if (item.IsOK())
				{
					IModNetwork* modNetwork = nullptr;
					ResourceID icon = item.TryGetModNetwork(modNetwork) ? modNetwork->GetIcon() : IModNetwork::GetGenericIcon();
					m_InfoDisplayModel->AddItem(KLabeledValue(item.GetURL(), item.GetName()), icon, InfoKind::ModSource);
				}
				return true;
			});
		};
		auto AddUserData = [this, &info, &AddString]()
		{
			for (const KLabeledValue& item: info.GetCustomFields())
			{
				m_InfoDisplayModel->AddItem(item);
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

		m_InfoDisplayModel->ItemsChanged();
	}
	void InfoPage::LoadDescriptionTab(const KPackageProject& package)
	{
		m_DescriptionView->SetTextValue(package.GetInfo().GetDescription());
	}
	void InfoPage::LoadDocumentsTab(const KPackageProject& package)
	{
		for (const KLabeledValue& item: package.GetInfo().GetDocuments())
		{
			m_DocumentsList->AddItem(item.GetLabel());
		}
		{
			wxCommandEvent event(wxEVT_LISTBOX, m_DocumentsList->GetId());
			event.SetEventObject(m_DocumentsList);
			event.SetInt(-1);
			m_DocumentsList->HandleWindowEvent(event);
		}
	}
	void InfoPage::LoadScreenshotsTab(const KPackageProject& package)
	{
		for (const KPPIImageEntry& item: package.GetInterface().GetImages())
		{
			if (item.IsVisible() && item.HasBitmap())
			{
				size_t index = m_ScreenshotsView->AddThumb(item.GetBitmap());
				m_ImagesMap.insert_or_assign(index, &item);
			}
		}
	}

	void InfoPage::OnLoadUIOptions(const Application::ActiveInstanceOption& option)
	{
		option.QueryElement(OName::GeneralInfo).LoadDataViewLayout(m_InfoDisplayModel->GetView());
		option.QueryElement(OName::Documents).LoadSplitterLayout(m_DocumentsContainer);
	}
	void InfoPage::OnSaveUIOptions(Application::ActiveInstanceOption& option) const
	{
		option.QueryOrCreateElement(OName::GeneralInfo).SaveDataViewLayout(m_InfoDisplayModel->GetView());
		option.QueryOrCreateElement(OName::Documents).SaveSplitterLayout(m_DocumentsContainer);
	}
	void InfoPage::OnPackageLoaded()
	{
		const KPackageProject& package = GetPackageConfig();

		LoadInfoTab(package);
		LoadDescriptionTab(package);
		LoadDocumentsTab(package);
		LoadScreenshotsTab(package);
	}

	InfoPage::InfoPage(WizardDialog& wizard)
		:WizardPage(wizard)
	{
	}
	
	wxWindow* InfoPage::Create()
	{
		m_TabsContainer = new KxAuiNotebook(GetPageContainer(), KxID_NONE, KxAuiNotebook::DefaultStyle|wxBORDER_NONE);
		m_TabsContainer->AddPage(CreateInfoTab(), KTr("InstallWizard.Page.Info"), true);
		m_TabsContainer->AddPage(CreateDescriptionTab(), KTr("Generic.Description"));
		m_TabsContainer->AddPage(CreateDocumentsTab(), KTr("InstallWizard.Page.Documnets"));
		m_TabsContainer->AddPage(CreateScreenshotsTab(), KTr("InstallWizard.Page.Screenshots"));

		return m_TabsContainer;
	}
}
