#include "stdafx.h"
#include "KAboutDialog.h"
#include "KThemeManager.h"
#include "KMainWindow.h"
#include "KWorkspace.h"
#include "KManager.h"
#include "Archive/KArchive.h"
#include "SettingsWindow/KSettingsWindowManager.h"
#include "PluginManager/LOOT API/KLootAPI.h"
#include "KAux.h"
#include "KApp.h"
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxCrypto.h>
#include <KxFramework/KxWebSockets.h>
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxJSON.h>
#include "VFS/KVFSService.h"

wxWindow* KAboutDialog::CreateTab_Info()
{
	KxHTMLWindow* info = new KxHTMLWindow(m_View, KxID_NONE);
	wxFont font = m_View->GetFont();
	info->SetStandardFonts(font.GetPointSize(), font.GetFaceName(), font.GetFaceName());
	info->SetBorders(KLC_HORIZONTAL_SPACING);
	info->Bind(wxEVT_HTML_LINK_CLICKED, &KAboutDialog::OnLinkClick, this);

	wxString libraries;
	auto AddLibrary = [&libraries](const char* name, const char* url, const char* extraName = NULL, const char* extraURL = NULL)
	{
		libraries << wxString::Format("<a href=\"%s\">%s</a>", url, name);
		if (extraName && extraURL)
		{
			libraries << wxString::Format(" (<a href=\"%s\">%s</a>)", extraURL, extraName);
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
		iconPacks << TF("About.Info.IconPack").arg(packName).arg(wxString::Format("<a href=\"%s\">%s</a>", url, authorName));
		iconPacks << "<br>";
	};
	AddIconPack("Fugue Icons", "Yusuke Kamiyamane", "http://p.yusukekamiyamane.com");
	AddIconPack("Tango 7-zip", "Laoism", "https://laoism.deviantart.com/art/Tango-7-zip-2-1-113983312");

	info->SetPage(wxString::Format("<div align=\"justify\">%s:<br>%s<br>%s</div>",
									T("About.Info.Libraries"), libraries,
									iconPacks));
	return info;
}
wxWindow* KAboutDialog::CreateTab_Modules()
{
	KxTreeList* list = new KxTreeList(m_View, KxID_NONE, KxTreeList::DefaultStyle|wxTL_NO_HEADER);
	list->GetDataView()->ToggleWindowStyle(wxBORDER_NONE);
	list->GetDataView()->SetAllowColumnsAutoSize(false);
	list->SetImageList(KApp::Get().GetImageList());
	list->AddColumn(wxEmptyString, 300);
	list->AddColumn(wxEmptyString, 75);

	for (KManager* manager: KManager::GetInstances())
	{
		// ConfigManager used by SettingsWindow should not be shown in this window
		if (manager != KApp::Get().GetSettingsManager())
		{
			KxTreeListItem item = list->GetRoot().Add(KxStringVector{manager->GetName(), manager->GetVersion()});
			item.SetImage(manager->GetImageID());
		}
	}

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
	if (KLootAPI* lootAPI = KLootAPI::GetInstance())
	{
		KxTreeListItem item = list->GetRoot().Add(KxStringVector{"LOOT API", lootAPI->GetVersion()});
		item.SetImage(KIMG_LOOT);
	}

	// WebSocket++
	{
		KxTreeListItem item = list->GetRoot().Add(KxStringVector{"WebSocket++", KxWebSocketClient::GetVersion()});
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
wxWindow* KAboutDialog::CreateTab_Permissions()
{
	KxHTMLWindow* info = new KxHTMLWindow(m_View, KxID_NONE);
	info->Bind(wxEVT_HTML_LINK_CLICKED, &KAboutDialog::OnLinkClick, this);
	wxFont font = m_View->GetFont();
	info->SetStandardFonts(font.GetPointSize(), font.GetFaceName(), font.GetFaceName());
	info->SetBorders(KLC_HORIZONTAL_SPACING);

	static const char* sSiteNameTA = "TESALL.RU";
	static const char* sSiteNameNexus = "NexusMods";
	static const char* sSiteTA = "http://tesall.ru";
	static const char* sSiteNexus = "https://www.nexusmods.com";
	static const char* sDeveloperProfileTA = "http://tesall.ru/user/5527-kerber";
	static const char* sDownloadPageTA = "http://tesall.ru/files/file/8153-kortex";
	static const char* sDownloadPageNexus = "https://www.nexusmods.com/skyrim/mods/90868";
	static const char* sSupportPageTA = "http://tesall.ru/topic/7489-kortex";

	auto MakeLink = [](const auto& name, const auto& url)
	{
		return wxString::Format("<a href=\"%s\">%s</a>", url, name);
	};
	auto MakePageLink = [](const auto& name, const auto& url, const char* siteName = NULL)
	{
		wxString link = wxString::Format("<a href=\"%s\">%s</a>", url, name);
		if (siteName)
		{
			link << " (" << siteName << ")";
		}
		return link;
	};

	wxString firstPart = TF("About.Permissions").arg(MakeLink(sSiteNameTA, sSiteTA)).arg(sSiteNameTA).arg(MakeLink(T("About.Permissions.ContactMethod"), sDeveloperProfileTA));
	wxString secondPart1 = MakePageLink(T("About.Permissions.DownloadPage"), sDownloadPageTA, sSiteNameTA);
	wxString secondPart2 = MakePageLink(T("About.Permissions.DownloadPage"), sDownloadPageNexus, sSiteNameNexus);
	wxString secondPart3 = MakePageLink(T("About.Permissions.SupportPage"), sSupportPageTA, sSiteNameTA);

	info->SetPage(wxString::Format("<div align=\"justify\">%s<br><br>%s<br>%s<br>%s</div>", firstPart, secondPart1, secondPart2, secondPart3));
	return info;
}

void KAboutDialog::OnLinkClick(wxHtmlLinkEvent& event)
{
	KAux::AskOpenURL(event.GetLinkInfo().GetHref(), this);
}

KAboutDialog::KAboutDialog(wxWindow* parent)
{
	if (KxStdDialog::Create(parent, KxID_NONE, V("$T(MainMenu.About) $(AppName)"), wxDefaultPosition, wxDefaultSize, KxBTN_OK))
	{
		SetDefaultBackgroundColor();
		GetContentWindow()->SetBackgroundColour(GetBackgroundColour());

		SetMainIcon(KxICON_NONE);
		SetWindowResizeSide((wxOrientation)0);

		m_View = new KxAuiNotebook(m_ContentPanel, KxID_NONE);
		KThemeManager::Get().ProcessWindow(m_View);

		m_View->AddPage(CreateTab_Info(), T("About.Tabs.Info"), true);
		m_View->AddPage(CreateTab_Modules(), T("About.Tabs.Modules"));
		m_View->AddPage(CreateTab_Permissions(), T("About.Tabs.Permissions"));

		PostCreate(wxDefaultPosition);
		SetLabel(wxString::Format("%s: %s\r\n%s: %s", T("About.Version"), KApp::Get().GetAppVersion(), T("About.Developer"), KApp::Get().GetVendorDisplayName()));
		AdjustWindow(wxDefaultPosition, wxSize(650, 425));
	}
}
KAboutDialog::~KAboutDialog()
{
}
