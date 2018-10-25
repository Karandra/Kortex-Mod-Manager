#pragma once
#include "stdafx.h"
#include "KSaveFileBethesdaBase.h"

class KSaveFileBethesdaMorrowind: public KSaveFileBethesdaBase
{
	protected:
		virtual bool DoInitializeSaveData() override;
};
