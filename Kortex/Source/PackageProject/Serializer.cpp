#include "stdafx.h"
#include "Serializer.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include "Network/ModNetwork/Nexus.h"
#include "Network/ModNetwork/LoversLab.h"
#include "Network/ModNetwork/TESALL.h"
#include "Utility/KAux.h"
#include <KxFramework/KxTextFile.h>

namespace Kortex::PackageProject
{
	ModSourceItem Serializer::TryParseWebSite(const wxString& url, wxString* domainNameOut)
	{
		using namespace Kortex::NetworkManager;
	
		long long id = -1;
		Kortex::IModNetwork* modNetwork = nullptr;
	
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
				modNetwork = Kortex::NetworkManager::TESALLModNetwork::GetInstance();
			}
			else if (siteName == "nexusmods.com" || siteName.AfterFirst('.') == "nexusmods.com" || siteName.Contains("nexus"))
			{
				modNetwork = Kortex::NetworkManager::NexusModNetwork::GetInstance();
			}
			else if (siteName == "loverslab.com")
			{
				modNetwork = Kortex::NetworkManager::LoversLabModNetwork::GetInstance();
			}
			KxUtility::SetIfNotNull(domainNameOut, siteName);
	
			// ID
			reURL.GetMatch(url, 2).ToLongLong(&id);
		}
	
		if (modNetwork)
		{
			return Kortex::ModSourceItem(modNetwork->GetName(), Kortex::ModID(id));
		}
		return Kortex::ModSourceItem();
	}
	wxString Serializer::ConvertBBCode(const wxString& bbSource)
	{
		wxString copy = bbSource;
		Kortex::NetworkManager::NexusModNetwork::GetInstance()->ConvertDescriptionText(copy);
		return copy;
	}
	wxString Serializer::PathNameToPackage(const wxString& pathName, ContentType type) const
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
	bool Serializer::CheckTag(const wxString& tagName) const
	{
		return Kortex::IModTagManager::GetInstance()->FindTagByName(tagName) != nullptr;
	}
	
	Serializer::Serializer()
	{
	}
	Serializer::~Serializer()
	{
	}
}
