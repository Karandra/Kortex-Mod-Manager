#include "stdafx.h"
#include "KPackageManagerWorkspace.h"
#include "KPackageManagerListModel.h"
#include "KPackageManager.h"
#include "KModPackage.h"
#include "UI/KImageViewerDialog.h"
#include "UI/KMainWindow.h"
#include "KThemeManager.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxSearchBox.h>
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxTextFile.h>

KxSingletonPtr_Define(KPackageManagerWorkspace);

KPackageManagerWorkspace::KPackageManagerWorkspace(KMainWindow* mainWindow)
	:KWorkspace(mainWindow), m_PackageListViewOptions(this, "PackageListView"), m_MainOptions(this, "MainUI")
{
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
	CreateItemInManagersMenu();
}
KPackageManagerWorkspace::~KPackageManagerWorkspace()
{
	if (IsWorkspaceCreated())
	{
		KProgramOptionSerializer::SaveDataViewLayout(m_MainView->GetView(), m_PackageListViewOptions);
		KProgramOptionSerializer::SaveSplitterLayout(m_Splitter, m_MainOptions);
		KProgramOptionSerializer::SaveSplitterLayout(m_InfoPane, m_MainOptions);
	}
}
bool KPackageManagerWorkspace::OnCreateWorkspace()
{
	m_Splitter = new KxSplitterWindow(this, KxID_NONE);
	m_Splitter->SetName("PackageViewPaneSize");
	m_Splitter->SetMinimumPaneSize(250);
	m_MainSizer->Add(m_Splitter, 1, wxEXPAND);
	KThemeManager::Get().ProcessWindow(m_Splitter);

	CreateViewPane();
	CreateInfoPane();
	m_Splitter->SplitVertically(m_ViewPane, m_InfoPane, -m_Splitter->GetMinimumPaneSize());

	KProgramOptionSerializer::LoadSplitterLayout(m_Splitter, m_MainOptions);
	KProgramOptionSerializer::LoadSplitterLayout(m_InfoPane, m_MainOptions);
	KProgramOptionSerializer::LoadDataViewLayout(m_MainView->GetView(), m_PackageListViewOptions);
	return true;
}

void KPackageManagerWorkspace::CreateViewPane()
{
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	m_ViewPane = new KxPanel(m_Splitter, KxID_NONE);
	m_ViewPane->SetSizer(sizer);

	// ToolBar
	m_ToolBar = new KxAuiToolBar(m_ViewPane, KxID_NONE, wxAUI_TB_HORZ_TEXT|wxAUI_TB_PLAIN_BACKGROUND);
	m_ToolBar->SetBackgroundColour(GetBackgroundColour());
	m_ToolBar->SetMargins(0, 0, 0, 0);
	sizer->Add(m_ToolBar, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

	{
		KxAuiToolBarItem* item = KMainWindow::CreateToolBarButton(m_ToolBar, wxEmptyString, KIMG_HOME);
		item->SetShortHelp(T(KxID_HOME));
		item->Bind(KxEVT_AUI_TOOLBAR_CLICK, [this](KxAuiToolBarEvent& event)
		{
			m_MainView->NavigateHome();
		});
	}
	{
		KxAuiToolBarItem* item = KMainWindow::CreateToolBarButton(m_ToolBar, wxEmptyString, KIMG_FOLDER_ARROW);
		item->SetShortHelp(T(KxID_UP));
		item->Bind(KxEVT_AUI_TOOLBAR_CLICK, [this](KxAuiToolBarEvent& event)
		{
			m_MainView->NavigateUp();
		});
	}
	m_ToolBar->AddSeparator();
	{
		m_SearchBox = new KxSearchBox(m_ToolBar, KxID_NONE);
		m_SearchBox->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &KPackageManagerWorkspace::OnSerach, this);
		m_SearchBox->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &KPackageManagerWorkspace::OnSerach, this);

		m_ToolBar->AddControl(m_SearchBox)->SetProportion(1);
	}

	m_ToolBar->Realize();

	// View
	m_MainView = new KPackageManagerListModel();
	m_MainView->Create(m_ViewPane, sizer);
	m_MainView->NavigateHome();
}
void KPackageManagerWorkspace::CreateInfoPane()
{
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	m_InfoPane = new KxSplitterWindow(m_Splitter, KxID_NONE);
	m_InfoPane->SetName("ImageViewHeight");
	m_InfoPane->SetMinimumPaneSize(150);
	m_InfoPane->SetSizer(sizer);
	KThemeManager::Get().ProcessWindow(m_InfoPane);

	// Image
	m_InfoImage = new KxImageView(m_InfoPane, KxID_NONE, wxBORDER_THEME);
	m_InfoImage->SetScaleMode(KxIV_SCALE_ASPECT_FIT);
	m_InfoImage->Bind(wxEVT_LEFT_DCLICK, [this](wxMouseEvent& event)
	{
		event.Skip();
		if (m_InfoImage->GetBitmap().IsOk())
		{
			KImageViewerDialog dialog(GetMainWindow());

			KImageViewerEvent evt;
			evt.SetBitmap(m_InfoImage->GetBitmap());
			dialog.Navigate(evt);

			dialog.ShowModal();
		}
	});
	m_MainView->SetImageView(m_InfoImage);

	// Description
	m_InfoDescription = new KxHTMLWindow(m_InfoPane, KxID_NONE, wxEmptyString, KxTextBox::DefaultStyle|wxBORDER_THEME);
	m_InfoDescription->SetEditable(false);
	m_InfoDescription->Enable(false);
	m_InfoDescription->Bind(wxEVT_HTML_LINK_CLICKED, [this](wxHtmlLinkEvent& event)
	{
		KAux::AskOpenURL(event.GetLinkInfo().GetHref(), GetMainWindow());
	});
	m_MainView->SetDescriptionView(m_InfoDescription);

	m_InfoPane->SplitHorizontally(m_InfoImage, m_InfoDescription, m_InfoPane->GetMinimumPaneSize());
}

bool KPackageManagerWorkspace::OnOpenWorkspace()
{
	return true;
}
bool KPackageManagerWorkspace::OnCloseWorkspace()
{
	return true;
}
void KPackageManagerWorkspace::OnReloadWorkspace()
{
	m_MainView->Navigate(KPackageManager::Get().GetPackagesFolder());
}

void KPackageManagerWorkspace::OnSerach(wxCommandEvent& event)
{
	wxString mask = event.GetString();
	if (event.GetEventType() == wxEVT_SEARCHCTRL_SEARCH_BTN && !mask.IsEmpty())
	{
		m_MainView->Search(mask);
	}
	else
	{
		m_MainView->NavigateHome();
	}
}

wxString KPackageManagerWorkspace::GetID() const
{
	return "KPackageManagerWorkspace";
}
wxString KPackageManagerWorkspace::GetName() const
{
	return T("ToolBar.PackageManager");
}
