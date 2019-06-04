#include "stdafx.h"
#include "Workspace.h"
#include "DisplayModel.h"
#include "ModPackages/IPackageManager.h"
#include "ModPackages/ModPackage.h"
#include "UI/KImageViewerDialog.h"
#include "UI/KMainWindow.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxSearchBox.h>
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxTextFile.h>

using namespace Kortex;
using namespace Kortex::Application;

namespace Kortex::Application::OName
{
	KortexDefOption(VSplitter);
	KortexDefOption(HSplitter);
}

namespace
{
	auto GetDisplayModelOption()
	{
		return GetAInstanceOptionOf<IPackageManager>(OName::Workspace, OName::DisplayModel);
	}
	auto GetVSplitterOption()
	{
		return GetAInstanceOptionOf<IPackageManager>(OName::Workspace, OName::VSplitter);
	}
	auto GetHSplitterOption()
	{
		return GetAInstanceOptionOf<IPackageManager>(OName::Workspace, OName::HSplitter);
	}
}

namespace Kortex::PackageManager
{
	Workspace::Workspace(KMainWindow* mainWindow)
		:KWorkspace(mainWindow)
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
		CreateItemInManagersMenu();
	}
	Workspace::~Workspace()
	{
		if (IsWorkspaceCreated())
		{
			GetDisplayModelOption().SaveDataViewLayout(m_DisplayModel->GetView());
			GetVSplitterOption().SaveSplitterLayout(m_Splitter);
			GetHSplitterOption().SaveSplitterLayout(m_InfoPane);
		}
	}
	bool Workspace::OnCreateWorkspace()
	{
		m_Splitter = new KxSplitterWindow(this, KxID_NONE);
		m_Splitter->SetName("PackageViewPaneSize");
		m_Splitter->SetMinimumPaneSize(250);
		m_MainSizer->Add(m_Splitter, 1, wxEXPAND);
		IThemeManager::GetActive().ProcessWindow(m_Splitter);

		CreateViewPane();
		CreateInfoPane();
		m_Splitter->SplitVertically(m_ViewPane, m_InfoPane, -m_Splitter->GetMinimumPaneSize());

		GetVSplitterOption().LoadSplitterLayout(m_Splitter);
		GetHSplitterOption().LoadSplitterLayout(m_InfoPane);
		GetDisplayModelOption().LoadDataViewLayout(m_DisplayModel->GetView());
		return true;
	}

	void Workspace::CreateViewPane()
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
			KxAuiToolBarItem* item = KMainWindow::CreateToolBarButton(m_ToolBar, wxEmptyString, ImageResourceID::Home);
			item->SetShortHelp(KTr(KxID_HOME));
			item->Bind(KxEVT_AUI_TOOLBAR_CLICK, [this](KxAuiToolBarEvent& event)
			{
				m_DisplayModel->NavigateHome();
			});
		}
		{
			KxAuiToolBarItem* item = KMainWindow::CreateToolBarButton(m_ToolBar, wxEmptyString, ImageResourceID::FolderArrow);
			item->SetShortHelp(KTr(KxID_UP));
			item->Bind(KxEVT_AUI_TOOLBAR_CLICK, [this](KxAuiToolBarEvent& event)
			{
				m_DisplayModel->NavigateUp();
			});
		}
		m_ToolBar->AddSeparator();
		{
			m_SearchBox = new KxSearchBox(m_ToolBar, KxID_NONE);
			m_SearchBox->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &Workspace::OnSerach, this);
			m_SearchBox->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &Workspace::OnSerach, this);

			m_ToolBar->AddControl(m_SearchBox)->SetProportion(1);
		}

		m_ToolBar->Realize();

		// View
		m_DisplayModel = new DisplayModel();
		m_DisplayModel->Create(m_ViewPane, sizer);
		m_DisplayModel->NavigateHome();
	}
	void Workspace::CreateInfoPane()
	{
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_InfoPane = new KxSplitterWindow(m_Splitter, KxID_NONE);
		m_InfoPane->SetName("ImageViewHeight");
		m_InfoPane->SetMinimumPaneSize(150);
		m_InfoPane->SetSizer(sizer);
		IThemeManager::GetActive().ProcessWindow(m_InfoPane);

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
		m_DisplayModel->SetImageView(m_InfoImage);

		// Description
		m_InfoDescription = new KxHTMLWindow(m_InfoPane, KxID_NONE, wxEmptyString, KxTextBox::DefaultStyle|wxBORDER_THEME);
		m_InfoDescription->SetEditable(false);
		m_InfoDescription->Enable(false);
		m_InfoDescription->Bind(wxEVT_HTML_LINK_CLICKED, [this](wxHtmlLinkEvent& event)
		{
			KAux::AskOpenURL(event.GetLinkInfo().GetHref(), GetMainWindow());
		});
		m_DisplayModel->SetDescriptionView(m_InfoDescription);

		m_InfoPane->SplitHorizontally(m_InfoImage, m_InfoDescription, m_InfoPane->GetMinimumPaneSize());
	}

	bool Workspace::OnOpenWorkspace()
	{
		return true;
	}
	bool Workspace::OnCloseWorkspace()
	{
		return true;
	}
	void Workspace::OnReloadWorkspace()
	{
		m_DisplayModel->Navigate(IPackageManager::GetInstance()->GetPackagesFolder());
	}

	void Workspace::OnSerach(wxCommandEvent& event)
	{
		wxString mask = event.GetString();
		if (event.GetEventType() == wxEVT_SEARCHCTRL_SEARCH_BTN && !mask.IsEmpty())
		{
			m_DisplayModel->Search(mask);
		}
		else
		{
			m_DisplayModel->NavigateHome();
		}
	}

	wxString Workspace::GetID() const
	{
		return "KPackageManagerWorkspace";
	}
	wxString Workspace::GetName() const
	{
		return KTr("PackageManager.PackagesListName");
	}
}
