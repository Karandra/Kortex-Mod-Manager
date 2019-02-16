#include "stdafx.h"
#include "FindFiles.h"
#include <KxFramework/KxFileFinder.h>

namespace Kortex::GameConfig::SamplingFunction
{
	void FindFiles::DoCall(const wxString& sourcePath) const
	{
		KxFileFinder finder(KVarExp(sourcePath));
		for (KxFileItem item = finder.FindNext(); item.IsOK(); item = finder.FindNext())
		{
			if (item.IsFile() && item.IsNormalItem())
			{
				m_Values.emplace_back(item.GetName());
			}
		}
	}
	void FindFiles::OnCall(const ItemValue::Vector& arguments)
	{
		if (arguments.size() >= 1)
		{
			DoCall(arguments[0].As<wxString>());
		}
	}
}
