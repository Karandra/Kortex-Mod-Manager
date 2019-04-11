#include "stdafx.h"
#include "ProfileEditor.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>
#include <Kortex/Events.hpp>
#include "Utility/KAux.h"
#include <KxFramework/KxButton.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxCheckBox.h>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxTaskDialog.h>

namespace
{
	enum ColumnID
	{
		Name,
		LocalSaves,
		LocalConfig,
	};
}

using namespace Kortex::GameInstance;

namespace Kortex::ProfileEditor
{
	void DisplayModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &DisplayModel::OnActivate, this);

		// Columns
		{
			auto info = GetView()->AppendColumn<KxDataViewBitmapTextToggleRenderer>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_ACTIVATABLE, 250);
			info.GetRenderer()->SetDefaultToggleType(KxDataViewToggleRenderer::ToggleType::RadioBox);
		}
		GetView()->AppendColumn<KxDataViewToggleRenderer>(KTr("ModManager.Profile.LocalSaves"), ColumnID::LocalSaves, KxDATAVIEW_CELL_ACTIVATABLE);
		GetView()->AppendColumn<KxDataViewToggleRenderer>(KTr("ModManager.Profile.LocalConfig"), ColumnID::LocalConfig, KxDATAVIEW_CELL_ACTIVATABLE);
	}

	void DisplayModel::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		GetValueByRow(value, row, column);
	}
	void DisplayModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		const IGameProfile* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					bool isCurrent = KxComparator::IsEqual(entry->GetID(), m_NewCurrentProfile, true);
					value = KxDataViewBitmapTextToggleValue(isCurrent, entry->GetID(), wxNullBitmap, KxDataViewBitmapTextToggleValue::RadioBox);
					return;
				}
				case ColumnID::LocalSaves:
				{
					value = entry->IsLocalSavesEnabled();
					return;
				}
				case ColumnID::LocalConfig:
				{
					value = entry->IsLocalConfigEnabled();
					return;
				}
			};
		}
	}
	bool DisplayModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
	{
		IGameProfile* entry = GetDataEntry(row);
		if (entry)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					if (value.CheckType<wxString>())
					{
						wxString name = value.As<wxString>();
						if (!name.IsEmpty() && name != entry->GetID())
						{
							if (IGameInstance::GetActive()->HasProfile(name))
							{
								KxTaskDialog(GetViewTLW(), KxID_NONE, KTr(KxID_RENAME), KTr("ModManager.Profile.AlreadyExist"), KxBTN_OK, KxICON_WARNING).ShowModal();
								return false;
							}

							bool isCurrent = entry->IsActive();
							if (IGameInstance::GetActive()->RenameProfile(*entry, name))
							{
								if (!isCurrent)
								{
									SetNewProfile(entry->GetID());
									MarkModified();
								}
								ProfileEvent(Events::ProfileChanged, *entry).Send();
								return true;
							}
							else
							{
								KxTaskDialog(GetViewTLW(), KxID_NONE, KTr(KxID_RENAME), KTr("ModManager.Profile.RenameFailed"), KxBTN_OK, KxICON_ERROR).ShowModal();
							}
						}
					}
					else
					{
						m_NewCurrentProfile = entry->GetID();
						MarkModified();
						return true;
					}
					return false;
				}
				case ColumnID::LocalSaves:
				{
					entry->SetLocalSavesEnabled(value.As<bool>());
					ProfileEvent(Events::ProfileChanged, *entry).Send();
					return true;
				}
				case ColumnID::LocalConfig:
				{
					entry->SetLocalConfigEnabled(value.As<bool>());
					ProfileEvent(Events::ProfileChanged, *entry).Send();
					return true;
				}
			};
		}
		return false;
	}
	bool DisplayModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
	{
		return true;
	}

	void DisplayModel::OnActivate(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		if (item.IsOK())
		{
			GetView()->EditItem(item, GetView()->GetColumnByID(ColumnID::Name));
		}
	}

	DisplayModel::DisplayModel()
		:m_NewCurrentProfile(IGameInstance::GetActive()->GetActiveProfileID())
	{
	}
}

