#include "stdafx.h"
#include "DefaultNetworkManager.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Events.hpp>
#include "UI/KMainWindow.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxMenu.h>

using namespace Kortex::Application::Options;

namespace Kortex::NetworkManager
{
	void DefaultNetworkManager::OnInit()
	{
		using namespace Network;

		// Init providers
		m_Providers.reserve(ProviderIDs::MAX_SYSTEM);
		AddProvider<NexusProvider>();
		AddProvider<TESALLProvider>();
		AddProvider<LoversLabProvider>();

		// Load default provider
		m_DefaultProvider = 0;
		if (INetworkProvider* provider = FindProvider(GetAInstanceOption(Option::Provider).GetAttribute(Provider::Default)))
		{
			m_DefaultProvider = provider->GetID();
		}
		KxFile(GetCacheFolder()).CreateFolder();
	}
	void DefaultNetworkManager::OnExit()
	{
		const INetworkProvider* provider = GetDefaultProvider();
		GetAInstanceOption(Option::Provider).SetAttribute(Provider::Default, provider ? provider->GetName() : wxEmptyString);
	}
	void DefaultNetworkManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		m_Config.OnLoadInstance(instance, managerNode);
	}

	void DefaultNetworkManager::ValidateAuth()
	{
		for (auto& provider: m_Providers)
		{
			provider->ValidateAuth();
		}
		SetDefaultProviderToFirstAvailableIfNone();
	}
	bool DefaultNetworkManager::SetDefaultProviderToFirstAvailableIfNone()
	{
		if (!IsDefaultProviderAvailable())
		{
			for (const auto& provider: m_Providers)
			{
				if (provider->IsAuthenticated())
				{
					m_DefaultProvider = provider->GetID();
					return true;
				}
			}

			m_DefaultProvider = Kortex::Network::ProviderIDs::Invalid;
			return false;
		}
		return true;
	}

	void DefaultNetworkManager::OnSetToolBarButton(KxAuiToolBarItem* button)
	{
		m_LoginButton = button;

		ValidateAuth();
		UpdateButton();
		CreateMenu();
	}
	bool DefaultNetworkManager::GetProviderInfo(const INetworkProvider& provider, wxString& label, wxBitmap& bitmap, bool name) const
	{
		bool authOK = false;
		wxString userName;
		if (provider.IsAuthenticated() && provider.LoadAuthInfo(userName))
		{
			authOK = true;

			label += provider.GetName();
			if (name)
			{
				label += wxS('/') + userName;
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
	void DefaultNetworkManager::UpdateButton()
	{
		INetworkProvider* provider = GetProvider(m_DefaultProvider);
		if (provider && provider->IsAuthenticated())
		{
			wxString label;
			wxBitmap bitmap;
			GetProviderInfo(*provider, label, bitmap);

			m_LoginButton->SetLabel(KTr("Network.SignedIn") + ": " + label);
			m_LoginButton->SetBitmap(KGetBitmap(provider->GetIcon()));
		}
		else
		{
			m_LoginButton->SetLabel(KTr("Network.NotSignedIn"));
			m_LoginButton->SetBitmap(KGetBitmap(INetworkProvider::GetGenericIcon()));
		}

		m_LoginButton->GetToolBar()->Realize();
		m_LoginButton->GetToolBar()->Refresh();
		KMainWindow::GetInstance()->Layout();
	}
	void DefaultNetworkManager::CreateMenu()
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
				label = KTr("Network.SignOut") + ": " + label;
			}
			else
			{
				label = KTr("Network.SignIn") + ": " + label;
			}

			KxMenuItem* item = m_Menu->Add(new KxMenuItem(label));
			item->Bind(KxEVT_MENU_SELECT, &DefaultNetworkManager::OnSignInOut, this);

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
				label = KTr("Network.NotSignedIn") + ": " + label;
			}

			KxMenuItem* item = m_Menu->Add(new KxMenuItem(label, wxEmptyString, wxITEM_RADIO));
			item->Bind(KxEVT_MENU_SELECT, &DefaultNetworkManager::OnSelectActiveProvider, this);

			item->Check(m_DefaultProvider == provider->GetID());
			item->Enable(authOK);
			item->SetBitmap(KGetBitmap(provider->GetIcon()));
			item->SetClientData(provider.get());
		}
	}
	void DefaultNetworkManager::QueueUIUpdate()
	{
		IEvent::CallAfter([this]()
		{
			CreateMenu();
			UpdateButton();
		});
	}

	void DefaultNetworkManager::OnSignInOut(KxMenuEvent& event)
	{
		INetworkProvider* provider = static_cast<INetworkProvider*>(event.GetItem()->GetClientData());
		if (provider->IsAuthenticated())
		{
			KxTaskDialog dialog(KMainWindow::GetInstance(), KxID_NONE, KTrf("Network.SignOutMessage", provider->GetName()), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
			if (dialog.ShowModal() == KxID_YES)
			{
				provider->SignOut();
				SetDefaultProviderToFirstAvailableIfNone();
				QueueUIUpdate();
			}
		}
		else
		{
			// Additional call to "INetworkProvider::IsAuthenticated' to make sure that provider is ready
			// as authentication process can be async.
			if (provider->Authenticate(KMainWindow::GetInstance()) && provider->IsAuthenticated() && !IsDefaultProviderAvailable())
			{
				m_DefaultProvider = provider->GetID();
			}
			QueueUIUpdate();
		}
	}
	void DefaultNetworkManager::OnSelectActiveProvider(KxMenuEvent& event)
	{
		INetworkProvider* provider = static_cast<INetworkProvider*>(event.GetItem()->GetClientData());
		m_DefaultProvider = provider->GetID();

		UpdateButton();
		GetAInstanceOption().SetAttribute("DefaultProvider", provider->GetName());
	}
	void DefaultNetworkManager::OnToolBarButton(KxAuiToolBarEvent& event)
	{
		wxPoint pos = m_LoginButton->GetDropdownMenuPosition();
		m_Menu->Show(m_LoginButton->GetToolBar(), pos);
	}

	void DefaultNetworkManager::OnAuthStateChanged()
	{
		CreateMenu();
		UpdateButton();

		SetDefaultProviderToFirstAvailableIfNone();
	}
	wxString DefaultNetworkManager::GetCacheFolder() const
	{
		return IApplication::GetInstance()->GetUserSettingsFolder() + wxS("\\WebCache");
	}

	INetworkProvider* DefaultNetworkManager::GetDefaultProvider() const
	{
		return GetProvider(m_DefaultProvider);
	}

	INetworkProvider* DefaultNetworkManager::FindProvider(const wxString& name) const
	{
		for (const auto& provider: m_Providers)
		{
			if (provider->GetName() == name)
			{
				return provider.get();
			}
		}
		return nullptr;
	}
	INetworkProvider* DefaultNetworkManager::GetProvider(Network::ProviderID providerID) const
	{
		if (providerID >= 0 && (size_t)providerID < m_Providers.size())
		{
			return m_Providers[providerID].get();
		}
		return nullptr;
	}
}

namespace Kortex::NetworkManager
{
	void Config::OnLoadInstance(IGameInstance& profile, const KxXMLNode& node)
	{
		m_NexusID = node.GetFirstChildElement("NexusID").GetValue();
		m_SteamID = node.GetFirstChildElement("NexusID").GetValueInt(m_SteamID);
	}
	wxString Config::GetNexusID() const
	{
		return KVarExp(m_NexusID);
	}
}
