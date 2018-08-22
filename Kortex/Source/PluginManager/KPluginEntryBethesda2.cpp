#include "stdafx.h"
#include "KPluginEntryBethesda2.h"
#include "KAux.h"

void KPluginEntryBethesda2::OnPluginRead(const KPluginReader& reader)
{
	KPluginEntryBethesda::OnPluginRead(reader);

	wxString ext = KxString::MakeLower(GetName().AfterLast('.'));
	if (ext == "esm")
	{
		SetMaster(true);
	}
	if (ext == "esl")
	{
		SetMaster(true);
		SetLight(true);
	}
}
