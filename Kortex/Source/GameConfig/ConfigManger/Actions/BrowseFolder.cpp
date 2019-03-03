#include "stdafx.h"
#include "BrowseFolder.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxFileBrowseDialog.h>

namespace Kortex::GameConfig::Actions
{
	void BrowseFolder::Invoke(ItemValue& value)
	{
		KxFileBrowseDialog dialog(IApplication::GetInstance()->GetTopWindow(), KxID_NONE, KxFBD_OPEN_FOLDER);
		dialog.SetFolder(value.As<wxString>());

		if (dialog.ShowModal() == KxID_OK)
		{
			value.Assign(dialog.GetResult());
		}
	}
}
