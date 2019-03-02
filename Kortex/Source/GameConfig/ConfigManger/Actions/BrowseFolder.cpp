#include "stdafx.h"
#include "BrowseFolder.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxFileBrowseDialog.h>

namespace Kortex::GameConfig::Actions
{
	void BrowseFolder::Invoke(ItemValue& value)
	{
		KxFileBrowseDialog dialog(IApplication::GetInstance()->GetTopWindow(), KxID_NONE, KxFBD_OPEN_FOLDER);
		if (dialog.ShowModal() == KxID_OK)
		{
			dialog.SetFolder(value.As<wxString>());
			value.Assign(dialog.GetResult());
		}
	}
}
