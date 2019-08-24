#include "stdafx.h"
#include "SelectorDisplayModelCB.h"
#include <Kortex/ModTagManager.hpp>
#include <KxFramework/KxDataViewComboBox.h>

namespace Kortex::ModTagManager
{
	KxDataViewCtrl* SelectorDisplayModelCB::OnCreateDataView(wxWindow* window)
	{
		m_ComboView = new KxDataViewComboBox();
		m_ComboView->SetDataViewFlags(KxDV_NO_HEADER);
		m_ComboView->SetOptionEnabled(KxDVCB_OPTION_ALT_POPUP_WINDOW);
		m_ComboView->SetOptionEnabled(KxDVCB_OPTION_FORCE_GET_STRING_VALUE_ON_DISMISS);
		m_ComboView->Create(window, KxID_NONE);

		m_ComboView->Bind(KxEVT_DVCB_GET_STRING_VALUE, &SelectorDisplayModelCB::OnGetStringValue, this);
		return m_ComboView;
	}
	wxWindow* SelectorDisplayModelCB::OnGetDataViewWindow()
	{
		return m_ComboView->GetComboControl();
	}

	wxString SelectorDisplayModelCB::DoGetStingValue() const
	{
		if (!m_TagStore->IsEmpty())
		{
			return KxString::Join(m_TagStore->GetNames(), ", ");
		}
		else
		{
			return KVarExp(wxS("<$T(ID_NONE)>"));
		}
	}
	void SelectorDisplayModelCB::SetStringValue(const wxString& value)
	{
		m_ComboView->GetComboControl()->SetText(value);
	}
	void SelectorDisplayModelCB::OnGetStringValue(KxDataViewEvent& event)
	{
		event.SetString(DoGetStingValue());
	}
	bool SelectorDisplayModelCB::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
	{
		bool bRet = SelectorDisplayModel::SetValueByRow(data, row, column);
		if (bRet)
		{
			SetStringValue(DoGetStingValue());
		}
		return bRet;
	}
	bool SelectorDisplayModelCB::IsEditorEnabledByRow(size_t row, const KxDataViewColumn* column) const
	{
		return false;
	}
}
