#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/IAction.h"

namespace Kortex::GameConfig::Actions
{
	class PickColor: public IAction
	{
		public:
			virtual void Invoke(Item& item, ItemValue& value) override;
	};
}
