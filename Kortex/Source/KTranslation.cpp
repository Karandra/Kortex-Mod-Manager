#include "stdafx.h"
#include "KTranslation.h"
#include "KApp.h"
#include "GameInstance/KGameInstance.h"

const KxTranslation& KTranslation::GetAppTranslation()
{
	return KApp::Get().GetTranslation();
}

//////////////////////////////////////////////////////////////////////////
wxString KVarExp(const wxString& source)
{
	return KApp::Get().ExpandVariables(source);
}
wxString KVarExp(const KGameInstance* instance, const wxString& source)
{
	return instance->ExpandVariables(source);
}
