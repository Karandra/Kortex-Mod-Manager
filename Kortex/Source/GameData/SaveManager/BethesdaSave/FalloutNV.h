#pragma once
#include "stdafx.h"
#include "BethesdaBasicSave.h"

namespace Kortex::SaveManager::BethesdaSave
{
	class FalloutNV: public BethesdaBasicSave
	{
		protected:
			virtual bool OnRead(const KxFileItem& fileItem) override;
	};
}
