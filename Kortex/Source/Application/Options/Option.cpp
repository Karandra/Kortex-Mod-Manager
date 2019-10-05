#include "stdafx.h"
#include "Option.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/PackageManager.hpp>
#include <Kortex/InstallWizard.hpp>

namespace
{
	using namespace Kortex;
	using namespace Kortex::Application;
	using Disposition = BasicOption::Disposition;

	template<class... Args> KxXMLNode InitNode(Disposition disposition, KxXMLDocument& xml, Args&&... arg)
	{
		const wxChar* root = wxS("");
		switch (disposition)
		{
			case Disposition::Global:
			{
				root = wxS("Global");
				break;
			}
			case Disposition::Instance:
			{
				root = wxS("Instance");
				break;
			}
			case Disposition::Profile:
			{
				root = wxS("Profile");
				break;
			}
		};
		return xml.QueryOrCreateElement(AppOption::MakeXPath(root, std::forward<Args>(arg)...));
	}
	template<class... Args> KxXMLNode InitNodeUsingModule(Disposition disposition, KxXMLDocument& xml, const  IModule& module, Args&&... arg)
	{
		return InitNode(disposition, xml, module.GetModuleInfo().GetID(), std::forward<Args>(arg)...);
	}
	template<class... Args> KxXMLNode InitNodeUsingManager(Disposition disposition, KxXMLDocument& xml, const IManager& manager, Args&&... arg)
	{
		return InitNodeUsingModule(disposition, xml, manager.GetModule(), manager.GetManagerInfo().GetID(), std::forward<Args>(arg)...);
	}
}

namespace Kortex::Application
{
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const wxString& branch)
	{
		KxXMLNode node = InitNode(disposition, xml, branch);
		AssignDisposition(disposition);
		AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const IApplication& app, const wxString& branch)
	{
		KxXMLNode node = InitNode(disposition, xml, wxS("Application"), branch);
		AssignDisposition(disposition);
		AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const IGameInstance& instance, const wxString& branch)
	{
		KxXMLNode node = InitNode(disposition, xml, branch);
		AssignDisposition(disposition);
		AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const IGameProfile& profile, const wxString& branch)
	{
		KxXMLNode node = InitNode(disposition, xml, branch);
		AssignDisposition(disposition);
		AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const IModule& module, const wxString& branch)
	{
		KxXMLNode node = InitNodeUsingModule(disposition, xml, module, branch);
		AssignDisposition(disposition);
		AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const IManager& manager, const wxString& branch)
	{
		KxXMLNode node = InitNodeUsingManager(disposition, xml, manager, branch);
		AssignDisposition(disposition);
		AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const IWorkspace& workspace, const wxString& branch)
	{
		KxXMLNode node = InitNode(disposition, xml, wxS("Workspace"), workspace.GetID(), branch);
		AssignDisposition(disposition);
		AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const IMainWindow& mainWindow, const wxString& branch)
	{
		KxXMLNode node = InitNode(disposition, xml, wxS("Application/MainWindow"), branch);
		AssignDisposition(disposition);
		AssignNode(node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const InstallWizard::WizardDialog& installWizard, const wxString& branch)
	{
		KxXMLNode node = InitNodeUsingManager(disposition, xml, *IPackageManager::GetInstance(), wxS("InstallWizard"), branch);
		AssignDisposition(disposition);
		AssignNode(node);
	}
}

namespace Kortex::Application
{
	IGameInstance* ActiveInstanceOption::GetActiveInstance() const
	{
		return IGameInstance::GetActive();
	}
}

namespace Kortex::Application
{
	KxXMLDocument& GlobalOption::GetXML() const
	{
		return IApplication::GetInstance()->GetGlobalConfig();
	}
}

namespace Kortex::Application
{
	IConfigurableGameInstance* InstanceOption::GetConfigurableInstance(IGameInstance* instance) const
	{
		if (instance)
		{
			return instance->QueryInterface<IConfigurableGameInstance>();
		}
		return nullptr;
	}
	KxXMLDocument& InstanceOption::GetXML(IConfigurableGameInstance* instance) const
	{
		return instance->GetConfig();
	}
}

namespace Kortex::Application
{
	KxXMLDocument& ProfileOption::GetXML(IGameProfile& profile) const
	{
		return profile.GetConfig();
	}
}

namespace Kortex::Application
{
	IGameProfile* ActiveProfileOption::GetActiveProfile() const
	{
		return IGameProfile::GetActive();
	}
}
