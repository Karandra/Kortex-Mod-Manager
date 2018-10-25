#pragma once
#include "stdafx.h"
#include "KSaveFileBethesdaBase.h"

class KSaveFileBethesdaSkyrim: public KSaveFileBethesdaBase
{
	protected:
		virtual bool DoInitializeSaveData() override;
};
