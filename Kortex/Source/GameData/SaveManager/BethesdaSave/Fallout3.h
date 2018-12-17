#pragma once
#include "stdafx.h"
#include "BaseSave.h"

namespace Kortex::SaveManager::BethesdaSave
{
	class Fallout3: public BaseSave
	{
		protected:
			virtual bool OnRead(const KxFileItem& fileItem) override;
	};
}
