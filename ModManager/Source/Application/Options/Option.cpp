#include "pch.hpp"
#include "Option.h"
#include "OptionDatabase.h"
#include "../IApplication.h"
#include "../IWorkspace.h"
#include "GameInstance/IGameInstance.h"
#include "GameInstance/IGameProfile.h"
//#include <Kortex/PackageManager.hpp>
//#include <Kortex/InstallWizard.hpp>

namespace
{
	using namespace Kortex;
	using namespace Kortex::Application;
	using Disposition = BasicOption::Disposition;

	template<class... Args>
	kxf::XMLNode InitNode(Disposition disposition, kxf::XMLDocument& xml, Args&&... arg)
	{
		const wxChar* root = wxS("");
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
		return xml.ConstructElement(xPath);
	}

	template<class... Args>
	kxf::XMLNode InitNodeUsingModule(Disposition disposition, kxf::XMLDocument& xml, const  IModule& module, Args&&... arg)
	{
		//return InitNode(disposition, xml, module.GetModuleInfo().GetID(), std::forward<Args>(arg)...);
	}

	template<class... Args>
	kxf::XMLNode InitNodeUsingManager(Disposition disposition, kxf::XMLDocument& xml, const IManager& manager, Args&&... arg)
	{
		//return InitNodeUsingModule(disposition, xml, manager.GetModule(), manager.GetManagerInfo().GetID(), std::forward<Args>(arg)...);
	}
}

namespace Kortex::Application
{
	void BasicOption::Create(Disposition disposition, kxf::XMLDocument& xml, const kxf::String& branch)
	{
		kxf::XMLNode node = InitNode(disposition, xml, branch);
		AssignDisposition(disposition);
		AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, kxf::XMLDocument& xml, const IApplication& app, const kxf::String& branch)
	{
		kxf::XMLNode node = InitNode(disposition, xml, wxS("Application"), branch);
		AssignDisposition(disposition);
		AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, kxf::XMLDocument& xml, const IGameInstance& instance, const kxf::String& branch)
	{
		kxf::XMLNode node = InitNode(disposition, xml, branch);
		AssignDisposition(disposition);
		AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, kxf::XMLDocument& xml, const IGameProfile& profile, const kxf::String& branch)
	{
		kxf::XMLNode node = InitNode(disposition, xml, branch);
		AssignDisposition(disposition);
		AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, kxf::XMLDocument& xml, const IModule& module, const kxf::String& branch)
	{
		//kxf::XMLNode node = InitNodeUsingModule(disposition, xml, module, branch);
		//AssignDisposition(disposition);
		//AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, kxf::XMLDocument& xml, const IManager& manager, const kxf::String& branch)
	{
		//kxf::XMLNode node = InitNodeUsingManager(disposition, xml, manager, branch);
		//AssignDisposition(disposition);
		//AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, kxf::XMLDocument& xml, const IWorkspace& workspace, const kxf::String& branch)
	{
		kxf::XMLNode node = InitNode(disposition, xml, wxS("Workspace"), workspace.GetID(), branch);
		AssignDisposition(disposition);
		AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, kxf::XMLDocument& xml, const IMainWindow& mainWindow, const kxf::String& branch)
	{
		kxf::XMLNode node = InitNode(disposition, xml, wxS("Application/MainWindow"), branch);
		AssignDisposition(disposition);
		AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, kxf::XMLDocument& xml, const InstallWizard::WizardDialog& installWizard, const kxf::String& branch)
	{
		//kxf::XMLNode node = InitNodeUsingManager(disposition, xml, *IPackageManager::GetInstance(), wxS("InstallWizard"), branch);
		//AssignDisposition(disposition);
		//AssignNode(node);
	}
}

namespace Kortex::Application
{
	IGameInstance* ActiveInstanceOption::GetActiveInstance() const
	{
		return IApplication::GetInstance().GetActiveGameInstance();
	}
}

namespace Kortex::Application
{
	kxf::XMLDocument& GlobalOption::GetXML() const
	{
		return IApplication::GetInstance().GetGlobalConfig();
	}
}

namespace Kortex::Application
{
	IConfigurableGameInstance* InstanceOption::GetConfigurableInstance(IGameInstance& instance) const
	{
		return instance.QueryInterface<IConfigurableGameInstance>().get();
	}
	kxf::XMLDocument& InstanceOption::GetXML(IConfigurableGameInstance& instance) const
	{
		return instance.GetConfig();
	}
}

namespace Kortex::Application
{
	kxf::XMLDocument* ProfileOption::GetXML(IGameProfile& profile) const
	{
		if (auto withConfig = profile.QueryInterface<IWithConfig>())
		{
			return &withConfig->GetConfig();
		}
		return nullptr;
	}
}

namespace Kortex::Application
{
	IGameProfile* ActiveProfileOption::GetActiveProfile() const
	{
		if (IGameInstance* instance = IApplication::GetInstance().GetActiveGameInstance())
		{
			return instance->GetActiveProfile();
		}
		return nullptr;
	}
}
