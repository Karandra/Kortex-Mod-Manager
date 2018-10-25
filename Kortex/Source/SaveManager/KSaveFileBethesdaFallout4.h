#pragma once
#include "stdafx.h"
#include "KSaveFileBethesdaBase.h"

class KSaveFileBethesdaFallout4: public KSaveFileBethesdaBase
{
	protected:
		virtual bool DoInitializeSaveData() override;
};
