#include "stdafx.h"
#include "InfoPage.h"
#include "InfoDisplayModel.h"
#include <Kortex/Application.hpp>
#include <Kortex/InstallWizard.hpp>
#include "UI/ImageViewerDialog.h"
#include "Utility/Common.h"
#include "Utility/Log.h"
#include "Utility/UI.h"
#include <KxFramework/KxTaskDialog.h>

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
	void InfoPage::OnTabOpened(wxAuiNotebookEvent& event)
	{
		if (!m_DescriptionLoaded)
		{
			LoadDescriptionTab(GetPackageConfig());
			m_DescriptionLoaded = true;
		}
	}
	void InfoPage::OnSelectDocument(int index, bool useAdvancedEditor)
	{
		wxWindowUpdateLocker lock(m_DocumentsContainer);
		m_DocumentSimple.GetWindow()->Show();
		m_DocumentAdvanced.GetWindow()->Hide();
		m_DocumentAdvanced.Unload();

		auto UnSplit = [this]()
		{
			m_DocumentsContainer->Unsplit(m_DocumentsList);
			m_DocumentsContainer->Unsplit(m_DocumentSimple);
			m_DocumentsContainer->Unsplit(m_DocumentAdvanced);
			m_DocumentSimple.GetWindow()->Hide();
			m_DocumentAdvanced.GetWindow()->Hide();
		};
		auto SwitchSimple = [this, &UnSplit]()
		{
			UnSplit();
			m_DocumentSimple.GetWindow()->Show();
			m_DocumentSimple.GetWindow()->Enable();
			m_DocumentsContainer->SplitVertically(m_DocumentsList, m_DocumentSimple, m_DocumentsContainer->GetSashPosition());
		};
		auto SwitchAdvanced = [this, &UnSplit]()
		{
			UnSplit();
			m_DocumentAdvanced.GetWindow()->Show();
			m_DocumentAdvanced.GetWindow()->Enable();
			m_DocumentsContainer->SplitVertically(m_DocumentsList, m_DocumentAdvanced, m_DocumentsContainer->GetSashPosition());
		};

		const Utility::LabeledValue::Vector& documents = GetPackageConfig().GetInfo().GetDocuments();
		if (index != -1 && (size_t)index < documents.size())
		{
			try
			{
				const ModPackage& package = GetWizard().GetPackage();
				const Utility::LabeledValue& entry = documents[index];

				if (useAdvancedEditor || Utility::FileExtensionMatches(entry.GetValue(), {"pdf", "xml", "htm", "html", "doc", "docx"}))
				{
					KxFileStream file(GetWizard().CreateTempFile(entry.GetValue().AfterLast('\\')), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Everything);
					const KArchive::Buffer& fileBuffer = package.GetDocumentBuffer(entry);
					file.Write(fileBuffer.data(), fileBuffer.size());

					m_DocumentAdvanced.LoadURL(file.GetFileName());
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
						m_DocumentSimple.LoadText(text);
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
			m_DocumentSimple.LoadHTML(Utility::UI::MakeHTMLWindowPlaceholder(KTr("InstallWizard.SelectDocumentHint"), m_DocumentSimple));
			m_DocumentSimple.GetWindow()->Disable();
		}
	}
	void InfoPage::SetImageViewerNavigationInfo(UI::ImageViewerEvent& event) const
	{
		event.SetHasPrevNext(m_CurrentImageIndex > 0, (size_t)(m_CurrentImageIndex + 1) < m_ImagesMap.size());
	}
	void InfoPage::OnNavigateImageViewer(UI::ImageViewerEvent& event)
	{
		int oldIndex = m_CurrentImageIndex;
		if (event.GetEventType() == UI::ImageViewerEvent::EvtNext)
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
			const PackageProject::ImageItem* entry = m_ImagesMap.at(m_CurrentImageIndex);
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
		m_DescriptionView.Create(m_TabsContainer, UI::WebView::Backend::Default, wxBORDER_NONE);
		return m_DescriptionView;
	}
	wxWindow* InfoPage::CreateDocumentsTab()
	{
		m_DocumentsContainer = new KxSplitterWindow(m_TabsContainer, KxID_NONE);
		m_DocumentsContainer->SetName("VSplitter");
		m_DocumentsContainer->SetMinimumPaneSize(200);
		m_DocumentsContainer->SetSashColor(IThemeManager::GetActive().GetColor(Theme::ColorIndex::Window, Theme::ColorFlags::Background));

		// List
		m_DocumentsList = new KxListBox(m_DocumentsContainer, KxID_NONE);
		m_DocumentsList->Bind(wxEVT_LISTBOX, [this](wxCommandEvent& event)
		{
			OnSelectDocument(event.GetInt());
		});

		m_DocumentSimple.Create(m_DocumentsContainer);
		m_DocumentAdvanced.Create(m_DocumentsContainer, UI::WebView::Backend::InternetExplorer);

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
				UI::ImageViewerDialog dialog(&GetWizard());
				dialog.Bind(UI::ImageViewerEvent::EvtPrevious, &InfoPage::OnNavigateImageViewer, this);
				dialog.Bind(UI::ImageViewerEvent::EvtNext, &InfoPage::OnNavigateImageViewer, this);

				const PackageProject::ImageItem* entry = m_ImagesMap.at(m_CurrentImageIndex);
				UI::ImageViewerEvent evt;
				evt.SetBitmap(entry->GetBitmap());
				evt.SetDescription(entry->GetDescription());
				SetImageViewerNavigationInfo(evt);
				dialog.Navigate(evt);

				dialog.ShowModal();
				m_CurrentImageIndex = -1;
			}
		});
		IThemeManager::GetActive().Apply(m_ScreenshotsView);

		return m_ScreenshotsView;
	}

	void InfoPage::LoadInfoTab(const ModPackageProject& package)
	{
		using InfoPageNS::InfoKind;
		const PackageProject::InfoSection& info = package.GetInfo();

		auto AddString = [this](const wxString& name, const wxString& value, InfoKind type = InfoKind::None, bool isRequired = false, ResourceID image = {})
		{
			if (isRequired || !value.IsEmpty())
			{
				m_InfoDisplayModel->AddItem(Utility::LabeledValue(value, name), image, type);
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
					m_InfoDisplayModel->AddItem(Utility::LabeledValue(item.GetURI().BuildUnescapedURI(), item.GetName()), icon, InfoKind::ModSource);
				}
				return true;
			});
		};
		auto AddUserData = [this, &info, &AddString]()
		{
			for (const Utility::LabeledValue& item: info.GetCustomFields())
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
	void InfoPage::LoadDescriptionTab(const ModPackageProject& package)
	{
		m_DescriptionView.LoadText(package.GetInfo().GetDescription());
	}
	void InfoPage::LoadDocumentsTab(const ModPackageProject& package)
	{
		for (const Utility::LabeledValue& item: package.GetInfo().GetDocuments())
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
	void InfoPage::LoadScreenshotsTab(const ModPackageProject& package)
	{
		for (const PackageProject::ImageItem& item: package.GetInterface().GetImages())
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
		const ModPackageProject& package = GetPackageConfig();

		LoadInfoTab(package);
		LoadDocumentsTab(package);
		LoadScreenshotsTab(package);
	}

	bool InfoPage::OnOpenPage()
	{
		return true;
	}
	bool InfoPage::OnClosePage()
	{
		bool allowSwitch = true;

		// Show this only if this is the first time user leaving info page
		if (!m_PageVisited)
		{
			// Reinstall confirmation
			WizardDialog& wizard = GetWizard();
			const IGameMod* existingMod = wizard.GetExistingMod();

			if (!wizard.IsOptionEnabled(DialogOptions::Debug) && existingMod && existingMod->IsInstalled())
			{
				KxTaskDialog dialog(&wizard, KxID_NONE, KTr("InstallWizard.Reinstall.Caption"), KTr("InstallWizard.Reinstall.Message"), KxBTN_YES|KxBTN_NO);
				dialog.SetMainIcon(KxICON_WARNING);
				allowSwitch = dialog.ShowModal() == KxID_YES;
			}
		}

		m_PageVisited = true;
		return allowSwitch;
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

		m_TabsContainer->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &InfoPage::OnTabOpened, this);

		return m_TabsContainer;
	}
}
