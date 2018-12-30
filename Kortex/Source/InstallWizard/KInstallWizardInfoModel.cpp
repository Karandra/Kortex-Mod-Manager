#include "stdafx.h"
#include <Kortex/Application.hpp>
#include "KInstallWizardInfoModel.h"
#include "KInstallWizardDialog.h"
#include "PackageCreator/KPackageCreatorPageComponents.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/ModTagManager.hpp>
#include "UI/KMainWindow.h"
#include "UI/KTextEditorDialog.h"
#include "KAux.h"
#include <KxFramework/KxTextBoxDialog.h>
#include <KxFramework/KxTaskDialog.h>

using namespace Kortex;
using namespace Kortex::ModManager;

enum ColumnID
{
	Icon,
	Name,
	Value,
};

void KInstallWizardInfoModel::OnInitControl()
{
	/* View */
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KInstallWizardInfoModel::OnActivateItem, this);

	/* Columns */
	GetView()->AppendColumn<KxDataViewBitmapRenderer>(wxEmptyString, ColumnID::Icon, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE, KxDV_COL_NONE);
	GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_INERT, 250);
	GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("Generic.Value"), ColumnID::Value, KxDATAVIEW_CELL_EDITABLE);
}

void KInstallWizardInfoModel::GetEditorValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
{
	const Item& item = m_DataVector[row];
	if (column->GetID() == ColumnID::Value)
	{
		switch (item.Type)
		{
			case KIWI_TYPE_ID:
			{
				data = m_Config.GetModID();
				break;
			}
			case KIWI_TYPE_NAME:
			{
				data = m_Config.GetModName();
				break;
			}
		};
	}
}
void KInstallWizardInfoModel::GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
{
	const Item& item = m_DataVector[row];
	switch (column->GetID())
	{
		case ColumnID::Icon:
		{
			if (item.IconID != KIMG_NONE)
			{
				data = KGetBitmap(item.IconID);
			}
			break;
		}
		case ColumnID::Name:
		{
			if (!item.Value.HasLabel())
			{
				wxString name = KAux::ExtractDomainName(item.Value.GetValue());
				if (!name.IsEmpty())
				{
					data = name;
				}
			}
			else
			{
				data = item.Value.GetLabel();
			}
			break;
		}
		case ColumnID::Value:
		{
			switch (item.Type)
			{
				case KIWI_TYPE_TAGS:
				{
					data = KPackageCreatorPageComponents::FormatArrayToText(GetTagStore().GetNames());
					break;
				}
				case KIWI_TYPE_ID:
				{
					data = KxString::Format("%1 (%2)", m_Config.GetModID(), m_Config.GetSignature());
					break;
				}
				case KIWI_TYPE_NAME:
				{
					data = m_Config.GetModName();
					break;
				}
				default:
				{
					data = item.Value.GetValue();
				}
			};
			break;
		}
	};
}
bool KInstallWizardInfoModel::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
{
	Item& item = m_DataVector[row];
	if (column->GetID() == ColumnID::Value)
	{
		switch (item.Type)
		{
			case KIWI_TYPE_ID:
			{
				wxString id = data.As<wxString>();
				if (id != m_Config.GetModID() && CheckModID(id))
				{
					m_Config.SetModID(id);
					m_InstallWizard.FindExistingMod();
					return true;
				}
				break;
			}
			case KIWI_TYPE_NAME:
			{
				m_Config.GetInfo().SetName(data.As<wxString>());
				return true;
			}
		}
	}
	return false;
}
bool KInstallWizardInfoModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	if (column->GetID() == ColumnID::Value)
	{
		const Item& item = m_DataVector[row];
		switch (item.Type)
		{
			case KIWI_TYPE_ID:
			case KIWI_TYPE_NAME:
			{
				return true;
			}
		};
	}
	return false;
}

bool KInstallWizardInfoModel::CheckModID(const wxString& id)
{
	if (id.IsEmpty())
	{
		KxTaskDialog msg(GetViewTLW(), KxID_NONE, KTr("InstallWizard.ChangeID.Invalid"), wxEmptyString, KxBTN_OK, KxICON_WARNING);
		msg.ShowModal();
		return false;
	}
	return true;
}
Kortex::ModTagStore& KInstallWizardInfoModel::GetTagStore() const
{
	return m_Config.GetInfo().GetTagStore();
}
void KInstallWizardInfoModel::OnActivateItem(KxDataViewEvent& event)
{
	Item* entry = GetDataEntry(event.GetItem());
	if (entry)
	{
		switch (entry->Type)
		{
			case KIWI_TYPE_ID:
			case KIWI_TYPE_NAME:
			{
				if (KxDataViewColumn* column = event.GetColumn())
				{
					GetView()->EditItem(event.GetItem(), column);
				}
				break;
			}
			case KIWI_TYPE_SITE:
			{
				if (entry->Value.HasValue())
				{
					KAux::AskOpenURL(entry->Value.GetValue(), GetViewTLW());
				}
				break;
			}
			case KIWI_TYPE_TAGS:
			{
				ModTagManager::SelectorDialog dialog(GetViewTLW(), entry->Value.GetLabel());
				dialog.SetDataVector(&GetTagStore(), m_InstallWizard.GetModEntry());
				dialog.ShowModal();
				if (dialog.IsModified())
				{
					dialog.ApplyChangesToMod();
					ItemChanged(event.GetItem());
				}
				break;
			}
			default:
			{
				if (entry->Value.HasValue())
				{
					KTextEditorDialog dialog(GetViewTLW());
					dialog.SetText(entry->Value.GetValue());
					dialog.SetEditable(false);
					dialog.ShowPreview(true);
					dialog.ShowModal();
				}
				break;
			}
		};
	}
}

void KInstallWizardInfoModel::AddItem(const KLabeledValue& value, KImageEnum image, KIWITypes type)
{
	Item& item = m_DataVector.emplace_back(value);
	item.IconID = image;
	item.Type = type;
}
