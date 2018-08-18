#include "stdafx.h"
#include "KPCInfoTagsListModel.h"
#include "PackageCreator/KPackageCreatorController.h"
#include "ModManager/KModManager.h"
#include <KxFramework/KxDataViewComboBox.h>
#include <KxFramework/KxString.h>

bool KPCInfoTagsListModel::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
{
	bool bRet = KModManagerTagSelectorCB::SetValueByRow(data, row, column);
	if (bRet)
	{
		m_Controller->ChangeNotify();
	}
	return bRet;
}

void KPCInfoTagsListModel::Create(KPackageCreatorController* controller, wxWindow* window, wxSizer* pSzier)
{
	m_Controller = controller;
	KModManagerTagSelectorCB::Create(window, pSzier);
}
