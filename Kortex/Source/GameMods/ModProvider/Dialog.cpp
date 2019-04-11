#include "stdafx.h"
#include "Dialog.h"
#include "Utility/KAux.h"
#include <Kortex/NetworkManager.hpp>
#include <KxFramework/KxButton.h>

namespace Kortex::ModProvider
{
	void Dialog::CreateAddMenu()
	{
		// Add known modSource
		for (const auto& modSource: INetworkManager::GetInstance()->GetModSources())
		{
			KxMenuItem* item = m_AddButtonMenu.Add(new KxMenuItem(modSource->GetName()));
			item->SetBitmap(KGetBitmap(modSource->GetIcon()));
			item->Enable(!m_ModSourceStore.HasItem(*modSource));

			item->Bind(KxEVT_MENU_SELECT, [this, &modSource](KxMenuEvent& event)
			{
				OnAddItem(m_ModSourceStore.AssignWith(*modSource, ModID()));
			});
		}

		// Add generic
		m_AddButtonMenu.AddSeparator();
		KxMenuItem* item = m_AddButtonMenu.Add(new KxMenuItem(KAux::MakeBracketedLabel(KTr(KxID_NEW))));
		item->SetBitmap(KGetBitmap(INetworkModSource::GetGenericIcon()));
		item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			OnAddItem(m_ModSourceStore.AssignWith(wxEmptyString, wxEmptyString));
		});
	}
	void Dialog::OnAddItem(ModSourceItem& node)
	{
		m_IsModified = true;

		RefreshItems();
		SelectItem(MakeItem(node));
		GetView()->EditItem(MakeItem(node), GetView()->GetColumn(node.HasModSource() ? ColumnID::Value : ColumnID::Name));
	}

	void Dialog::OnSelectItem(KxDataViewEvent& event)
	{
		const ModSourceItem* node = GetNode(event.GetItem());
		m_RemoveButton->Enable(node != nullptr);
	}
	void Dialog::OnAddProvider(wxCommandEvent& event)
	{
		m_AddButtonMenu.ShowAsPopup(m_AddButton);
	}
	void Dialog::OnRemoveProvider(wxCommandEvent& event)
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

	Dialog::Dialog(wxWindow* parent, ModSourceStore& store)
		:DisplayModel(store)
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
