#include "stdafx.h"
#include "IAction.h"
#include "Actions/BrowseFile.h"
#include "Actions/BrowseFolder.h"
#include "Actions/PickColor.h"

namespace Kortex::GameConfig
{
	bool IAction::InvokeAction(ActionID id, Item& item, ItemValue& value)
	{
		switch (id)
		{
			case ActionID::BrowseFile:
			{
				Actions::BrowseFile().Invoke(item, value);
				break;
			}
			case ActionID::BrowseFolder:
			{
				Actions::BrowseFolder().Invoke(item, value);
				break;
			}
			case ActionID::PickColor:
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
