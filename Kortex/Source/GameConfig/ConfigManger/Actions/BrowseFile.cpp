#include "stdafx.h"
#include "BrowseFile.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxFileBrowseDialog.h>

namespace Kortex::GameConfig::Actions
{
	void BrowseFile::Invoke(Item& item, ItemValue& value)
	{
		KxFileBrowseDialog dialog(item.GetInvokingTopLevelWindow(), KxID_NONE, KxFBD_OPEN);

		wxString folder = value.As<wxString>().BeforeLast(wxS('\\'));
		if (folder.IsEmpty())
		{
			folder = value.As<wxString>();
		}
		dialog.SetFolder(folder);

		if (dialog.ShowModal() == KxID_OK)
		{
			value.Assign(dialog.GetResult());
		}
	}
}
