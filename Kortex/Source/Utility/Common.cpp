#include "stdafx.h"
#include "Common.h"
#include <KxFramework/KxCallAtScopeExit.h>
#include <wx/clipbrd.h>

namespace Kortex::Utility
{
	bool CopyTextToClipboard(const wxString& text)
	{
		if (wxTheClipboard->Open())
		{
			KxCallAtScopeExit atExit([]()
			{
				wxTheClipboard->Close();
			});
			return wxTheClipboard->SetData(new wxTextDataObject(text));
		}
		return false;
	}
}
