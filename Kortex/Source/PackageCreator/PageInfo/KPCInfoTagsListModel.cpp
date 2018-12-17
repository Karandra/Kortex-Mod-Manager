#include "stdafx.h"
#include "KPCInfoTagsListModel.h"
#include "PackageCreator/KPackageCreatorController.h"
#include <Kortex/ModManager.hpp>
#include <KxFramework/KxDataViewComboBox.h>
#include <KxFramework/KxString.h>

bool KPCInfoTagsListModel::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
{
	bool bRet = SelectorDisplayModelCB::SetValueByRow(data, row, column);
	if (bRet)
	{
		m_Controller->ChangeNotify();
	}
	return bRet;
}

void KPCInfoTagsListModel::Create(KPackageCreatorController* controller, wxWindow* window, wxSizer* pSzier)
{
	m_Controller = controller;
	SelectorDisplayModelCB::Create(window, pSzier);
}
