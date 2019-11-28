#include "stdafx.h"
#include "DisplayModel.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>

namespace
{
	enum class ColumnRef
	{
		Name,
		Version,
		ObjectState,
	};
}

namespace Kortex::InstallWizard::RequirementsPageNS
{
	wxAny DisplayModel::GetValue(const Node& node, const Column& column) const
	{
		const PackageProject::RequirementItem& item = GetItem(node);
		switch (column.GetID<ColumnRef>())
		{
			case ColumnRef::Name:
			{
				wxString label;
				if (item.GetName() != item.GetID() && item.GetID() != IGameInstance::GetActive()->GetGameID())
				{
					label = KxString::Format("%1 (%2)", item.GetName(), item.GetID());
				}
				else
				{
					label = item.GetName();
				}
				return KxDataView2::BitmapTextValue(label, GetIconByState(item.GetOverallStatus()));
			}
			case ColumnRef::Version:
			{
				wxBitmap icon = GetIconByState(item.CheckVersion() ? wxCHK_CHECKED : wxCHK_UNCHECKED);
				const bool currentOK = item.GetCurrentVersion().IsOK();
				const bool requiredOK = item.GetRequiredVersion().IsOK();

				if ((!currentOK && !requiredOK) || (currentOK && !requiredOK))
				{
					return icon;
				}
				else
				{
					wxString cv = item.GetCurrentVersion().ToString();
					wxString rv = item.GetRequiredVersion().ToString();
					wxString operatorSymbol = ModPackageProject::OperatorToSymbolicName(item.GetRVFunction());
					return KxDataView2::BitmapTextValue(KxString::Format("%1 %2 %3", cv, operatorSymbol, rv), icon);
				}
			}
			case ColumnRef::ObjectState:
			{
				const wxString& object = item.GetObject();
				PackageProject::ObjectFunction objectFunc = item.GetObjectFunction();
				wxBitmap icon = GetIconByState(item.GetObjectFunctionResult());
				const bool objFunction = objectFunc == PackageProject::ObjectFunction::FileExist || objectFunc == PackageProject::ObjectFunction::FileNotExist;

				// There's not much sense displaying required state string under this conditions
				if (objFunction && object.IsEmpty())
				{
					return icon;
				}
				else
				{
					wxString label = KTr("PackageCreator.PageRequirements.RequiredState." + PackageProject::RequirementsSection::ObjectFunctionToString(objectFunc));
					if (!object.IsEmpty())
					{
						return KxDataView2::BitmapTextValue(KxString::Format("%1: \"%2\"", label, object), icon);
					}
					else
					{
						return KxDataView2::BitmapTextValue(label, icon);
					}
				}
			}
		};
		return {};
	}
	auto DisplayModel::GetToolTip(const Node& node, const Column& column) const -> ToolTip
	{
		const PackageProject::RequirementItem& item = GetItem(node);
		if (const wxString& text = item.GetDescription(); !text.IsEmpty())
		{
			return ToolTip(GetValue(node, column).As<KxDataView2::BitmapTextValue>().GetText(), text);
		}
		return VirtualListModel::GetToolTip(node, column);
	}
	bool DisplayModel::IsEnabled(const Node& node, const Column& column) const
	{
		return false;
	}

	wxBitmap DisplayModel::GetIconByState(PackageProject::ReqState state) const
	{
		switch (state)
		{
			case PackageProject::ReqState::False:
			{
				return ImageProvider::GetBitmap(ImageResourceID::CrossCircleFrame);
			}
			case PackageProject::ReqState::Unknown:
			{
				return ImageProvider::GetBitmap(ImageResourceID::Exclamation);
			}
		};
		return ImageProvider::GetBitmap(ImageResourceID::TickCircleFrame);
	}

	void DisplayModel::CreateView(wxWindow* parent, bool noBorder)
	{
		// View
		View* view = new View(parent, KxID_NONE, CtrlStyle::VerticalRules|CtrlStyle::CellFocus|CtrlStyle::FitLastColumn);
		view->AssignModel(this);
		if (noBorder)
		{
			view->ToggleWindowStyle(wxBORDER_NONE);
		}

		// Columns
		using namespace KxDataView2;

		view->AppendColumn<BitmapTextRenderer>(KTr("Generic.Name"), ColumnRef::Name);
		{
			wxString title = KxString::Format("%1/%2", KTr("PackageCreator.PageRequirements.CurrentVersion"), KTr("PackageCreator.PageRequirements.RequiredVersion"));
			view->AppendColumn<BitmapTextRenderer>(title, ColumnRef::Version);
		}
		view->AppendColumn<BitmapTextRenderer>(KTr("PackageCreator.PageRequirements.RequiredState"), ColumnRef::ObjectState);
	}

	void DisplayModel::ShowGroups(const KxStringVector& groupIDs)
	{
		m_Items.clear();
		for (const wxString& id: groupIDs)
		{
			PackageProject::RequirementGroup* group = m_Page.GetPackageConfig().GetRequirements().FindGroupWithID(id);
			if (group)
			{
				for (const auto& entry: group->GetEntries())
				{
					// No need to show requirements with no object function and no required version.
					// They always be true.
					if (entry->GetObjectFunction() == PackageProject::ObjectFunction::None && !entry->GetRequiredVersion().IsOK())
					{
						break;
					}
					m_Items.push_back(entry.get());
				}
			}
		}
		SetItemCount(m_Items.size());
	}
	void DisplayModel::ClearDisplay()
	{
		m_Items.clear();
		SetItemCount(m_Items.size());
	}
}
