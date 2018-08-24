#include "stdafx.h"
#include "KTranslation.h"
#include "KApp.h"
#include "Profile/KProfile.h"

const KxTranslation& KTranslation::GetTranslation()
{
	return KApp::Get().GetTranslation();
}

wxString V(const wxString& source)
{
	return KApp::Get().ExpandVariables(source);
}
wxString V(KProfile* profile, const wxString& source)
{
	return profile->ExpandVariables(source);
}
