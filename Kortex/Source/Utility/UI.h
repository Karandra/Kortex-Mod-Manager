#pragma once
#include "stdafx.h"
#include <KxFramework/KxAuiToolBar.h>

namespace Kortex
{
	class ResourceID;
}

namespace Kortex::Utility::UI
{
	KxAuiToolBarItem* CreateToolBarButton(KxAuiToolBar* toolBar,
										  const wxString& label,
										  const ResourceID& imageID = {},
										  wxItemKind kind = wxITEM_NORMAL,
										  int index = -1);
}
