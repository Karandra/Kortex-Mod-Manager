#include "stdafx.h"
#include "IAction.h"
#include "Actions/BrowseFile.h"
#include "Actions/BrowseFolder.h"
#include "Actions/PickColor.h"

namespace Kortex::GameConfig
{
	bool IAction::InvokeIntrinsicAction(IntrinsicActionID id, Item& item, ItemValue& value)
	{
		switch (id)
		{
			case IntrinsicActionID::BrowseFile:
			{
				Actions::BrowseFile().Invoke(item, value);
				break;
			}
			case IntrinsicActionID::BrowseFolder:
			{
				Actions::BrowseFolder().Invoke(item, value);
				break;
			}
			case IntrinsicActionID::PickColor:
			{
				Actions::PickColor().Invoke(item, value);
				break;
			}
			default:
			{
				return false;
			}
		};
		return true;
	}
}
