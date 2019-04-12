#include "stdafx.h"
#include "KPackageProjectSerializer.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include "Utility/KAux.h"
#include <KxFramework/KxTextFile.h>

Kortex::ModSourceItem KPackageProjectSerializer::TryParseWebSite(const wxString& url, wxString* domainNameOut)
{
	using namespace Kortex::NetworkManager;

	long long id = -1;
	Kortex::IModSource* modSource = nullptr;

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
			modSource = Kortex::NetworkManager::TESALLSource::GetInstance();
		}
		else if (siteName == "nexusmods.com" || siteName.AfterFirst('.') == "nexusmods.com" || siteName.Contains("nexus"))
		{
			modSource = Kortex::NetworkManager::NexusSource::GetInstance();
		}
		else if (siteName == "loverslab.com")
		{
			modSource = Kortex::NetworkManager::LoversLabSource::GetInstance();
		}
		KxUtility::SetIfNotNull(domainNameOut, siteName);

		// ID
		reURL.GetMatch(url, 2).ToLongLong(&id);
	}

	if (modSource)
	{
		return Kortex::ModSourceItem(modSource->GetName(), Kortex::ModID(id));
	}
	return Kortex::ModSourceItem();
}
wxString KPackageProjectSerializer::ConvertBBCode(const wxString& bbSource)
{
	wxString copy = bbSource;
	return Kortex::NetworkManager::NexusSource::GetInstance()->ConvertDescriptionToHTML(copy);
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
