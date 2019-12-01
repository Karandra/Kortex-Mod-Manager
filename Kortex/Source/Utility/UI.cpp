#include "stdafx.h"
#include "UI.h"
#include "Application/Resources/IImageProvider.h"

namespace Kortex::Utility::UI
{
	KxAuiToolBarItem* CreateToolBarButton(KxAuiToolBar* toolBar,
										  const wxString& label,
										  const ResourceID& imageID,
										  wxItemKind kind,
										  int index
	)
	{
		wxBitmap bitmap = ImageProvider::GetBitmap(imageID);
		KxAuiToolBarItem* button = toolBar->AddTool(label, bitmap, kind);
		if (!toolBar->HasFlag(wxAUI_TB_TEXT))
		{
			button->SetShortHelp(label);
		}

		return button;
	}
}
