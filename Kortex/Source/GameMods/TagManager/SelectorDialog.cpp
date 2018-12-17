#include "stdafx.h"
#include "SelectorDialog.h"
#include <Kortex/ModTagManager.hpp>
#include <KxFramework/KxDataViewComboBox.h>
#include <KxFramework/KxButton.h>

namespace Kortex::ModTagManager
{
	bool SelectorDialog::IsEditorEnabledByRow(size_t row, const KxDataViewColumn* column) const
	{
		const IModTag* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					return !entry->IsSystemTag();
				}
			};
		}
		return SelectorDisplayModel::IsEditorEnabledByRow(row, column);
	}
	void SelectorDialog::OnSelectItem(KxDataViewEvent& event)
	{
		const IModTag* entry = GetDataEntry(event.GetItem());
		m_RemoveButton->Enable(entry);
	}

	void SelectorDialog::OnAddTag(wxCommandEvent& event)
	{
		IModTagManager::GetInstance()->EmplaceTagWith(wxEmptyString, KAux::MakeBracketedLabel(KTr(KxID_NEW)));
		RefreshItems();

		KxDataViewItem newItem = GetItem(GetItemCount() - 1);
		SelectItem(newItem);
		GetView()->EditItem(newItem, GetView()->GetColumnByID(ColumnID::Name));
	}
	void SelectorDialog::OnRemoveTag(wxCommandEvent& event)
	{
		KxDataViewItem item = GetView()->GetSelection();
		IModTag* entry = GetDataEntry(item);
		if (entry)
		{
			// Find all mods with this tag and remove it from them
			for (auto& modEntry: IModManager::GetInstance()->GetMods())
			{
				ModTagStore& tagStore = modEntry->GetTagStore();
				if (tagStore.HasTag(*entry))
				{
					tagStore.RemoveTag(*entry);

					m_IsModified = true;
					if (m_AllowSave)
					{
						modEntry->Save();
					}
				}
			}

			// Remove tag from system
			IModTagManager::GetInstance()->RemoveTag(*entry);

			// Reload control
			KxDataViewItem prevItem = GetPrevItem(item);
			RefreshItems();
			SelectItem(prevItem);
			GetView()->SetFocus();
		}
	}

	SelectorDialog::SelectorDialog(wxWindow* parent, const wxString& caption)
		:SelectorDisplayModel(true)
	{
		if (KxStdDialog::Create(parent, KxID_NONE, caption, wxDefaultPosition, wxDefaultSize, KxBTN_OK))
		{
			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide(wxBOTH);

			m_RemoveButton = AddButton(KxID_REMOVE, wxEmptyString, true).As<KxButton>();
			m_RemoveButton->Bind(wxEVT_BUTTON, &SelectorDialog::OnRemoveTag, this);
			m_RemoveButton->Disable();

			m_AddButton = AddButton(KxID_ADD, wxEmptyString, true).As<KxButton>();
			m_AddButton->Bind(wxEVT_BUTTON, &SelectorDialog::OnAddTag, this);

			wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
			m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
			m_ViewPane->SetSizer(sizer);
			PostCreate();

			// List
			SelectorDisplayModel::Create(m_ViewPane, sizer);

			AdjustWindow(wxDefaultPosition, wxSize(400, 550));
			GetView()->SetFocus();
		}
	}
	SelectorDialog::~SelectorDialog()
	{
		IncRef();
	}
}
