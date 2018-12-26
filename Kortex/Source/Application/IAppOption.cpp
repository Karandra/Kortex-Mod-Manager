#include "stdafx.h"
#include "IAppOption.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include "SystemApplication.h"

namespace Kortex
{
	wxString IAppOption::DoGetValue(const wxString& defaultValue) const
	{
		return m_ConfigNode.GetValue(defaultValue);
	}
	bool IAppOption::DoSetValue(const wxString& value, bool isCDATA)
	{
		const bool res = m_ConfigNode.SetValue(value, isCDATA);
		NotifyChange();
		return res;
	}

	wxString IAppOption::DoGetAttribute(const wxString& name, const wxString& defaultValue) const
	{
		return m_ConfigNode.GetAttribute(name, defaultValue);
	}
	bool IAppOption::DoSetAttribute(const wxString& name, const wxString& value)
	{
		const bool res = m_ConfigNode.SetAttribute(name, value);
		NotifyChange();
		return res;
	}
	
	bool IAppOption::AssignInstance(const IConfigurableGameInstance* instance)
	{
		m_Instance = const_cast<IConfigurableGameInstance*>(instance);
		return true;
	}
	bool IAppOption::AssignActiveInstance()
	{
		IGameInstance* instance = IGameInstance::GetActive();
		if (instance && instance->QueryInterface(m_Instance))
		{
			return true;
		}
		else
		{
			wxLogError("Failed to assign 'IConfigurableGameInstance' interface to 'IAppOption' object");
			return false;
		}
	}
	
	void IAppOption::AssignProfile(IGameProfile* profile)
	{
		m_Profile = profile;
	}
	void IAppOption::AssignActiveProfile()
	{
		m_Profile = IGameProfile::GetActive();
	}

	void IAppOption::NotifyChange()
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
