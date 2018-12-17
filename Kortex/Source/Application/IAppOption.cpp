#include "stdafx.h"
#include "IAppOption.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include "SystemApplication.h"

namespace Kortex
{
	bool IAppOption::DoSetValue(const wxString& value, bool isCDATA)
	{
		const bool res = m_ConfigNode->SetValue(value, isCDATA);
		ChangeNotify();
		return res;
	}
	bool IAppOption::DoSetAttribute(const wxString& name, const wxString& value)
	{
		const bool res = m_ConfigNode->SetAttribute(name, value);
		ChangeNotify();
		return res;
	}
	void IAppOption::ChangeNotify()
	{
		if (IsGlobalOption())
		{
			SystemApplication::GetInstance()->OnGlobalConfigChanged(*this);
		}
		else if (IsInstanceOption())
		{
			m_Instance->OnConfigChanged(*this);
		}
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

	void IAppOption::Save()
	{
		if (IsGlobalOption())
		{
			SystemApplication::GetInstance()->SaveGlobalSettings();
		}
		else if (IsInstanceOption())
		{
			m_Instance->SaveConfig();
		}
	}
}
