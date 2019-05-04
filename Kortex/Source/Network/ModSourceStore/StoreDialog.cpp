#include "stdafx.h"
#include "StoreDialog.h"
#include "Application/Resources/IImageProvider.h"
#include "Utility/KAux.h"
#include <Kortex/NetworkManager.hpp>
#include <KxFramework/KxButton.h>

namespace Kortex::ModSource
{
	void StoreDialog::CreateAddMenu()
	{
		// Add known mod networks
		for (const auto& modNetwork: INetworkManager::GetInstance()->GetModNetworks())
		{
			KxMenuItem* item = m_AddButtonMenu.Add(new KxMenuItem(modNetwork->GetName()));
			item->SetBitmap(ImageProvider::GetBitmap(modNetwork->GetIcon()));
			item->Enable(!m_ModSourceStore.HasItem(*modNetwork));

			item->Bind(KxEVT_MENU_SELECT, [this, &modNetwork](KxMenuEvent& event)
			{
				OnAddItem(m_ModSourceStore.AssignWith(*modNetwork, ModID()));
			});
		}

		// Add generic
		m_AddButtonMenu.AddSeparator();
		KxMenuItem* item = m_AddButtonMenu.Add(new KxMenuItem(KAux::MakeBracketedLabel(KTr(KxID_NEW))));
		item->SetBitmap(ImageProvider::GetBitmap(IModNetwork::GetGenericIcon()));
		item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			OnAddItem(m_ModSourceStore.AssignWith(wxEmptyString, wxEmptyString));
		});
	}
	void StoreDialog::OnAddItem(ModSourceItem& node)
	{
		m_IsModified = true;

		RefreshItems();
		SelectItem(MakeItem(node));
		GetView()->EditItem(MakeItem(node), GetView()->GetColumn(node.HasModNetwork() ? ColumnID::Value : ColumnID::Name));
	}

	void StoreDialog::OnSelectItem(KxDataViewEvent& event)
	{
		const ModSourceItem* node = GetNode(event.GetItem());
		m_RemoveButton->Enable(node != nullptr);
	}
	void StoreDialog::OnAddProvider(wxCommandEvent& event)
	{
		m_AddButtonMenu.ShowAsPopup(m_AddButton);
	}
	void StoreDialog::OnRemoveProvider(wxCommandEvent& event)
	{
		ModSourceItem* node = GetNode(GetView()->GetSelection());
		if (node)
		{
			m_ModSourceStore.RemoveItem(node->GetName());
			m_IsModified = true;
			
			RefreshItems();
			GetView()->SetFocus();
		}
	}

	StoreDialog::StoreDialog(wxWindow* parent, ModSourceStore& store)
		:StoreDisplayModel(store)
	{
		if (KxStdDialog::Create(parent, KxID_NONE, KTr("ModManager.SitesEditor"), wxDefaultPosition, wxDefaultSize, KxBTN_OK))
		{
			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide(wxBOTH);
			Bind(wxEVT_CLOSE_WINDOW, &StoreDialog::OnCloseDialog, this);

			m_RemoveButton = AddButton(KxID_REMOVE, wxEmptyString, true).As<KxButton>();
			m_RemoveButton->Bind(wxEVT_BUTTON, &StoreDialog::OnRemoveProvider, this);
			m_RemoveButton->Disable();

			m_AddButton = AddButton(KxID_ADD, wxEmptyString, true).As<KxButton>();
			m_AddButton->Bind(wxEVT_BUTTON, &StoreDialog::OnAddProvider, this);
			CreateAddMenu();

			wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
			m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
			m_ViewPane->SetSizer(sizer);
			PostCreate();

			// List
			StoreDisplayModel::Create(m_ViewPane, sizer);
			GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &StoreDialog::OnSelectItem, this);
			RefreshItems();

			AdjustWindow(wxDefaultPosition, wxSize(500, 350));
			GetView()->SetFocus();
		}
	}
	StoreDialog::~StoreDialog()
	{
		IncRef();
	}

	void StoreDialog::OnCloseDialog(wxCloseEvent& event)
	{
		ApplyChanges();
		event.Skip();
	}
}
