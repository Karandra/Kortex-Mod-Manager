#include "stdafx.h"
#include "Dialog.h"
#include "DisplayModel.h"
#include <Kortex/Application.hpp>
#include "Utility/UI.h"
#include "Utility/BitmapSize.h"
#include <KxFramework/KxURI.h>
#include <KxFramework/KxHTMLWindow.h>

namespace
{
	using namespace Kortex;
	using namespace Kortex::Application;

	enum class TabID
	{
		Info,
		Components,
		License
	};
	enum class InfoText
	{
		Version,
		LicenseNotice
	};

	wxString CreateInfoText(const About::INode& info, InfoText infoVariant, int yearBegin = -1, int yearEnd = -1)
	{
		switch (infoVariant)
		{
			case InfoText::Version:
			{
				return ITranslator::ExpandVariables(wxS("$T(Generic.Version): $(AppVersion)\n$T(Generic.Revision): $(AppRevision)\n"));
			}
			case InfoText::LicenseNotice:
			{
				const wxChar* formatString = wxS("This program is distributed in the hope that it will be useful, "
												 "but WITHOUT ANY WARRANTY; without even the implied warranty of "
												 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
												 "GNU General Public License for more details.\n\n"

												 "<div align='right'>"
												 "Copyright %1-%2 $(AppDeveloper)\n"
												 "</div>"

												 "$T(About.SourceCodeLocation) <a href=\"%3\">GitHub</a>"
				);
				return KxString::Format(KVarExp(formatString), yearBegin, yearEnd, info.GetURI().BuildUnescapedURI());
			}
		};
		return {};
	}
}

namespace Kortex::Application
{
	wxSize AboutDialog::GetLogoSize() const
	{
		return FromDIP(Utility::BitmapSize().FromSystemIcon().GetSize() * 4);
	}
	wxString AboutDialog::GetCaption() const
	{
		return KxString::Format("%1 %2 %3", KTr("MainMenu.About"), m_AppInfo->GetName(), m_AppInfo->GetVersion());
	}

	wxWindow* AboutDialog::CreateTab_Info()
	{
		KxPanel* panel = new KxPanel(m_TabView, KxID_NONE);
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		panel->SetSizer(sizer);

		KxHTMLWindow* info = CreateHTMLWindow(panel);
		info->SetValue(CreateInfoText(*m_AppInfo, InfoText::Version));
		sizer->Add(info, 3, wxEXPAND);

		KxHTMLWindow* licenseNotice = CreateHTMLWindow(panel);
		licenseNotice->SetValue(CreateInfoText(*m_AppInfo, InfoText::LicenseNotice, 2018, 2019));
		sizer->Add(licenseNotice, 2, wxEXPAND);

		return panel;
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
		KxHTMLWindow* info = CreateHTMLWindow(m_TabView);
		info->SetValue(m_AppInfo->GetLicense());

		return info;
	}

	KxHTMLWindow* AboutDialog::CreateHTMLWindow(wxWindow* parent)
	{
		KxHTMLWindow* window = new KxHTMLWindow(parent ? parent : m_TabView, KxID_NONE, wxEmptyString, wxBORDER_NONE);
		window->Bind(wxEVT_HTML_LINK_CLICKED, &AboutDialog::OnLinkClicked, this);
		return window;
	}
	void AboutDialog::CreateTemporaryTab(wxWindow* window, const wxString& label, const wxBitmap& bitmap)
	{
		m_TabView->InsertPage((int)TabID::License, window, label, true, bitmap);
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
		Utility::UI::AskOpenURL(event.GetLinkInfo().GetHref(), this);
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

			m_TabView->InsertPage((int)TabID::Info, CreateTab_Info(), KTr("Generic.Info"), true);
			m_TabView->InsertPage((int)TabID::Components, CreateTab_Components(), KTr("About.Components"));
			m_TabView->InsertPage((int)TabID::License, CreateTab_License(), KTr("Generic.License"));

			PostCreate(wxDefaultPosition);
			GetContentWindowSizer()->Prepend(m_Logo, 0, wxEXPAND)->SetMinSize(GetLogoSize());
			AdjustWindow(wxDefaultPosition, FromDIP(wxSize(780, 490)));
		}
	}
	AboutDialog::~AboutDialog()
	{
	}
}
