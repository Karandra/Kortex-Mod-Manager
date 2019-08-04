#include "stdafx.h"
#include "SelectorDialog.h"
#include <Kortex/ModTagManager.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <KxFramework/KxDataViewComboBox.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxTaskDialog.h>

namespace Kortex::Application::OName
{
	KortexDefOption(SelectorDialog);
}

namespace
{
	using namespace Kortex;
	using namespace Kortex::Application;

	auto GetOptions()
	{
		return GetAInstanceOptionOf<IModTagManager>(OName::SelectorDialog);
	}
}

namespace Kortex::ModTagManager
{
	void SelectorDialog::OnSelectItem(KxDataViewEvent& event)
	{
		const IModTag* tag = GetDataEntry(GetRow(event.GetItem()));
		m_RemoveButton->Enable(tag != nullptr);

		event.Skip();
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
		KxTaskDialog dialog(this, KxID_NONE, KTr("TagManager.RemoveTag.Message"), {}, KxBTN_YES|KxBTN_NO, KxICON_WARNING);
		if (dialog.ShowModal() == KxID_YES)
		{
			KxDataViewItem item = GetView()->GetSelection();
			IModTag* tag = GetDataEntry(GetRow(item));
			if (tag)
			{
				// Find all mods with this tag and remove it from them
				for (IGameMod* mod: IModManager::GetInstance()->GetMods())
				{
					ModTagStore& tagStore = mod->GetTagStore();
					if (tagStore.HasTag(*tag))
					{
						if (mod->GetPriorityGroupTag() == tag->GetID())
						{
							mod->SetPriorityGroupTag(wxString());
						}

						tagStore.RemoveTag(*tag);
						mod->Save();
						m_IsModified = true;
					}
				}

				// Remove tag from system
				IModTagManager::GetInstance()->RemoveTag(*tag);

				// Reload control
				KxDataViewItem prevItem = GetPrevItem(item);
				RefreshItems();
				SelectItem(prevItem);
				GetView()->SetFocus();
			}
		}
	}
	void SelectorDialog::OnLoadDefaultTags(wxCommandEvent& event)
	{
		KxTaskDialog dialog(this, KxID_NONE, KTr("TagManager.LoadDefaultTags.Message"), {}, KxBTN_YES|KxBTN_NO, KxICON_QUESTION);
		if (dialog.ShowModal() == KxID_YES)
		{
			IModTagManager::GetInstance()->LoadDefaultTags();
			m_IsModified = true;
			RefreshItems();
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

			m_LoadDefaultTagsButton = AddButton(KxID_DEFAULT, KTr("TagManager.LoadDefaultTags"), true).As<KxButton>();
			m_LoadDefaultTagsButton->Bind(wxEVT_BUTTON, &SelectorDialog::OnLoadDefaultTags, this);

			wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
			m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
			m_ViewPane->SetSizer(sizer);
			PostCreate();

			// List
			SelectorDisplayModel::Create(m_ViewPane, sizer);
			GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &SelectorDialog::OnSelectItem, this);
			GetOptions().LoadDataViewLayout(GetView());

			AdjustWindow(wxDefaultPosition, wxSize(980, 720));
			GetView()->SetFocus();
		}
	}
	SelectorDialog::~SelectorDialog()
	{
		GetOptions().SaveDataViewLayout(GetView());
		IncRef();
	}
}
