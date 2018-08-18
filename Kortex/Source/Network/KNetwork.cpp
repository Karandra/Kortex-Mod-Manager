#include "stdafx.h"
#include "KNetwork.h"
#include "KNetworkProviderNexus.h"
#include "KNetworkProviderTESALL.h"
#include "KNetworkProviderLoversLab.h"
#include "UI/KMainWindow.h"
#include "DownloadManager/KDownloadManager.h"
#include "Profile/KProfileID.h"
#include "Profile/KNetworkConfig.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxMenu.h>

KxSingletonPtr_Define(KNetwork);

void KNetwork::ValidateAuth()
{
	for (auto& provider: m_Providers)
	{
		provider->ValidateAuth();
	}
	SetCurrentProviderToFirstAvailableIfNone();
}
bool KNetwork::SetCurrentProviderToFirstAvailableIfNone()
{
	if (!IsCurrentProviderAvailable())
	{
		for (const auto& provider: m_Providers)
		{
			if (provider->IsAuthenticated())
			{
				m_CurrentProvider = provider->GetID();
				return true;
			}
		}

		m_CurrentProvider = KNETWORK_PROVIDER_ID_INVALID;
		return false;
	}
	return true;
}

void KNetwork::SetLoginButton(KxAuiToolBarItem* button)
{
	m_LoginButton = button;

	ValidateAuth();
	UpdateButton();
	CreateMenu();
}
bool KNetwork::GetProviderInfo(const KNetworkProvider& provider, wxString& label, wxBitmap& bitmap, bool name) const
{
	bool authOK = false;
	wxString userName;
	if (provider.IsAuthenticated() && provider.LoadAuthInfo(userName))
	{
		authOK = true;

		label += provider.GetName();
		if (name)
		{
			label += '/' + userName;
		}
		bitmap = provider.GetUserPicture();
	}
	else
	{
		label = provider.GetName();
		bitmap = KGetBitmap(KIMG_CROSS_CIRCLE_FRAME);
	}
	return authOK;
}
void KNetwork::UpdateButton()
{
	KNetworkProvider* provider = GetProvider(m_CurrentProvider);
	if (provider && provider->IsAuthenticated())
	{
		wxString label;
		wxBitmap bitmap;
		GetProviderInfo(*provider, label, bitmap);

		m_LoginButton->SetLabel(T("Network.SignedIn") + ": " + label);
		m_LoginButton->SetBitmap(KGetBitmap(provider->GetIcon()));
	}
	else
	{
		m_LoginButton->SetLabel(T("Network.NotSignedIn"));
		m_LoginButton->SetBitmap(KGetBitmap(KNetworkProvider::GetGenericIcon()));
	}

	m_LoginButton->GetToolBar()->Realize();
	m_LoginButton->GetToolBar()->Refresh();
	KMainWindow::GetInstance()->Layout();
}
void KNetwork::CreateMenu()
{
	KxMenu::EndMenu();
	m_Menu = new KxMenu();
	m_LoginButton->AssignDropdownMenu(m_Menu);

	// Add sign-in/sign-out items.
	for (const auto& provider: m_Providers)
	{
		wxString label;
		wxBitmap bitmap;
		bool authOK = GetProviderInfo(*provider, label, bitmap);
		if (authOK)
		{
			label = T("Network.SignOut") + ": " + label;
		}
		else
		{
			label = T("Network.SignIn") + ": " + label;
		}

		KxMenuItem* item = m_Menu->Add(new KxMenuItem(label));
		item->Bind(KxEVT_MENU_SELECT, &KNetwork::OnSignInOut, this);
		
		item->SetBitmap(bitmap);
		item->SetClientData(provider.get());
	}

	// Add current provider selections
	m_Menu->AddSeparator();
	for (const auto& provider: m_Providers)
	{
		wxString label;
		wxBitmap bitmap;
		bool authOK = GetProviderInfo(*provider, label, bitmap, false);
		if (!authOK)
		{
			label = T("Network.NotSignedIn") + ": " + label;
		}

		KxMenuItem* item = m_Menu->Add(new KxMenuItem(label, wxEmptyString, wxITEM_RADIO));
		item->Bind(KxEVT_MENU_SELECT, &KNetwork::OnSelectProvider, this);
		
		item->Check(m_CurrentProvider == provider->GetID());
		item->Enable(authOK);
		item->SetBitmap(KGetBitmap(provider->GetIcon()));
		item->SetClientData(provider.get());
	}
}

