#include "stdafx.h"
#include "Option.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include "PackageManager/KPackageManager.h"
#include "UI/KWorkspace.h"
#include "InstallWizard/KInstallWizardDialog.h"

namespace
{
	using namespace Kortex;
	using namespace Kortex::Application;
	using Disposition = BasicOption::Disposition;

	template<class... Args> wxString Concat(Args&&... arg)
	{
		return ((wxString(wxS('/')) + arg) + ...);
	}
	KxXMLNode QueryConfigNode(KxXMLDocument& xml, const wxString& XPath)
	{
		return xml.QueryOrCreateElement(XPath);
	}

	template<class... Args> void InitFromEx(Disposition disposition, KxXMLDocument& xml, KxXMLNode& node, Args&&... arg)
	{
		const wxChar* root = wxS("/");
		switch (disposition)
		{
			case Disposition::Global:
			{
				root = wxS("Global/");
				break;
			}
			case Disposition::Instance:
			{
				root = wxS("Instance/");
				break;
			}
			case Disposition::Profile:
			{
				root = wxS("Profile/");
				break;
			}
		};
		node = QueryConfigNode(xml, Concat(root, std::forward<Args>(arg)...));
	}
	template<class... Args> void InitFrom(Disposition disposition, KxXMLDocument& xml, KxXMLNode& node, const  IModule& module, Args&&... arg)
	{
		InitFromEx(disposition, xml, node, module.GetModuleInfo().GetID(), std::forward<Args>(arg)...);
	}
	template<class... Args> void InitFrom(Disposition disposition, KxXMLDocument& xml, KxXMLNode& node, const IManager& manager, Args&&... arg)
	{
		InitFrom(disposition, xml, node, manager.GetModule(), wxS('/'), manager.GetManagerInfo().GetID(), std::forward<Args>(arg)...);
	}
}

namespace Kortex::Application
{
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const wxString& branch)
	{
		InitFromEx(disposition, xml, m_Node, branch);
		AssignDisposition(disposition);
		AssignNode(m_Node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const IApplication& app, const wxString& branch)
	{
		InitFromEx(disposition, xml, m_Node, wxS("Application"), branch);
		AssignDisposition(disposition);
		AssignNode(m_Node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const IGameInstance& instance, const wxString& branch)
	{
		InitFromEx(disposition, xml, m_Node, branch);
		AssignDisposition(disposition);
		AssignNode(m_Node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const IGameProfile& profile, const wxString& branch)
	{
		InitFromEx(disposition, xml, m_Node, branch);
		AssignDisposition(disposition);
		AssignNode(m_Node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const IModule& module, const wxString& branch)
	{
		InitFrom(disposition, xml, m_Node, module, branch);
		AssignDisposition(disposition);
		AssignNode(m_Node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const IManager& manager, const wxString& branch)
	{
		InitFrom(disposition, xml, m_Node, manager, branch);
		AssignDisposition(disposition);
		AssignNode(m_Node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const KWorkspace& workspace, const wxString& branch)
	{
		// TODO: Workspace
		AssignDisposition(disposition);
		AssignNode(m_Node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const KMainWindow& mainWindow, const wxString& branch)
	{
		InitFromEx(disposition, xml, m_Node, wxS("Application/MainWindow"), branch);
		AssignDisposition(disposition);
		AssignNode(m_Node);
	}
	void BasicOption::Create(Disposition disposition, KxXMLDocument& xml, const KInstallWizardDialog& installWizard, const wxString& branch)
	{
		InitFrom(disposition, xml, m_Node, *KPackageManager::GetInstance(), wxS("InstallWizard"), branch);
		AssignDisposition(disposition);
		AssignNode(m_Node);
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
