#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/IAction.h"

namespace Kortex::GameConfig::Actions
{
	class BrowseFile: public IAction
	{
		public:
			virtual void Invoke(Item& item, ItemValue& value) override;
	};
}
