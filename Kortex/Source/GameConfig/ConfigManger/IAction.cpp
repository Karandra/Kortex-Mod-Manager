#include "stdafx.h"
#include "IAction.h"
#include "Actions/BrowseFile.h"
#include "Actions/BrowseFolder.h"
#include "Actions/PickColor.h"

namespace Kortex::GameConfig
{
	bool IAction::InvokeAction(ActionID id, ItemValue& value)
	{
		switch (id)
		{
			case ActionID::BrowseFile:
			{
				Actions::BrowseFile().Invoke(value);
				break;
			}
			case ActionID::BrowseFolder:
			{
				Actions::BrowseFolder().Invoke(value);
				break;
			}
			case ActionID::PickColor:
			{
				Actions::PickColor().Invoke(value);
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
