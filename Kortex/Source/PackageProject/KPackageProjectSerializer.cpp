#include "stdafx.h"
#include "KPackageProjectSerializer.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include "KAux.h"
#include <KxFramework/KxTextFile.h>

Kortex::ModProviderItem KPackageProjectSerializer::TryParseWebSite(const wxString& url, wxString* domainNameOut)
{
	using namespace Kortex::Network;

	long long id = -1;
	Kortex::INetworkProvider* provider = nullptr;

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
			provider = Kortex::Network::TESALLProvider::GetInstance();
		}
		else if (siteName == "nexusmods.com" || siteName.AfterFirst('.') == "nexusmods.com" || siteName.Contains("nexus"))
		{
			provider = Kortex::Network::NexusProvider::GetInstance();
		}
		else if (siteName == "loverslab.com")
		{
			provider = Kortex::Network::LoversLabProvider::GetInstance();
		}
		KxUtility::SetIfNotNull(domainNameOut, siteName);

		// ID
		reURL.GetMatch(url, 2).ToLongLong(&id);
	}

	if (provider)
	{
		Kortex::ModProviderItem(provider->GetName(), id);
	}
	return Kortex::ModProviderItem();
}
wxString KPackageProjectSerializer::ConvertBBCode(const wxString& bbSource)
{
	wxString copy = bbSource;
	return Kortex::Network::NexusProvider::GetInstance()->ConvertDescriptionToHTML(copy);
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
	return Kortex::IModTagManager::GetInstance()->FindTagByName(tagName) != nullptr;
}

KPackageProjectSerializer::KPackageProjectSerializer()
{
}
KPackageProjectSerializer::~KPackageProjectSerializer()
{
}
