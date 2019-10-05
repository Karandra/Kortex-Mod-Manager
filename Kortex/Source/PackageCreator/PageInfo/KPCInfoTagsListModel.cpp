#include "stdafx.h"
#include "KPCInfoTagsListModel.h"
#include "PackageCreator/KPackageCreatorController.h"
#include <Kortex/ModManager.hpp>
#include <KxFramework/KxDataViewComboBox.h>
#include <KxFramework/KxString.h>

namespace Kortex::PackageDesigner
{
	bool KPCInfoTagsListModel::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
	{
		bool result = SelectorDisplayModelCB::SetValueByRow(data, row, column);
		if (result)
		{
			m_Controller->ChangeNotify();
		}
		return result;
	}
	void KPCInfoTagsListModel::Create(KPackageCreatorController* controller, wxWindow* window, wxSizer* pSzier)
	{
		m_Controller = controller;
		SelectorDisplayModelCB::Create(window, pSzier);
	}
}
