#pragma once
#include "stdafx.h"
#include "LabeledValue.h"
class KxAuiToolBar;
class KxURI;

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

	// Shows a dialog that asks user to confirm opening a URI in default browser. Returns true if the user has agreed.
	bool AskOpenURL(const KxURI& uri, wxWindow* parent = nullptr);
	bool AskOpenURL(const LabeledValue::Vector& urlList, wxWindow* parent = nullptr);

	// Creates placeholder for KxHTMLWindow to be showed when actual content is unavailable.
	// Window is required if you want correct text color.
	wxString MakeHTMLWindowPlaceholder(const wxString& text, const wxWindow* window = nullptr);

	bool SetSearchMask(wxString& storage, const wxString& newMask);
	bool CheckSearchMask(const wxString& mask, const wxString& string);
}
