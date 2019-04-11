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
		m_ModSources.reserve(ModSourceIDs::MAX_SYSTEM);
		AddModSource<NexusProvider>();
		AddModSource<LoversLabProvider>();
		AddModSource<TESALLProvider>();

		// Load default source
		m_DefaultModSource = 0;
		if (INetworkModSource* modSource = FindModSource(GetAInstanceOption(OName::ModSource).GetAttribute(OName::Default)))
		{
			m_DefaultModSource = modSource->GetID();
		}
		KxFile(GetCacheFolder()).CreateFolder();
	}
	void DefaultNetworkManager::OnExit()
	{
		using namespace Application;

		const INetworkModSource* modSource = GetDefaultModSource();
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
			modSource->ValidateAuth();
		}
		SetDefaultProviderToFirstAvailableIfNone();
	}
	bool DefaultNetworkManager::SetDefaultProviderToFirstAvailableIfNone()
	{
		if (!IsDefaultProviderAvailable())
		{
			for (const auto& modSource: m_ModSources)
			{
				if (modSource->IsAuthenticated())
				{
					m_DefaultModSource = modSource->GetID();
					return true;
				}
			}

			m_DefaultModSource = ModSourceIDs::Invalid;
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
	bool DefaultNetworkManager::GetProviderInfo(const INetworkModSource& modSource, wxString& label, wxBitmap& bitmap, bool name) const
	{
		bool authOK = false;
		wxString userName;
		if (modSource.IsAuthenticated() && modSource.LoadAuthInfo(userName))
		{
			authOK = true;

			label += modSource.GetName();
			if (name)
			{
				label += wxS('/') + userName;
			}
			bitmap = modSource.GetUserPicture();
		}
		else
		{
			label = modSource.GetName();
			bitmap = KGetBitmap(KIMG_CROSS_CIRCLE_FRAME);
		}
		return authOK;
	}
	void DefaultNetworkManager::UpdateButton()
	{
		INetworkModSource* modSource = GetModSource(m_DefaultModSource);
		if (modSource && modSource->IsAuthenticated())
		{
			wxString label;
			wxBitmap bitmap;
			GetProviderInfo(*modSource, label, bitmap);

			m_LoginButton->SetLabel(KTr("Network.SignedIn") + ": " + label);
			m_LoginButton->SetBitmap(KGetBitmap(modSource->GetIcon()));
		}
		else
		{
			m_LoginButton->SetLabel(KTr("Network.NotSignedIn"));
			m_LoginButton->SetBitmap(KGetBitmap(INetworkModSource::GetGenericIcon()));
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
			wxString label;
			wxBitmap bitmap;
			bool authOK = GetProviderInfo(*modSource, label, bitmap);
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

		// Add current modSource selections
		m_Menu->AddSeparator();
		for (const auto& modSource: m_ModSources)
		{
			wxString label;
			wxBitmap bitmap;
			bool authOK = GetProviderInfo(*modSource, label, bitmap, false);
			if (!authOK)
			{
				label = KTr("Network.NotSignedIn") + ": " + label;
			}

			KxMenuItem* item = m_Menu->Add(new KxMenuItem(label, wxEmptyString, wxITEM_RADIO));
			item->Bind(KxEVT_MENU_SELECT, &DefaultNetworkManager::OnSelectActiveProvider, this);

			item->Check(m_DefaultModSource == modSource->GetID());
			item->Enable(authOK);
			item->SetBitmap(KGetBitmap(modSource->GetIcon()));
			item->SetClientData(modSource.get());
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
		INetworkModSource* modSource = static_cast<INetworkModSource*>(event.GetItem()->GetClientData());
		if (modSource->IsAuthenticated())
		{
			KxTaskDialog dialog(KMainWindow::GetInstance(), KxID_NONE, KTrf("Network.SignOutMessage", modSource->GetName()), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
			if (dialog.ShowModal() == KxID_YES)
			{
				modSource->SignOut();
				SetDefaultProviderToFirstAvailableIfNone();
				QueueUIUpdate();
			}
		}
		else
		{
			// Additional call to "INetworkModSource::IsAuthenticated' to make sure that modSource is ready
			// as authentication process can be async.
			if (modSource->Authenticate(KMainWindow::GetInstance()) && modSource->IsAuthenticated() && !IsDefaultProviderAvailable())
			{
				m_DefaultModSource = modSource->GetID();
			}
			QueueUIUpdate();
		}
	}
	void DefaultNetworkManager::OnSelectActiveProvider(KxMenuEvent& event)
	{
		INetworkModSource* modSource = static_cast<INetworkModSource*>(event.GetItem()->GetClientData());
		m_DefaultModSource = modSource->GetID();

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

	INetworkModSource* DefaultNetworkManager::GetDefaultModSource() const
	{
		return GetModSource(m_DefaultModSource);
	}

	INetworkModSource* DefaultNetworkManager::FindModSource(const wxString& name) const
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
	INetworkModSource* DefaultNetworkManager::GetModSource(ModSourceID sourceID) const
	{
		if (sourceID >= 0 && (size_t)sourceID < m_ModSources.size())
		{
			return m_ModSources[sourceID].get();
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
