#include "stdafx.h"
#include "DisplayModelNode.h"
#include "Utility/String.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxTextFile.h>

using namespace Kortex::Utility;

namespace Kortex::Application::About
{
	wxString INode::GetLocation(Type type) const
	{
		const IApplication* app = IApplication::GetInstance();

		switch (type)
		{
			case Type::Application:
			{
				return String::ConcatWithSeparator(wxS('\\'), app->GetDataFolder(), wxS("About"));
			}
			case Type::Software:
			{
				return String::ConcatWithSeparator(wxS('\\'), app->GetDataFolder(), wxS("About"), wxS("Software"), GetName());
			}
			case Type::Resource:
			{
				return String::ConcatWithSeparator(wxS('\\'), app->GetDataFolder(), wxS("About"), wxS("Resource"), GetName());
			}
		};
		return wxEmptyString;
	}
	wxString INode::ReadLicense(Type type) const
	{
		wxString license = KxTextFile::ReadToString(String::ConcatWithSeparator(wxS('\\'), GetLocation(type), wxS("License.txt")));
		if (!license.IsEmpty())
		{
			// Convert any '<text>' which is not a link to '&lt;text&gt;'
			{
				wxRegEx regex("<(?!http:)(.*?)>", wxRE_ADVANCED|wxRE_ICASE);
				regex.ReplaceAll(&license, wxS("\\&lt;\\1\\&gt;"));
			}

			// Create clickable links
			{
				wxRegEx regex("<http:(.*?)>", wxRE_ADVANCED|wxRE_ICASE);
				regex.ReplaceAll(&license, wxS("<a href=\"http:\\1\">http:\\1</a>"));
			}
		}
		return license;
	}
	const wxString& INode::LoadLicense(LicenseData& data, Type type) const
	{
		if (data.m_License.IsEmpty() && data.m_ShouldLoad)
		{
			data.m_License = ReadLicense(type);
			data.m_ShouldLoad = false;
		}
		return data;
	}
}

namespace Kortex::Application::About
{
	wxString AppNode::GetName() const
	{
		return IApplication::GetInstance()->GetName();
	}
	KxVersion AppNode::GetVersion() const
	{
		return IApplication::GetInstance()->GetVersion();
	}
	ResourceID AppNode::GetIconID() const
	{
		return ImageResourceID::KortexLogoSmall;
	}
	wxString AppNode::GetLicense() const
	{
		return LoadLicense(m_Licence, Type::Application);
	}
	wxString AppNode::GetURL() const
	{
		return "https://github.com/KerberX/Kortex-Mod-Manager";
	}
}

namespace Kortex::Application::About
{
	wxString ModuleNode::GetName() const
	{
		return m_Module.GetModuleInfo().GetName();
	}
	KxVersion ModuleNode::GetVersion() const
	{
		return m_Module.GetModuleInfo().GetVersion();
	}
	ResourceID ModuleNode::GetIconID() const
	{
		return m_Module.GetModuleInfo().GetImageID();
	}
	wxString ModuleNode::GetLicense() const
	{
		return LoadLicense(m_Licence, Type::Software);
	}
}
