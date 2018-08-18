#include "stdafx.h"
#include "KPackageProjectSerializer.h"
#include "ModManager/KModManager.h"
#include "Network/KNetwork.h"
#include "KAux.h"
#include <KxFramework/KxTextFile.h>

KModEntry::FixedWebSitePair KPackageProjectSerializer::TryParseWebSite(const wxString& url, wxString* domainNameOut)
{
	long long id = -1;
	KNetworkProviderID index = KNETWORK_PROVIDER_ID_INVALID;

	// https://regex101.com
	wxString regEx = wxString::FromUTF8Unchecked(u8R"((?:http:\/\/)?(?:https:\/\/)?(?:[^@\n]+@)?(?:www\.)?([^:\/\n]+)(?:.*\/)(?:[^\d]+)(\d+))");
	wxRegEx reURL(regEx, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
	if (reURL.Matches(url))
	{
		// Site name
		//wxString siteName = reURL.GetMatch(url, 1).MakeLower();
		wxString siteName = KAux::ExtractDomainName(url);
		if (siteName == "tesall.ru")
		{
			index = KNETWORK_PROVIDER_ID_TESALL;
		}
		else if (siteName == "nexusmods.com" || siteName.AfterFirst('.') == "nexusmods.com" || siteName.Contains("nexus"))
		{
			index = KNETWORK_PROVIDER_ID_NEXUS;
		}
		else if (siteName == "loverslab.com")
		{
			index = KNETWORK_PROVIDER_ID_LOVERSLAB;
		}
		KxUtility::SetIfNotNull(domainNameOut, siteName);

		// ID
		reURL.GetMatch(url, 2).ToLongLong(&id);
	}
	return std::make_pair(id, index);
}
wxString KPackageProjectSerializer::ConvertBBCode(const wxString& bbSource)
{
	wxString copy = bbSource;
	return KNetwork::GetInstance()->GetProvider(KNETWORK_PROVIDER_ID_NEXUS)->ConvertDescriptionToHTML(copy);
}
wxString KPackageProjectSerializer::PathNameToPackage(const wxString& pathName, KPPContentType type) const
{
	switch (type)
	{
		case KPP_CONTENT_IMAGES:
		{
			wxString name = pathName.AfterLast('\\');
			return m_PackageDataRoot + "\\Images\\" + name;
		}
		case KPP_CONTENT_DOCUMENTS:
		{
			wxString name = pathName.AfterLast('\\');
			return m_PackageDataRoot + "\\Documents\\" + name;
		}
		case KPP_CONTENT_FILEDATA:
		{
			return pathName;
		}
	};
	return wxEmptyString;
}
bool KPackageProjectSerializer::CheckTag(const wxString& tagName) const
{
	return KModManager::GetTagManager().FindModTag(tagName) != NULL;
}

KPackageProjectSerializer::KPackageProjectSerializer()
{
}
KPackageProjectSerializer::~KPackageProjectSerializer()
{
}
