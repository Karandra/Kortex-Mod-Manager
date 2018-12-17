#pragma once
#include "stdafx.h"
#include "BaseSave.h"

namespace Kortex::SaveManager::BethesdaSave
{
	class Oblivion: public BaseSave
	{
		protected:
			virtual bool OnRead(const KxFileItem& fileItem) override;
	};
}
