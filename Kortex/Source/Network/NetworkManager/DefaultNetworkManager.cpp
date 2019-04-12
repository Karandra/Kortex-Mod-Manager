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

namespace Kortex::NetworkManager
{
	void DefaultNetworkManager::OnInit()
	{
		using namespace NetworkManager;
		using namespace Application;

		// Init sources
		m_ModSources.reserve(3);
		AddModSource<NexusProvider>();
		AddModSource<LoversLabProvider>();
		AddModSource<TESALLProvider>();

		// Load default source
		if (IModSource* modSource = GetModSource(GetAInstanceOption(OName::ModSource).GetAttribute(OName::Default)))
		{
			m_DefaultModSource = modSource;
		}
		KxFile(GetCacheFolder()).CreateFolder();
	}
	void DefaultNetworkManager::OnExit()
	{
		using namespace Application;

		const IModSource* modSource = GetDefaultModSource();
		GetAInstanceOption(OName::ModSource).SetAttribute(OName::Default, modSource ? modSource->GetName() : wxEmptyString);
	}
	void DefaultNetworkManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		m_Config.OnLoadInstance(instance, managerNode);
	}

	void DefaultNetworkManager::ValidateAuth()
	{
		for (auto& modSource: m_ModSources)
		{
			if (auto auth = modSource->QueryInterface<IAuthenticableModSource>())
			{
				auth->ValidateAuth();
			}
		}
		SetDefaultProviderToFirstAvailableIfNone();
	}
	bool DefaultNetworkManager::SetDefaultProviderToFirstAvailableIfNone()
	{
		if (!IsDefaultModSourceAuthenticated())
		{
			m_DefaultModSource = nullptr;
			for (auto& modSource: m_ModSources)
			{
				const IAuthenticableModSource* auth = nullptr;
				if (modSource->QueryInterface(auth) && auth->IsAuthenticated())
				{
					m_DefaultModSource = modSource.get();
					return true;
				}
			}
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
	bool DefaultNetworkManager::GetModSourceInfo(const IModSource& modSource, wxString& label, wxBitmap& bitmap, bool name) const
	{
		const IAuthenticableModSource* auth = nullptr;
		if (modSource.QueryInterface(auth) && auth->IsAuthenticated())
		{
			if (auto credentials = auth->LoadCredentials())
			{
				label += modSource.GetName();
				if (name)
				{
					label += wxS('/') + credentials->UserID;
				}
				bitmap = modSource.GetUserPicture();

				return true;
			}
			else
			{
				label = modSource.GetName();
				bitmap = KGetBitmap(KIMG_CROSS_CIRCLE_FRAME);
			}
		}
		return false;
	}
	void DefaultNetworkManager::UpdateButton()
	{
		const IAuthenticableModSource* auth = nullptr;
		if (m_DefaultModSource && m_DefaultModSource->QueryInterface(auth) && auth->IsAuthenticated())
		{
			wxString label;
			wxBitmap bitmap;
			GetModSourceInfo(*m_DefaultModSource, label, bitmap);

			m_LoginButton->SetLabel(KTr("Network.SignedIn") + ": " + label);
			m_LoginButton->SetBitmap(KGetBitmap(m_DefaultModSource->GetIcon()));
		}
		else
		{
			m_LoginButton->SetLabel(KTr("Network.NotSignedIn"));
			m_LoginButton->SetBitmap(KGetBitmap(IModSource::GetGenericIcon()));
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
		for (const auto& modSource: m_ModSources)
		{
			if (modSource->QueryInterface<IAuthenticableModSource>())
			{
				wxString label;
				wxBitmap bitmap;
				bool authOK = GetModSourceInfo(*modSource, label, bitmap);
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
				item->SetClientData(modSource.get());
			}
		}

		// Add current modSource selections
		m_Menu->AddSeparator();
		for (const auto& modSource: m_ModSources)
		{
			if (modSource->QueryInterface<IAuthenticableModSource>())
			{
				wxString label;
				wxBitmap bitmap;
				bool authOK = GetModSourceInfo(*modSource, label, bitmap, false);
				if (!authOK)
				{
					label = KTr("Network.NotSignedIn") + ": " + label;
				}

				KxMenuItem* item = m_Menu->Add(new KxMenuItem(label, wxEmptyString, wxITEM_RADIO));
				item->Bind(KxEVT_MENU_SELECT, &DefaultNetworkManager::OnSelectActiveModSource, this);

				item->Check(m_DefaultModSource == modSource.get());
				item->Enable(authOK);
				item->SetBitmap(KGetBitmap(modSource->GetIcon()));
				item->SetClientData(modSource.get());
			}
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
		IModSource* modSource = static_cast<IModSource*>(event.GetItem()->GetClientData());
		if (auto auth = modSource->QueryInterface<IAuthenticableModSource>(); auth && auth->IsAuthenticated())
		{
			KxTaskDialog dialog(KMainWindow::GetInstance(), KxID_NONE, KTrf("Network.SignOutMessage", modSource->GetName()), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
			if (dialog.ShowModal() == KxID_YES)
			{
				auth->SignOut();
				SetDefaultProviderToFirstAvailableIfNone();
				QueueUIUpdate();
			}
		}
		else
		{
			// Additional call to "IModSource::IsAuthenticated' to make sure that modSource is ready
			// as authentication process can be async.
			if (auth->Authenticate() && auth->IsAuthenticated() && !IsDefaultModSourceAuthenticated())
			{
				m_DefaultModSource = modSource;
			}
			QueueUIUpdate();
		}
	}
	void DefaultNetworkManager::OnSelectActiveModSource(KxMenuEvent& event)
	{
		IModSource* modSource = static_cast<IModSource*>(event.GetItem()->GetClientData());
		m_DefaultModSource = modSource;

		UpdateButton();
		GetAInstanceOption().SetAttribute("DefaultProvider", modSource->GetName());
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

	IModSource* DefaultNetworkManager::GetDefaultModSource() const
	{
		return m_DefaultModSource;
	}
	IModSource* DefaultNetworkManager::GetModSource(const wxString& name) const
	{
		for (const auto& modSource: m_ModSources)
		{
			if (modSource->GetName() == name)
			{
				return modSource.get();
			}
		}
		return nullptr;
	}
}

namespace Kortex::NetworkManager
{
	void Config::OnLoadInstance(IGameInstance& profile, const KxXMLNode& node)
	{
		m_NexusID = node.GetFirstChildElement("NexusID").GetValue();
		m_SteamID = node.GetFirstChildElement("SteamID").GetValueInt(m_SteamID);
	}
	wxString Config::GetNexusID() const
	{
		return KVarExp(m_NexusID);
	}
}
