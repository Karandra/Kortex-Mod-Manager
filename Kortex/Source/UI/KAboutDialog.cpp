#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/PluginManager.hpp>
#include "KAboutDialog.h"
#include "KMainWindow.h"
#include "KWorkspace.h"
#include "Archive/KArchive.h"
#include "KAux.h"
#include "KBitmapSize.h"
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxCrypto.h>
#include <KxFramework/KxWebSockets.h>
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxJSON.h>
#include "VFS/KVFSService.h"

namespace
{
	wxString GetAppLicense()
	{
		return Kortex::IApplication::GetInstance()->GetDataFolder();
	}
	wxString CreateInfoText(int yearBegin, int yearEnd, const wxString& sourceLink)
	{
		const wxChar* formatString = wxS("$T(Generic.Version): $(AppVersion)\n"
										 "$T(Generic.Revision): $(AppRevision)\n\n\n\n\n\n\n\n\n\n\n"

										 "This program is distributed in the hope that it will be useful, "
										 "but WITHOUT ANY WARRANTY; without even the implied warranty of "
										 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
										 "GNU General Public License for more details.\n\n"

										 "Copyright %1-%2 $(AppDeveloper).\n\n"
										 "$T(About.Info.SourceCodeLocation) <a href=\"%3\">GitHub</a>."
		);
		return KxString::Format(KVarExp(formatString), yearBegin, yearEnd, sourceLink);
	}
}

using namespace Kortex;

//////////////////////////////////////////////////////////////////////////
wxString KAboutInfo::GetLocation() const
{
	switch (m_Type)
	{
		case Type::App:
		{
			return Kortex::IApplication::GetInstance()->GetDataFolder() + wxS("\\About");
		}
		case Type::Software:
		{
			return Kortex::IApplication::GetInstance()->GetDataFolder() + wxS("\\About\\Software");
		}
		case Type::Resource:
		{
			return Kortex::IApplication::GetInstance()->GetDataFolder() + wxS("\\About\\Resource");
		}
	};
	return wxEmptyString;
}
wxString KAboutInfo::GetLicense() const
{
	if (m_License.IsEmpty())
	{
		m_License = KxTextFile::ReadToString(GetLocation() + wxS("\\License.txt"));

		// Convert any '<text>' which is not a link to '&lt;text&gt;'
		{
			wxRegEx regex("<(?!http:)(.*?)>", wxRE_ADVANCED|wxRE_ICASE);
			regex.ReplaceAll(&m_License, wxS("\\&lt;\\1\\&gt;"));
		}

		// Create clickable links
		{
			wxRegEx regex("<http:(.*?)>", wxRE_ADVANCED|wxRE_ICASE);
			regex.ReplaceAll(&m_License, wxS("<a href=\"http:\\1\">http:\\1</a>"));
		}
	}
	return m_License;
}

//////////////////////////////////////////////////////////////////////////
KxHTMLWindow* KAboutDialog::CreateHTMLWindow(wxWindow* parent)
{
	KxHTMLWindow* window = new KxHTMLWindow(parent, KxID_NONE);
	wxFont font = GetFont();
	window->SetStandardFonts(font.GetPointSize(), font.GetFaceName(), font.GetFaceName());
	window->SetBorders(KLC_HORIZONTAL_SPACING);
	window->Bind(wxEVT_HTML_LINK_CLICKED, &KAboutDialog::OnLinkClick, this);
	return window;
}
wxSize KAboutDialog::GetLogoSize() const
{
	return FromDIP(KBitmapSize().FromSystemIcon().GetSize() * 4);
}

