#include "stdafx.h"
#include "Dialog.h"
#include "Utility/KAux.h"
#include <Kortex/NetworkManager.hpp>
#include <KxFramework/KxButton.h>

namespace Kortex::ModProvider
{
	void Dialog::CreateAddMenu()
	{
		// Add known provider
		for (const auto& provider: INetworkManager::GetInstance()->GetProviders())
		{
			KxMenuItem* item = m_AddButtonMenu.Add(new KxMenuItem(provider->GetName()));
			item->SetBitmap(KGetBitmap(provider->GetIcon()));
			item->Enable(!m_ProviderStore.HasItem(*provider));

			item->Bind(KxEVT_MENU_SELECT, [this, &provider](KxMenuEvent& event)
			{
				OnAddItem(m_ProviderStore.AssignWith(*provider, ModID()));
			});
		}

		// Add generic
		m_AddButtonMenu.AddSeparator();
		KxMenuItem* item = m_AddButtonMenu.Add(new KxMenuItem(KAux::MakeBracketedLabel(KTr(KxID_NEW))));
		item->SetBitmap(KGetBitmap(INetworkProvider::GetGenericIcon()));
		item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			OnAddItem(m_ProviderStore.AssignWith(wxEmptyString, wxEmptyString));
		});
	}
	void Dialog::OnAddItem(ModProviderItem& node)
	{
		m_IsModified = true;

		RefreshItems();
		SelectItem(MakeItem(node));
		GetView()->EditItem(MakeItem(node), GetView()->GetColumn(node.HasProvider() ? ColumnID::Value : ColumnID::Name));
	}

	void Dialog::OnSelectItem(KxDataViewEvent& event)
	{
		const ModProviderItem* node = GetNode(event.GetItem());
		m_RemoveButton->Enable(node != nullptr);
	}
	void Dialog::OnAddProvider(wxCommandEvent& event)
	{
		m_AddButtonMenu.ShowAsPopup(m_AddButton);
	}
	void Dialog::OnRemoveProvider(wxCommandEvent& event)
	{
		ModProviderItem* node = GetNode(GetView()->GetSelection());
		if (node)
		{
			m_ProviderStore.RemoveItem(node->GetName());
			m_IsModified = true;
			
			RefreshItems();
			GetView()->SetFocus();
		}
	}

	Dialog::Dialog(wxWindow* parent, ModProviderStore& providerStore)
		:DisplayModel(providerStore)
	{
		if (KxStdDialog::Create(parent, KxID_NONE, KTr("ModManager.SitesEditor"), wxDefaultPosition, wxDefaultSize, KxBTN_OK))
		{
			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide(wxBOTH);
			Bind(wxEVT_CLOSE_WINDOW, &Dialog::OnCloseDialog, this);

			m_RemoveButton = AddButton(KxID_REMOVE, wxEmptyString, true).As<KxButton>();
			m_RemoveButton->Bind(wxEVT_BUTTON, &Dialog::OnRemoveProvider, this);
			m_RemoveButton->Disable();

			m_AddButton = AddButton(KxID_ADD, wxEmptyString, true).As<KxButton>();
			m_AddButton->Bind(wxEVT_BUTTON, &Dialog::OnAddProvider, this);
			CreateAddMenu();

			wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
			m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
			m_ViewPane->SetSizer(sizer);
			PostCreate();

			// List
			DisplayModel::Create(m_ViewPane, sizer);
			GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &Dialog::OnSelectItem, this);
			RefreshItems();

			AdjustWindow(wxDefaultPosition, wxSize(500, 350));
			GetView()->SetFocus();
		}
	}
	Dialog::~Dialog()
	{
		IncRef();
	}

	void Dialog::OnCloseDialog(wxCloseEvent& event)
	{
		ApplyChanges();
		event.Skip();
	}
}
