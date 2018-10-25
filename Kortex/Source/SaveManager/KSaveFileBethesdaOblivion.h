#pragma once
#include "stdafx.h"
#include "KSaveFileBethesdaBase.h"

class KSaveFileBethesdaOblivion: public KSaveFileBethesdaBase
{
	protected:
		virtual bool DoInitializeSaveData() override;
};
