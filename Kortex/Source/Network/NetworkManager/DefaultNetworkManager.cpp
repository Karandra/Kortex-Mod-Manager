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
		AddModSource<NexusSource>();
		AddModSource<LoversLabSource>();
		AddModSource<TESALLSource>();

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
		AdjustDefaultModSource();
	}
	bool DefaultNetworkManager::AdjustDefaultModSource()
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
	void DefaultNetworkManager::UpdateButton()
	{
		const IAuthenticableModSource* auth = nullptr;
		if (m_DefaultModSource && m_DefaultModSource->QueryInterface(auth) && auth->IsAuthenticated())
		{
			m_LoginButton->SetLabel(KTr("NetworkManager.SignedIn") + ": " + m_DefaultModSource->GetName());
			m_LoginButton->SetBitmap(KGetBitmap(m_DefaultModSource->GetIcon()));
		}
		else
		{
			m_LoginButton->SetLabel(KTr("NetworkManager.NotSignedIn"));
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

		for (const auto& modSource: m_ModSources)
		{
			if (KxMenu* subMenu = new KxMenu(); true)
			{
				KxMenuItem* rootItem = m_Menu->Add(subMenu, modSource->GetName());
				rootItem->SetBitmap(KGetBitmap(modSource->GetIcon()));

				const IAuthenticableModSource* authenticable = modSource->QueryInterface<IAuthenticableModSource>();
				const IModRepository* repository = modSource->QueryInterface<IModRepository>();

				// Add default source toggle
				{
					KxMenuItem* item = subMenu->Add(new KxMenuItem(wxEmptyString, wxEmptyString, wxITEM_CHECK));
					item->SetClientData(modSource.get());
					item->Enable(false);

					if (authenticable)
					{
						if (m_DefaultModSource == modSource.get())
						{
							item->Check();
							item->SetItemLabel(KTr("NetworkManager.ModSource.Default"));
						}
						else if (!authenticable->IsAuthenticated())
						{
							item->SetItemLabel(KxString::Format("%1 (%2)", KTr("NetworkManager.ModSource.MakeDefault"), KTr("NetworkManager.NotSignedIn")));
						}
						else
						{
							item->Enable();
							item->SetItemLabel(KTr("NetworkManager.ModSource.MakeDefault"));
							item->Bind(KxEVT_MENU_SELECT, &DefaultNetworkManager::OnSelectDefaultModSource, this);
						}
					}
					else
					{
						item->SetItemLabel(KTr("NetworkManager.ModSource.MakeDefault"));
					}
				}

				// Add sign-in/sign-out items.
				if (authenticable)
				{
					wxString label;
					if (authenticable->IsAuthenticated())
					{
						label = KTr("NetworkManager.SignOut") + ": " + modSource->GetName();
					}
					else
					{
						label = KTr("NetworkManager.SignIn") + ": " + modSource->GetName();
					}
					
					KxMenuItem* item = subMenu->Add(new KxMenuItem(label));
					item->Bind(KxEVT_MENU_SELECT, &DefaultNetworkManager::OnSignInOut, this);
					item->SetBitmap(modSource->GetUserPicture());
					item->SetClientData(modSource.get());
				}

				// Add limits information display
				if (repository)
				{
					if (ModRepositoryLimits limits = repository->GetRequestLimits(); limits.IsOK() && limits.HasLimits())
					{
						wxString label;
						auto AddSeparator = [&label]()
						{
							if (!label.IsEmpty())
							{
								label += wxS("; ");
							}
						};

						if (limits.HasHourlyLimit())
						{
							label += KTrf(wxS("NetworkManager.QueryLimits.Hourly"), limits.GetHourlyRemaining(), limits.GetHourlyLimit());
						}
						if (limits.HasDailyLimit())
						{
							AddSeparator();
							label += KTrf(wxS("NetworkManager.QueryLimits.Daily"), limits.GetDailyRemaining(), limits.GetDailyLimit());
						}
						label = KxString::Format(wxS("%1: [%2]"), KTr(wxS("NetworkManager.QueryLimits")), label);

						KxMenuItem* item = subMenu->Add(new KxMenuItem(label));
						item->SetBitmap(KGetBitmap(limits.AnyLimitDepleted() ? KIMG_EXCLAMATION : KIMG_TICK_CIRCLE_FRAME));
						item->SetClientData(modSource.get());
						item->Enable(false);
					}
				}
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
			KxTaskDialog dialog(KMainWindow::GetInstance(), KxID_NONE, KTrf("NetworkManager.SignOutMessage", modSource->GetName()), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
			if (dialog.ShowModal() == KxID_YES)
			{
				auth->SignOut();
				AdjustDefaultModSource();
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
	void DefaultNetworkManager::OnSelectDefaultModSource(KxMenuEvent& event)
	{
		IModSource* modSource = static_cast<IModSource*>(event.GetItem()->GetClientData());
		m_DefaultModSource = modSource;

		UpdateButton();
		GetAInstanceOption().SetAttribute("DefaultProvider", modSource->GetName());
	}
	void DefaultNetworkManager::OnToolBarButton(KxAuiToolBarEvent& event)
	{
		m_LoginButton->ShowDropdownMenuLeftAlign();
	}

	void DefaultNetworkManager::OnAuthStateChanged()
	{
		AdjustDefaultModSource();

		CreateMenu();
		UpdateButton();
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
