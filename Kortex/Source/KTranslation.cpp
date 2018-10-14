#include "stdafx.h"
#include "KTranslation.h"
#include "KApp.h"
#include "GameInstance/KGameInstance.h"

const KxTranslation& KTranslation::GetTranslation()
{
	return KApp::Get().GetTranslation();
}

wxString V(const wxString& source)
{
	return KApp::Get().ExpandVariables(source);
}
wxString V(KGameInstance* profile, const wxString& source)
{
	return profile->ExpandVariables(source);
}
