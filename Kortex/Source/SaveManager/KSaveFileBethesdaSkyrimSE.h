#pragma once
#include "stdafx.h"
#include "KSaveFileBethesdaBase.h"

class KSaveFileBethesdaSkyrimSE: public KSaveFileBethesdaBase
{
	protected:
		virtual bool DoInitializeSaveData() override;
};
