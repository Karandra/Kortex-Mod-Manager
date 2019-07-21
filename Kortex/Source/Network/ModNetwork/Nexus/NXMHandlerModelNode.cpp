#include "stdafx.h"
#include "NXMHandlerModel.h"
#include "NXMHandlerModelNode.h"
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>

namespace Kortex::NetworkManager
{
	wxAny NXMHandlerModelNode::GetValue(const KxDataView2::Column& column) const
	{
		using namespace KxDataView2;
		using ColumnID = NXMHandlerModel::ColumnID;

		switch (column.GetID<ColumnID>())
		{
			case ColumnID::NexusID:
			{
				return m_Instance.GetVariables().GetVariable(wxS("NexusDomainName")).AsString();
			}
			case ColumnID::Game:
			{
				return m_Instance.GetGameName();
			}
			case ColumnID::Target:
			{
				return {};
			}
		};
		return {};
	}
	wxAny NXMHandlerModelNode::GetEditorValue(const KxDataView2::Column& column) const
	{
		using namespace KxDataView2;
		using ColumnID = NXMHandlerModel::ColumnID;

		switch (column.GetID<ColumnID>())
		{
			case ColumnID::Target:
			{
				return 0;
			}
		};
		return {};
	}
	bool NXMHandlerModelNode::SetValue(KxDataView2::Column& column, const wxAny& value)
	{
		return false;
	}

	bool NXMHandlerModelNode::IsEnabled(const KxDataView2::Column& column) const
	{
		using ColumnID = NXMHandlerModel::ColumnID;
		return column.GetID<ColumnID>() == ColumnID::Target;
	}
}
