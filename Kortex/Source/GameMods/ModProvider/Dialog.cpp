#include "stdafx.h"
#include "Dialog.h"
#include <KxFramework/KxButton.h>

namespace Kortex::ModProvider
{
	void Dialog::OnSelectItem(KxDataViewEvent& event)
	{
		const Item* node = GetNode(event.GetItem());
		m_RemoveButton->Enable(node && node->HasProvider());
	}
	void Dialog::OnAddProvider(wxCommandEvent& event)
	{
		Item& node = m_ProviderStore.AssignWith(wxEmptyString, wxEmptyString);
		m_IsModified = true;

		RefreshItems();
		SelectItem(MakeItem(node));
		GetView()->EditItem(MakeItem(node), GetView()->GetColumn(ColumnID::Name));
	}
	void Dialog::OnRemoveProvider(wxCommandEvent& event)
	{
		const Item* node = GetNode(GetView()->GetSelection());
		if (node)
		{
			m_ProviderStore.RemoveItem(node->GetName());
			m_IsModified = true;

			RefreshItems();
			GetView()->SetFocus();
		}
	}

	Dialog::Dialog(wxWindow* parent, Store& providerStore)
		:DisplayModel(providerStore)
	{
		if (KxStdDialog::Create(parent, KxID_NONE, KTr("ModManager.SitesEditor"), wxDefaultPosition, wxDefaultSize, KxBTN_OK))
		{
			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide(wxBOTH);

			m_RemoveButton = AddButton(KxID_REMOVE, wxEmptyString, true).As<KxButton>();
			m_RemoveButton->Bind(wxEVT_BUTTON, &Dialog::OnRemoveProvider, this);
			m_RemoveButton->Disable();

			m_AddButton = AddButton(KxID_ADD, wxEmptyString, true).As<KxButton>();
			m_AddButton->Bind(wxEVT_BUTTON, &Dialog::OnAddProvider, this);

			wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
			m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
			m_ViewPane->SetSizer(sizer);
			PostCreate();

			// List
			DisplayModel::Create(m_ViewPane, sizer);
			RefreshItems();

			AdjustWindow(wxDefaultPosition, wxSize(500, 350));
			GetView()->SetFocus();
		}
	}
	Dialog::~Dialog()
	{
		IncRef();
	}
}
