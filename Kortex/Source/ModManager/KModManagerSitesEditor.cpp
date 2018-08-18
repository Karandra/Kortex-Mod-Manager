#include "stdafx.h"
#include "KModManagerSitesEditor.h"
#include "KModManager.h"
#include "Network/KNetwork.h"
#include "KAux.h"
#include "KApp.h"
#include <KxFramework/KxButton.h>
#include <KxFramework/KxString.h>

enum ColumnID
{
	Name,
	Value
};

void KModManagerSitesEditor::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KModManagerSitesEditor::OnActivate, this);

	// Columns
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer, KxDataViewTextEditor>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 150);
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(T("Generic.Value"), ColumnID::Value, KxDATAVIEW_CELL_EDITABLE);
}

bool KModManagerSitesEditor::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	const KMMSitesEditorNode& node = GetNode(row);
	if (node.IsFixed())
	{
		return column->GetID() == ColumnID::Value;
	}
	else if (node.IsNormal())
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			case ColumnID::Value:
			{
				return true;
			}
		};
	}
	return false;
}
void KModManagerSitesEditor::GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KMMSitesEditorNode& node = GetNode(row);
	if (node.IsFixed())
	{
		switch (column->GetID())
		{
			case ColumnID::Value:
			{
				value = node.HasFixedSiteModID() ? wxString::Format("%lld", node.GetFixedSiteModID()) : wxEmptyString;
				return;
			}
		};
	}
	GetValueByRow(value, row, column);
}
void KModManagerSitesEditor::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	const KMMSitesEditorNode& node = GetNode(row);
	if (node.IsFixed())
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				KNetworkProvider* site = KNetwork::GetInstance()->GetProvider(node.GetFixedSiteIndex());
				value = KxDataViewBitmapTextValue(site->GetName(), KGetBitmap(site->GetIcon()));
				break;
			}
			case ColumnID::Value:
			{
				if (node.HasFixedSiteModID())
				{
					value = KModEntry::GetWebSite(m_FixedSites, node.GetFixedSiteIndex()).GetValue();
				}
				else
				{
					value = wxEmptyString;
				}
				break;
			}
		};
	}
	else if (node.IsNormal())
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				wxString label;
				if (node.GetNormalSite().HasLabel())
				{
					label = node.GetNormalSite().GetLabel();
				}
				else
				{
					label = KAux::ExtractDomainName(node.GetNormalSite().GetValue());
				}

				value = KxDataViewBitmapTextValue(label, KGetBitmap(KNetworkProvider::GetGenericIcon()));
				break;
			}
			case ColumnID::Value:
			{
				value = node.GetNormalSite().GetValue();
				break;
			}
		};
	}
}
bool KModManagerSitesEditor::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
{
	KMMSitesEditorNode& node = GetNode(row);
	if (node.IsFixed())
	{
		switch (column->GetID())
		{
			case ColumnID::Value:
			{
				int64_t oldValue = m_FixedSites[node.GetFixedSiteIndex()];
				int64_t newValue = KNETWORK_SITE_INVALID_MODID;
				data.GetAs(&newValue);

				m_FixedSites[node.GetFixedSiteIndex()] = newValue;
				if (oldValue != m_FixedSites[node.GetFixedSiteIndex()])
				{
					node.SetFixedSiteModID(newValue);
					m_IsModified = true;
					return true;
				}
				break;
			}
		};
	}
	else if (node.IsNormal())
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				wxString name = data.As<wxString>();

				if (name != node.GetNormalSite().GetLabel())
				{
					node.GetNormalSite().SetLabel(name);
					m_IsModified = true;
					return true;
				}
				return false;
			}
			case ColumnID::Value:
			{
				wxString sNewValue = data.As<wxString>();
				if (sNewValue != node.GetNormalSite().GetValue())
				{
					node.GetNormalSite().SetValue(sNewValue);
					m_IsModified = true;
					return true;
				}
				return false;
			}
		};
	}
	return false;
}

void KModManagerSitesEditor::OnActivate(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	if (item.IsOK())
	{
		const KMMSitesEditorNode& node = GetNode(GetRow(item));
		KxDataViewColumn* column = node.IsNormal() ? event.GetColumn() : GetView()->GetColumnByID(ColumnID::Value);
		if (column)
		{
			GetView()->EditItem(item, column);
		}
	}
}