wxWindow* KAboutDialog::CreateTab_Info()
{
	KxHTMLWindow* info = CreateHTMLWindow(m_View);
	info->SetTextValue(CreateInfoText(2018, 2019, "https://github.com/KerberX/Kortex-Mod-Manager"));
	return info;

	wxString libraries;
	auto AddLibrary = [&libraries](const char* name, const char* url, const char* extraName = nullptr, const char* extraURL = nullptr)
	{
		libraries << KxString::Format("<a href=\"%1\">%2</a>", url, name);
		if (extraName && extraURL)
		{
			libraries << KxString::Format(" (<a href=\"%1\">%2</a>)", extraURL, extraName);
		}
		libraries << "<br>";
	};
	AddLibrary("KxVirtualFileSystem", "https://github.com/KerberX/KxVirtualFileSystem");
	AddLibrary("Dokany", "https://github.com/dokan-dev/dokany");
	AddLibrary("wxWidgets", "https://www.wxwidgets.org");
	AddLibrary("SimpleINI", "https://github.com/brofield/simpleini");
	AddLibrary("TinyXML2", "https://github.com/leethomason/tinyxml2");
	AddLibrary("LOOT API", "https://github.com/loot/loot-api");
	AddLibrary("OpenSSL", "https://www.openssl.org", "Precompiled OpenSSL", "https://www.npcglib.org/~stathis/blog/precompiled-openssl");
	AddLibrary("7-Zip", "https://www.7-zip.org");
	AddLibrary("WebSocket++", "https://github.com/zaphoyd/websocketpp");
	AddLibrary("cURL", "https://curl.haxx.se");
	AddLibrary(KxJSON::GetLibraryName(), "https://github.com/nlohmann/json");

	wxString iconPacks;
	auto AddIconPack = [&iconPacks](const char* packName, const char* authorName, const char* url)
	{
		iconPacks << KTrf("About.Info.IconPack", packName, KxString::Format("<a href=\"%1\">%2</a>", url, authorName));
		iconPacks << "<br>";
	};
	AddIconPack("Fugue Icons", "Yusuke Kamiyamane", "http://p.yusukekamiyamane.com");
	AddIconPack("Tango 7-zip", "Laoism", "https://laoism.deviantart.com/art/Tango-7-zip-2-1-113983312");

	info->SetPage(KxString::Format("<div align=\"justify\">%1:<br>%2<br>%3</div>",
									KTr("About.Info.Libraries"), libraries,
									iconPacks));
	return info;
}
wxWindow* KAboutDialog::CreateTab_Modules()
{
	KxTreeList* list = new KxTreeList(m_View, KxID_NONE, KxTreeList::DefaultStyle|wxTL_NO_HEADER);
	list->GetDataView()->ToggleWindowStyle(wxBORDER_NONE);
	list->GetDataView()->SetAllowColumnsAutoSize(false);
	list->SetImageList(&Kortex::IApplication::GetInstance()->GetImageList());
	list->AddColumn(wxEmptyString, 300);
	list->AddColumn(wxEmptyString, 75);

	Kortex::IModule::ForEachModule([list](Kortex::IModule& module)
	{
		const Kortex::IModuleInfo& info = module.GetModuleInfo();
		KxTreeListItem item = list->GetRoot().Add(KxStringVector {info.GetName(), info.GetVersion()});
		item.SetImage(info.GetImageID());
	});

	// 7-Zip
	{
		KxTreeListItem item = list->GetRoot().Add(KxStringVector{"7-Zip", KArchive::GetLibraryVersion()});
		item.SetImage(KIMG_7ZIP);
	}

	// OpenSSL
	{
		KxTreeListItem item = list->GetRoot().Add(KxStringVector{"OpenSSL", KxCrypto::GetOpenSSLVersion()});
		item.SetImage(KIMG_LOCK_SSL);
	}

	// KxVFS
	{
		KxTreeListItem item = list->GetRoot().Add(KxStringVector{"KxVirtualFileSystem", KVFSService::GetLibraryVersion()});
		item.SetImage(KIMG_FOLDERS);
	}

	// LOOT API
	if (Kortex::PluginManager::LootAPI* lootAPI = Kortex::PluginManager::LootAPI::GetInstance())
	{
		KxTreeListItem item = list->GetRoot().Add(KxStringVector{lootAPI->GetLibraryName(), lootAPI->GetLibraryVersion()});
		item.SetImage(KIMG_LOOT);
	}

	// WebSocket++
	{
		KxTreeListItem item = list->GetRoot().Add(KxStringVector{KxWebSocketClient::GetLibraryName(), KxWebSocketClient::GetLibraryName()});
		item.SetImage(KIMG_WEBSOCKET);
	}

	// cURL
	{
		KxTreeListItem item = list->GetRoot().Add(KxStringVector{"cURL", KxCURL::GetVersion()});
		item.SetImage(KIMG_CURL);
	}
	
	// JSON
	{
		KxTreeListItem item = list->GetRoot().Add(KxStringVector{KxJSON::GetLibraryName(), KxJSON::GetVersion()});
		item.SetImage(KIMG_JSON);
	}
	return list;
}
wxWindow* KAboutDialog::CreateTab_License()
{
	KxHTMLWindow* info = CreateHTMLWindow(m_View);
	info->SetTextValue(m_AppInfo.GetLicense());

	return info;
}

void KAboutDialog::OnLinkClick(wxHtmlLinkEvent& event)
{
	KAux::AskOpenURL(event.GetLinkInfo().GetHref(), this);
}

KAboutDialog::KAboutDialog(wxWindow* parent)
	:m_AppInfo(KAboutInfo::Type::App)
{
	if (KxStdDialog::Create(parent, KxID_NONE, KVarExp("$T(MainMenu.About) $(AppName)"), wxDefaultPosition, wxDefaultSize, KxBTN_OK))
	{
		SetDefaultBackgroundColor();
		GetContentWindow()->SetBackgroundColour(GetBackgroundColour());

		SetMainIcon(KxICON_NONE);
		SetWindowResizeSide((wxOrientation)0);

		m_Logo = new KxImageView(m_ContentPanel, KxID_NONE, wxBORDER_NONE);
		m_Logo->SetBitmap(KGetBitmap("application-logo"));
		m_Logo->SetScaleMode(KxImageView_ScaleMode::KxIV_SCALE_ASPECT_FIT);

		m_View = new KxAuiNotebook(m_ContentPanel, KxID_NONE);
		IThemeManager::GetActive().ProcessWindow(m_View);

		m_View->AddPage(CreateTab_Info(), KTr("About.Info.Caption"), true);
		m_View->AddPage(CreateTab_Modules(), KTr("About.Modules.Caption"));
		m_View->AddPage(CreateTab_License(), KTr("About.License.Caption"));

		PostCreate(wxDefaultPosition);
		GetContentWindowSizer()->Prepend(m_Logo, 0, wxEXPAND)->SetMinSize(GetLogoSize());
		AdjustWindow(wxDefaultPosition, wxSize(700, 450));
	}
}
KAboutDialog::~KAboutDialog()
{
}