namespace Kortex::ProfileEditor
{
	void Dialog::OnSelectProfile(KxDataViewEvent& event)
	{
		const IGameProfile* profile = GetDataEntry(GetRow(event.GetItem()));
		if (profile)
		{
			m_CopyButton->Enable();
			m_RemoveButton->Enable(!profile->IsActive());
		}
		else
		{
			m_CopyButton->Disable();
			m_RemoveButton->Disable();
		}
	}
	void Dialog::OnAddProfile(wxCommandEvent& event)
	{
		ProfileEvent listEvent(Events::ProfileAdding);
		listEvent.Send();
		if (!listEvent.IsAllowed())
		{
			return;
		}

		IGameProfile* newProfile = IGameInstance::GetActive()->CreateProfile(listEvent.GetProfileID());
		if (newProfile)
		{
			ProfileEvent(Events::ProfileAdded, *newProfile).Send();

			MarkModified();
			RefreshItems();
			m_RemoveButton->Enable(!IsEmpty());

			KxDataViewItem newItem = GetItem(GetItemCount() - 1);
			SelectItem(newItem);
			GetView()->EditItem(newItem, GetView()->GetColumn(ColumnID::Name));
		}
	}
	void Dialog::OnCopyProfile(wxCommandEvent& event)
	{
		KxDataViewItem item = GetView()->GetSelection();
		if (IGameProfile* profile = GetDataEntry(GetRow(item)))
		{
			ProfileEvent listEvent(Events::ProfileAdding, *profile);
			listEvent.Send();
			if (!listEvent.IsAllowed())
			{
				return;
			}

			IGameProfile* newProfile = IGameInstance::GetActive()->ShallowCopyProfile(*profile, listEvent.GetProfileID());
			if (newProfile)
			{
				ProfileEvent(Events::ProfileAdded, *newProfile).Send();

				MarkModified();
				RefreshItems();

				KxDataViewItem newItem = GetItem(GetItemCount() - 1);
				SelectItem(newItem);
				GetView()->EditItem(newItem, GetView()->GetColumn(ColumnID::Name));
			}
		}
	}
	void Dialog::OnRemoveProfile(wxCommandEvent& event)
	{
		if (IGameInstance::GetActive()->HasProfiles())
		{
			KxDataViewItem item = GetView()->GetSelection();
			if (IGameProfile* profile = GetDataEntry(GetRow(item)))
			{
				ProfileEvent listEvent(Events::ProfileRemoving, *profile);
				listEvent.Send();
				if (!listEvent.IsAllowed())
				{
					return;
				}

				KxTaskDialog dialog(GetView(), KxID_NONE, KTr(KxID_REMOVE), KTr("ModManager.Profile.RemoveDialog"), KxBTN_YES|KxBTN_NO, KxICON_WARNING);
				if (dialog.ShowModal() == KxID_YES)
				{
					const wxString profileID = profile->GetID();
					if (IGameInstance::GetActive()->RemoveProfile(*profile))
					{
						SetNewProfile(IGameInstance::GetActive()->GetActiveProfileID());
						ProfileEvent(Events::ProfileRemoved, profileID).Send();

						size_t index = GetRow(item);
						MarkModified();
						RefreshItems();
						SelectItem(GetItem(index < GetItemCount() ? index : GetItemCount() - 1));

						GetView()->SetFocus();
						m_RemoveButton->Enable(GetItemCount() != 0);
					}
				}
			}
		}
	}

	Dialog::Dialog(wxWindow* parent)
	{
		if (KxStdDialog::Create(parent, KxID_NONE, KTr("ModManager.Profile.Configure"), wxDefaultPosition, wxDefaultSize, KxBTN_OK))
		{
			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide(wxBOTH);

			m_RemoveButton = AddButton(KxID_REMOVE, wxEmptyString, true).As<KxButton>();
			m_RemoveButton->Bind(wxEVT_BUTTON, &Dialog::OnRemoveProfile, this);
			m_RemoveButton->Disable();

			m_AddButton = AddButton(KxID_ADD, wxEmptyString, true).As<KxButton>();
			m_AddButton->Bind(wxEVT_BUTTON, &Dialog::OnAddProfile, this);

			m_CopyButton = AddButton(KxID_COPY, wxEmptyString, true).As<KxButton>();
			m_CopyButton->Bind(wxEVT_BUTTON, &Dialog::OnCopyProfile, this);
			m_CopyButton->Disable();

			wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
			m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
			m_ViewPane->SetSizer(sizer);
			PostCreate();

			// List
			DisplayModel::Create(m_ViewPane, sizer);
			GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &Dialog::OnSelectProfile, this);
			SetDataVector(&IGameInstance::GetActive()->GetProfiles());
			RefreshItems();

			AdjustWindow(wxDefaultPosition, wxSize(500, 375));
			GetView()->SetFocus();
		}
	}
	Dialog::~Dialog()
	{
		IncRef();
	}
}
