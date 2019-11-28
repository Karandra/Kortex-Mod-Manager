#include "stdafx.h"
#include "TagsListModel.h"
#include "PackageCreator/WorkspaceDocument.h"
#include <Kortex/ModManager.hpp>
#include <KxFramework/KxDataViewComboBox.h>
#include <KxFramework/KxString.h>

namespace Kortex::PackageDesigner::PageInfoNS
{
	bool TagsListModel::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
	{
		bool result = SelectorDisplayModelCB::SetValueByRow(data, row, column);
		if (result)
		{
			m_Controller->ChangeNotify();
		}
		return result;
	}
	void TagsListModel::Create(WorkspaceDocument* controller, wxWindow* window, wxSizer* sizer)
	{
		m_Controller = controller;
		SelectorDisplayModelCB::Create(window, sizer);
	}
}
