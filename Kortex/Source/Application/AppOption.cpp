#include "stdafx.h"
#include "AppOption.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include "SystemApplication.h"
#include "Utility/Log.h"

namespace Kortex
{
	wxString AppOption::DoGetValue(const wxString& defaultValue) const
	{
		return m_ConfigNode.GetValue(defaultValue);
	}
	bool AppOption::DoSetValue(const wxString& value, WriteEmpty writeEmpty, AsCDATA asCDATA)
	{
		const bool res = m_ConfigNode.SetValue(value, WriteEmpty::Never, asCDATA);
		NotifyChange();
		return res;
	}

	wxString AppOption::DoGetAttribute(const wxString& name, const wxString& defaultValue) const
	{
		return m_ConfigNode.GetAttribute(name, defaultValue);
	}
	bool AppOption::DoSetAttribute(const wxString& name, const wxString& value, WriteEmpty writeEmpty)
	{
		const bool res = m_ConfigNode.SetAttribute(name, value, WriteEmpty::Never);
		NotifyChange();
		return res;
	}
	
	bool AppOption::AssignInstance(const IConfigurableGameInstance* instance)
	{
		m_Instance = const_cast<IConfigurableGameInstance*>(instance);
		return true;
	}
	bool AppOption::AssignActiveInstance()
	{
		IGameInstance* instance = IGameInstance::GetActive();
		if (instance && instance->QueryInterface(m_Instance))
		{
			return true;
		}
		else
		{
			Utility::Log::LogError("Failed to assign 'IConfigurableGameInstance' interface to an 'AppOption' object");
			return false;
		}
	}
	
	void AppOption::AssignProfile(IGameProfile* profile)
	{
		m_Profile = profile;
	}
	void AppOption::AssignActiveProfile()
	{
		m_Profile = IGameProfile::GetActive();
	}

	AppOption::AppOption(const AppOption& other, const KxXMLNode& node)
	{
		*this = other;
		m_ConfigNode = node;
	}

	bool AppOption::IsOK() const
	{
		return m_ConfigNode.IsOK() && m_Disposition != Disposition::None;
	}
	AppOption AppOption::QueryElement(const wxString& XPath) const
	{
		KxXMLNode node = m_ConfigNode.QueryElement(XPath);
		if (node.IsOK())
		{
			return AppOption(*this, node);
		}
		return {};
	}
	AppOption AppOption::ConstructElement(const wxString& XPath)
	{
		if (KxXMLNode node = m_ConfigNode.ConstructElement(XPath))
		{
			return AppOption(*this, node);
		}
		return {};
	}

	void AppOption::NotifyChange()
	{
		// Disable for now
		return;

		SystemApplication* sysApp = SystemApplication::GetInstance();
		switch (m_Disposition)
		{
			case Disposition::Global:
			{
				sysApp->OnGlobalConfigChanged(*this);
				break;
			}
			case Disposition::Instance:
			{
				sysApp->OnInstanceConfigChanged(*this, *m_Instance->QueryInterface<IGameInstance>());
				break;
			}
			case Disposition::Profile:
			{
				sysApp->OnProfileConfigChanged(*this, *m_Profile);
				break;
			}
		};
	}
}
