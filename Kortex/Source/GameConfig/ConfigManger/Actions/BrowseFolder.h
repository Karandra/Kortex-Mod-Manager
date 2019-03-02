#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/IAction.h"

namespace Kortex::GameConfig::Actions
{
	class BrowseFolder: public IAction
	{
		public:
			virtual void Invoke(ItemValue& value) override;
	};
}
