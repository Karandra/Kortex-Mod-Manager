#include "stdafx.h"
#include <Kortex/Application.hpp>
#include "InfoDisplayModel.h"
#include "WizardDialog.h"
#include "PackageCreator/KPackageCreatorPageComponents.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/ModTagManager.hpp>
#include "UI/KMainWindow.h"
#include "UI/KTextEditorDialog.h"
#include "Utility/KAux.h"
#include <KxFramework/KxTextBoxDialog.h>
#include <KxFramework/KxTaskDialog.h>

namespace
{
	enum class ColumnID
	{
		Name,
		Value,
	};
}

namespace Kortex::InstallWizard
{
	wxAny InfoDisplayModel::GetValue(const KxDataView2::Node& node, const KxDataView2::Column& column) const
	{
		const Item& item = m_Items[node.GetRow()];
		const KLabeledValue& itemValue = item.Value;

		switch (column.GetID<ColumnID>())
		{
			case ColumnID::Name:
			{
				KxDataView2::BitmapTextValue value;

				// Label
				if (!itemValue.HasLabel())
				{
					value.SetText(KAux::ExtractDomainName(itemValue.GetValue()));
				}
				else
				{
					value.SetText(itemValue.GetLabel());
				}

				// Icon
				if (item.IconID)
				{
					value.SetBitmap(ImageProvider::GetBitmap(item.IconID));
				}
				else
				{
					value.SetReservedBitmapWidth(KBitmapSize().FromSystemSmallIcon().GetWidth());
				}

				return value;
			}
			case ColumnID::Value:
			{
				switch (item.Type)
				{
					case InfoKind::Tags:
					{
						const ModTagStore& tags = m_PackageConfig.GetInfo().GetTagStore();
						return KPackageCreatorPageComponents::FormatArrayToText(tags.GetNames());
					}
					case InfoKind::ID:
					{
						return KxString::Format(wxS("%1 (%2)"), m_PackageConfig.GetModID(), m_PackageConfig.GetSignature());
					}
					case InfoKind::Name:
					{
						return m_PackageConfig.GetModName();
					}
				};
				return itemValue.GetValue();
			}
		};
		return {};
	}
	wxAny InfoDisplayModel::GetEditorValue(const KxDataView2::Node& node, const KxDataView2::Column& column) const
	{
		if (column.GetID<ColumnID>() == ColumnID::Value)
		{
			const Item& item = m_Items[node.GetRow()];
			switch (item.Type)
			{
				case InfoKind::ID:
				{
					return m_PackageConfig.GetModID();
				}
				case InfoKind::Name:
				{
					return m_PackageConfig.GetModName();
				}
			};
		}
		return {};
	}
	bool InfoDisplayModel::SetValue(KxDataView2::Node& node, const wxAny& value, KxDataView2::Column& column)
	{
		if (column.GetID<ColumnID>() == ColumnID::Value)
		{
			const Item& item = m_Items[node.GetRow()];
			switch (item.Type)
			{
				case InfoKind::ID:
				{
					wxString id = value.As<wxString>();
					if (id != m_PackageConfig.GetModID() && CheckModID(id))
					{
						m_PackageConfig.SetModID(id);
						m_InstallWizard.FindExistingMod();
						return true;
					}
					return false;
				}
				case InfoKind::Name:
				{
					m_PackageConfig.GetInfo().SetName(value.As<wxString>());
					return true;
				}
			};
		}
		return false;
	}
	bool InfoDisplayModel::IsEnabled(const KxDataView2::Node& node, const KxDataView2::Column& column) const
	{
		if (column.GetID<ColumnID>() == ColumnID::Value)
		{
			const Item& item = m_Items[node.GetRow()];
			switch (item.Type)
			{
				case InfoKind::ID:
				case InfoKind::Name:
				{
					return true;
				}
			};
		}
		return false;
	}

	bool InfoDisplayModel::CheckModID(const wxString& id)
	{
		if (id.IsEmpty())
		{
			KxTaskDialog msg(GetView(), KxID_NONE, KTr("InstallWizard.ChangeID.Invalid"), wxEmptyString, KxBTN_OK, KxICON_WARNING);
			msg.ShowModal();
			return false;
		}
		return true;
	}
	void InfoDisplayModel::OnActivateItem(KxDataView2::Event& event)
	{
		KxDataView2::Node* node = event.GetNode();
		const Item& item = m_Items[node->GetRow()];
		const KLabeledValue& itemValue = item.Value;

		switch (item.Type)
		{
			case InfoKind::ID:
			case InfoKind::Name:
			{
				if (KxDataView2::Column* column = event.GetColumn())
				{
					node->Edit(*column);
				}
				break;
			}
			case InfoKind::ModSource:
			{
				if (itemValue.HasValue())
				{
					KAux::AskOpenURL(itemValue.GetValue(), GetView());
				}
				break;
			}
			case InfoKind::Tags:
			{
				ModTagStore& tags = m_PackageConfig.GetInfo().GetTagStore();

				ModTagManager::SelectorDialog dialog(GetView(), itemValue.GetLabel());
				dialog.SetDataVector(&tags, m_InstallWizard.GetModEntry());
				dialog.ShowModal();
				if (dialog.IsModified())
				{
					dialog.ApplyChangesToMod();
					node->Refresh();
				}
				break;
			}
			default:
			{
				if (itemValue.HasValue())
				{
					KTextEditorDialog dialog(GetView());
					dialog.SetText(itemValue.GetValue());
					dialog.SetEditable(false);
					dialog.ShowPreview(true);
					dialog.ShowModal();
				}
				break;
			}
		};
	}

	void InfoDisplayModel::CreateView(wxWindow* parent)
	{
		using namespace KxDataView2;

		// View
		View* view = new View(parent, KxID_NONE, CtrlStyle::VerticalRules|CtrlStyle::CellFocus);
		view->AssignModel(this);

		// Columns
		view->AppendColumn<BitmapTextRenderer>(KTr("Generic.Name"), ::ColumnID::Name, 250);
		view->AppendColumn<TextRenderer, TextEditor>(KTr("Generic.Value"), ::ColumnID::Value);

		// Events
		view->Bind(KxDataView2::EVENT_ITEM_ACTIVATED, &InfoDisplayModel::OnActivateItem, this);
	}
	void InfoDisplayModel::AddItem(const KLabeledValue& value, const ResourceID& image, InfoKind type)
	{
		Item& item = m_Items.emplace_back(value);
		item.IconID = image;
		item.Type = type;

		SetItemCount(m_Items.size());
	}
}
