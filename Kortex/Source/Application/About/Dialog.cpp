#include "stdafx.h"
#include "Dialog.h"
#include "DisplayModel.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"
#include "Utility/KBitmapSize.h"
#include <KxFramework/KxHTMLWindow.h>

namespace
{
	using namespace Kortex;
	using namespace Kortex::Application;

	enum TabID
	{
		Info,
		Components,
		License
	};

	wxString CreateInfoText(int yearBegin, int yearEnd, const About::INode& info)
	{
		const wxChar* formatString = wxS("$T(Generic.Version): $(AppVersion)\n"
										 "$T(Generic.Revision): $(AppRevision)\n"
										 "\n\n\n\n\n\n\n\n\n\n\n\n"

										 "This program is distributed in the hope that it will be useful, "
										 "but WITHOUT ANY WARRANTY; without even the implied warranty of "
										 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
										 "GNU General Public License for more details.\n\n"

										 "Copyright %1-%2 $(AppDeveloper).\n\n"
										 "$T(About.SourceCodeLocation) <a href=\"%3\">GitHub</a>."
		);
		return KxString::Format(KVarExp(formatString), yearBegin, yearEnd, info.GetURL());
	}
}

namespace Kortex::Application
{
	wxSize AboutDialog::GetLogoSize() const
	{
		return FromDIP(KBitmapSize().FromSystemIcon().GetSize() * 4);
	}
	wxString AboutDialog::GetCaption() const
	{
		return KxString::Format("%1 %2 %3", KTr("MainMenu.About"), m_AppInfo->GetName(), m_AppInfo->GetVersion());
	}

	wxWindow* AboutDialog::CreateTab_Info()
	{
		KxHTMLWindow* info = CreateHTMLWindow();
		info->SetValue(CreateInfoText(2018, 2019, *m_AppInfo));
		return info;
	}
	wxWindow* AboutDialog::CreateTab_Components()
	{
		m_DisplayModel = new About::DisplayModel(*this);
		m_DisplayModel->CreateView(m_TabView);
		m_DisplayModel->RefreshItems();

		return m_DisplayModel->GetView();
	}
	wxWindow* AboutDialog::CreateTab_License()
	{
		KxHTMLWindow* info = CreateHTMLWindow();
		info->SetValue(m_AppInfo->GetLicense());

		return info;
	}

	KxHTMLWindow* AboutDialog::CreateHTMLWindow()
	{
		KxHTMLWindow* window = new KxHTMLWindow(m_TabView, KxID_NONE, wxEmptyString, wxBORDER_NONE);
		window->Bind(wxEVT_HTML_LINK_CLICKED, &AboutDialog::OnLinkClicked, this);
		return window;
	}
	void AboutDialog::CreateTemporaryTab(wxWindow* window, const wxString& label, const wxBitmap& bitmap)
	{
		m_TabView->InsertPage(TabID::License, window, label, true, bitmap);
		m_TemporaryTab = window;
	}
	void AboutDialog::OnTabChanged(wxAuiNotebookEvent& event)
	{
		if (m_TemporaryTab)
		{
			int index = m_TabView->GetPageIndex(m_TemporaryTab);
			if (index != wxNOT_FOUND)
			{
				m_TabView->DeletePage(index);
				m_TemporaryTab = nullptr;
			}
		}
	}
	void AboutDialog::OnLinkClicked(wxHtmlLinkEvent& event)
	{
		KAux::AskOpenURL(event.GetLinkInfo().GetHref(), this);
	}

	AboutDialog::AboutDialog(wxWindow* parent)
		:m_AppInfo(std::make_unique<About::AppNode>())
	{
		if (KxStdDialog::Create(parent, KxID_NONE, GetCaption(), wxDefaultPosition, wxDefaultSize, KxBTN_OK))
		{
			SetDefaultBackgroundColor();
			GetContentWindow()->SetBackgroundColour(GetBackgroundColour());

			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide((wxOrientation)0);

			m_Logo = new KxImageView(m_ContentPanel, KxID_NONE, wxBORDER_NONE);
			m_Logo->SetBitmap(ImageProvider::GetBitmap("kortex-logo"));
			m_Logo->SetScaleMode(KxImageView_ScaleMode::KxIV_SCALE_ASPECT_FIT);

			m_TabView = new KxAuiNotebook(m_ContentPanel, KxID_NONE);
			m_TabView->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &AboutDialog::OnTabChanged, this);

			m_TabView->InsertPage(TabID::Info, CreateTab_Info(), KTr("Generic.Info"), true);
			m_TabView->InsertPage(TabID::Components, CreateTab_Components(), KTr("About.Components"));
			m_TabView->InsertPage(TabID::License, CreateTab_License(), KTr("Generic.License"));

			PostCreate(wxDefaultPosition);
			GetContentWindowSizer()->Prepend(m_Logo, 0, wxEXPAND)->SetMinSize(GetLogoSize());
			AdjustWindow(wxDefaultPosition, wxSize(780, 490));
		}
	}
	AboutDialog::~AboutDialog()
	{
	}
}