void KNetwork::QueueUIUpdate()
{
	CallAfter([this]()
	{
		CreateMenu();
		UpdateButton();
	});
}

void KNetwork::OnSignInOut(KxMenuEvent& event)
{
	KNetworkProvider* provider = static_cast<KNetworkProvider*>(event.GetItem()->GetClientData());
	if (provider->IsAuthenticated())
	{
		KxTaskDialog dialog(KMainWindow::GetInstance(), KxID_NONE, T("Network.SignOutMessage", provider->GetName()), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
		if (dialog.ShowModal() == KxID_YES)
		{
			provider->SignOut();
			SetCurrentProviderToFirstAvailableIfNone();
			QueueUIUpdate();
		}
	}
	else
	{
		// Additional call to "KNetworkProvider::IsAuthenticated' to make sure that provider is ready
		// as authentication process can be async.
		if (provider->Authenticate(KMainWindow::GetInstance()) && provider->IsAuthenticated() && !IsCurrentProviderAvailable())
		{
			m_CurrentProvider = provider->GetID();
		}
		QueueUIUpdate();
	}
}
void KNetwork::OnSelectProvider(KxMenuEvent& event)
{
	KNetworkProvider* provider = static_cast<KNetworkProvider*>(event.GetItem()->GetClientData());
	m_CurrentProvider = provider->GetID();

	UpdateButton();
	m_Options.SetAttribute("CurrentProvider", m_CurrentProvider);
}
void KNetwork::OnLoginButton(KxAuiToolBarEvent& event)
{
	m_Menu->Show(m_LoginButton->GetToolBar(), m_LoginButton->GetDropdownMenuPosition());
}

KNetwork::KNetwork()
	:m_Options("KNetwork", wxEmptyString)
{
	KxFile(GetCacheFolder()).CreateFolder();
	m_DownloadManager = std::make_unique<KDownloadManager>();

	// Init providers
	NewProvider<KNetworkProviderNexus>();
	NewProvider<KNetworkProviderTESALL>();
	NewProvider<KNetworkProviderLoversLab>();

	// Load current provider
	m_CurrentProvider = static_cast<KNetworkProviderID>(m_Options.GetAttributeInt("CurrentProvider", KNETWORK_PROVIDER_ID_FIRST));
	if (m_CurrentProvider < KNETWORK_PROVIDER_ID_FIRST || m_CurrentProvider >= KNETWORK_PROVIDER_ID_MAX)
	{
		m_CurrentProvider = KNETWORK_PROVIDER_ID_FIRST;
	}
}
KNetwork::~KNetwork()
{
	m_Options.SetAttribute("CurrentProvider", m_CurrentProvider);
}

void KNetwork::OnAuthStateChanged()
{
	CreateMenu();
	UpdateButton();

	SetCurrentProviderToFirstAvailableIfNone();
}

wxString KNetwork::GetUniqueID() const
{
	return "0882E0D3-1619-406F-AEDE-96FD4C3DC9AB";
}
wxString KNetwork::GetCacheFolder() const
{
	return KApp::Get().GetUserSettingsFolder() + "\\WebCache";
}

bool KNetwork::IsCurrentProviderAvailable() const
{
	KNetworkProvider* provider = GetCurrentProvider();
	if (provider && provider->IsAuthenticated())
	{
		return true;
	}
	return false;
}
KNetworkProvider* KNetwork::GetCurrentProvider() const
{
	return GetProvider(m_CurrentProvider);
}

KNetworkProvider* KNetwork::FindProvider(const wxString& name) const
{
	for (const auto& provider: m_Providers)
	{
		if (provider->GetName() == name)
		{
			return provider.get();
		}
	}
	return NULL;
}