void KModManagerSitesEditor::RefreshItems()
{
	m_DataVector.clear();
	m_DataVector.reserve(KNETWORK_PROVIDER_ID_MAX + m_Sites.size());

	for (int i = 0; i < KNETWORK_PROVIDER_ID_MAX; i++)
	{
		// Using 'AddItem' here is fine as it don't modifies original container
		AddItem((KNetworkProviderID)i, m_FixedSites[i]);
		//ItemAdded(KxDataViewItem(), MakeItem(AddItem((KNetworkProviderID)i, m_FixedSites[i])));
	}
	for (auto& v: m_Sites)
	{
		// Can't use 'AddItem' here
		m_DataVector.emplace_back(v);
	}
	ItemsCleared();
}
bool KModManagerSitesEditor::RemoveItem(KMMSitesEditorNode& node)
{
	if (node.IsNormal())
	{
		auto it = std::remove_if(m_Sites.begin(), m_Sites.end(), [&node](const KLabeledValue& v)
		{
			return &v == &node.GetNormalSite();
		});
		if (it != m_Sites.end())
		{
			m_Sites.erase(it, m_Sites.end());
			ItemDeleted(GetItem(std::distance(m_Sites.begin(), it)));
			m_IsModified = true;
			return true;
		}
	}
	else if (node.IsFixed())
	{
		node.SetFixedSiteModID(KNETWORK_SITE_INVALID_MODID);
		m_IsModified = true;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void KModManagerSitesEditorDialog::OnSelectItem(KxDataViewEvent& event)
{
	if (event.GetItem().IsOK())
	{
		const KMMSitesEditorNode& node = GetNode(event.GetItem());
		m_RemoveButton->Enable(node.IsNormal() || (node.IsFixed() && node.HasFixedSiteModID()));
	}
	else
	{
		m_RemoveButton->Disable();
	}
}
void KModManagerSitesEditorDialog::OnAddTag(wxCommandEvent& event)
{
	KMMSitesEditorNode& node = AddItem(KLabeledValue(wxEmptyString));
	RefreshItems();

	KxDataViewItem tNewItem = GetItem(GetItemCount() - 1);
	SelectItem(tNewItem);
	GetView()->EditItem(tNewItem, GetView()->GetColumn(ColumnID::Name));
}
void KModManagerSitesEditorDialog::OnRemoveTag(wxCommandEvent& event)
{
	KxDataViewItem item = GetView()->GetSelection();
	KMMSitesEditorNode& node = GetNode(item);

	if (RemoveItem(node) && node.IsNormal())
	{
		SelectItem(GetItemCount());
		ItemChanged(item);
	}
	else
	{
		SelectItem(GetRow(item));
	}
	GetView()->SetFocus();
}

KModManagerSitesEditorDialog::KModManagerSitesEditorDialog(wxWindow* parent, KLabeledValueArray& sites, KModEntry::FixedWebSitesArray& fixedSites)
	:KModManagerSitesEditor(sites, fixedSites)
{
	if (KxStdDialog::Create(parent, KxID_NONE, T("ModManager.SitesEditor"), wxDefaultPosition, wxDefaultSize, KxBTN_OK))
	{
		SetMainIcon(KxICON_NONE);
		SetWindowResizeSide(wxBOTH);

		m_RemoveButton = AddButton(KxID_REMOVE, wxEmptyString, true).As<KxButton>();
		m_RemoveButton->Bind(wxEVT_BUTTON, &KModManagerSitesEditorDialog::OnRemoveTag, this);
		m_RemoveButton->Disable();

		m_AddButton = AddButton(KxID_ADD, wxEmptyString, true).As<KxButton>();
		m_AddButton->Bind(wxEVT_BUTTON, &KModManagerSitesEditorDialog::OnAddTag, this);

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
		m_ViewPane->SetSizer(sizer);
		PostCreate();

		// List
		KModManagerSitesEditor::Create(m_ViewPane, sizer);
		RefreshItems();

		AdjustWindow(wxDefaultPosition, wxSize(500, 350));
		GetView()->SetFocus();
	}
}
KModManagerSitesEditorDialog::~KModManagerSitesEditorDialog()
{
	IncRef();
}
