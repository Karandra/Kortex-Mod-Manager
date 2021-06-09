#include "pch.hpp"
#include "Option.h"
#include "OptionDatabase.h"
#include "../IApplication.h"
#include "../IMainWindow.h"
#include "../IWorkspace.h"
#include "../IModule.h"
#include "GameDefinition/IGameInstance.h"
#include "GameDefinition/IGameProfile.h"

namespace
{
	using namespace Kortex;
	using namespace Kortex::Application;
	using Disposition = BasicOption::Disposition;

	template<class TXML, class... Args>
	kxf::XMLNode InitNode(Disposition disposition, TXML&& xml, Args&&... arg)
	{
		const kxf::XChar* root = wxS("");
		switch (disposition)
		{
			case Disposition::Global:
			{
				root = OName::Global;
				break;
			}
			case Disposition::Instance:
			{
				root = OName::Instance;
				break;
			}
			case Disposition::Profile:
			{
				root = OName::Profile;
				break;
			}
		};

		kxf::String xPath = AppOption::MakeXPath(root, std::forward<Args>(arg)...);
		xPath.Replace(wxS("::"), wxS("-"));

		if constexpr(std::is_const_v<std::remove_reference_t<TXML>>)
		{
			return xml.QueryElement(xPath);
		}
		else
		{
			return xml.ConstructElement(xPath);
		}
	}

	template<class TXML, class... Args>
	kxf::XMLNode InitNodeUsingModule(Disposition disposition, TXML&& xml, const IModule& module, Args&&... arg)
	{
		auto classInfo = module.QueryInterface<kxf::RTTI::ClassInfo>();
		if (auto iid = classInfo->GetIID().ToUniversallyUniqueID())
		{
			kxf::String moduleDescriptor = kxf::Format(wxS("Module-[{}]"), iid.ToString());

			kxf::XMLNode node = InitNode(disposition, std::forward<TXML>(xml), wxS("Modules"), std::move(moduleDescriptor), std::forward<Args>(arg)...);
			if constexpr(!std::is_const_v<std::remove_reference_t<TXML>>)
			{
				node.SetAttribute(wxS("IID"), iid.ToString(kxf::UUIDFormat::CurlyBraces|kxf::UUIDFormat::HexPrefix|kxf::UUIDFormat::Grouped, wxS(", ")));
				node.SetAttribute(wxS("Name"), classInfo->GetFullyQualifiedName());
			}
			return node;
		}
		return {};
	}
}

namespace Kortex::Application
{
	void BasicOption::CreateFromAny(Disposition disposition, kxf::XMLDocument& xml, const kxf::IObject& ref, const kxf::String& branch, bool readOnly)
	{
		auto DoCreate = [this](Disposition disposition, auto&& xml, const kxf::IObject& ref, const kxf::String& branch)
		{
			if (auto module = ref.QueryInterface<IModule>())
			{
				Create(disposition, xml, *module, branch);
			}
			else if (ref.QueryInterface<IApplication>())
			{
				AssignNode(InitNode(disposition, xml, wxS("Application"), branch));
			}
			else if (ref.QueryInterface<IMainWindow>())
			{
				AssignNode(InitNode(disposition, xml, wxS("MainWindow"), branch));
			}
			else
			{
				Create(disposition, xml, branch);
			}
		};

		if (readOnly)
		{
			DoCreate(disposition, const_cast<const kxf::XMLDocument&>(xml), ref, branch);
		}
		else
		{
			DoCreate(disposition, const_cast<kxf::XMLDocument&>(xml), ref, branch);
		}
	}

	void BasicOption::Create(Disposition disposition, kxf::XMLDocument& xml, const kxf::String& branch)
	{
		AssignNode(InitNode(disposition, xml, branch));
	}
	void BasicOption::Create(Disposition disposition, const kxf::XMLDocument& xml, const kxf::String& branch)
	{
		AssignNode(InitNode(disposition, xml, branch));
	}

	void BasicOption::Create(Disposition disposition, kxf::XMLDocument& xml, const IModule& module, const kxf::String& branch)
	{
		AssignNode(InitNodeUsingModule(disposition, xml, module, branch));
	}
	void BasicOption::Create(Disposition disposition, const kxf::XMLDocument& xml, const IModule& module, const kxf::String& branch)
	{
		AssignNode(InitNodeUsingModule(disposition, xml, module, branch));
	}
}

namespace Kortex::Application::Private
{
	kxf::XMLDocument& GlobalOption::GetXML()
	{
		return IApplication::GetInstance().GetGlobalConfig();
	}
}
namespace Kortex::Application::Private
{
	kxf::XMLDocument& InstanceOption::GetXML(IGameInstance& instance)
	{
		return instance.GetnstanceData();
	}
	const kxf::XMLDocument& InstanceOption::GetXML(const IGameInstance& instance) const
	{
		return instance.GetnstanceData();
	}
}
namespace Kortex::Application::Private
{
	kxf::XMLDocument& ProfileOption::GetXML(IGameProfile& profile)
	{
		return profile.GetProfileData();
	}
	const kxf::XMLDocument& ProfileOption::GetXML(const IGameProfile& profile) const
	{
		return profile.GetProfileData();
	}
}
