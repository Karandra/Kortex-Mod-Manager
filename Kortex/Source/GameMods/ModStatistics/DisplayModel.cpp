#include "stdafx.h"
#include "DisplayModel.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxProgressDialog.h>

namespace
{
	enum ColumnID
	{
		Name,
		Value
	};
}

namespace Kortex::ModStatistics
{
	void DisplayModel::OnInitControl()
	{
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_INERT, 250);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Value"), ColumnID::Value, KxDATAVIEW_CELL_INERT, KxCOL_WIDTH_AUTOSIZE);
	}

	void DisplayModel::GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				data = GetStatName(row);
				break;
			}
			case ColumnID::Value:
			{
				data = GetStatValue(row);
				break;
			}
		};
	}
	bool DisplayModel::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
	{
		return false;
	}

	wxString DisplayModel::GetStatName(size_t index) const
	{
		return m_Stats->GetStatName(index);
	}
	const wxString& DisplayModel::GetStatValue(size_t index) const
	{
		return index < m_DataVector.size() ? m_DataVector[index] : KxNullWxString;
	}

	DisplayModel::DisplayModel():m_Stats(IModStatistics::GetInstance())
	{
		m_DataVector.resize(m_Stats->GetStatCount());
		SetDataVector(&m_DataVector);
	}

	void DisplayModel::RefreshItems()
	{
		for (size_t i = 0; i < m_Stats->GetStatCount(); i++)
		{
			m_DataVector[i] = m_Stats->GetStatValue(i);
		}
		KxDataViewVectorListModelEx::RefreshItems();
	}
}
