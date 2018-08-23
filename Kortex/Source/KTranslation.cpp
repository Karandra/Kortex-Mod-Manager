#include "stdafx.h"
#include "KTranslation.h"
#include "KApp.h"
#include "Profile/KProfile.h"

wxString V(const wxString& source)
{
	return KApp::Get().ExpandVariables(source);
}
wxString V(KProfile* profile, const wxString& source)
{
	return profile->ExpandVariables(source);
}
