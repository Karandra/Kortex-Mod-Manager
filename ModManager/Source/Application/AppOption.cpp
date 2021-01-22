#include "pch.hpp"
#include "AppOption.h"
#include "IApplication.h"
#include "GameInstance/IGameProfile.h"
#include "GameInstance/IGameInstance.h"
#include "Log.h"

namespace Kortex
{
	std::optional<kxf::String> AppOption::DoGetValue() const
	{
		return m_ConfigNode.GetValue();
	}
	bool AppOption::DoSetValue(const kxf::String& value, WriteEmpty writeEmpty, AsCDATA asCDATA)
	{
		const bool res = m_ConfigNode.SetValue(value, WriteEmpty::Never, asCDATA);
		NotifyChange();
		return res;
	}

	std::optional<kxf::String> AppOption::DoGetAttribute(const kxf::String& name) const
	{
		return m_ConfigNode.GetAttribute(name);
	}
	bool AppOption::DoSetAttribute(const kxf::String& name, const kxf::String& value, WriteEmpty writeEmpty)
	{
		const bool res = m_ConfigNode.SetAttribute(name, value, WriteEmpty::Never);
		NotifyChange();
		return res;
	}
	
	bool AppOption::AssignInstance(IGameInstance& instance)
	{
		m_Instance = &instance;
		return true;
	}
	bool AppOption::AssignActiveInstance()
	{
		IGameInstance* instance = IApplication::GetInstance().GetActiveGameInstance();
		if (instance && instance->QueryInterface<IConfigurableGameInstance>())
		{
			m_Instance = instance;
			return true;
		}
		else
		{
			Log::Error("Failed to assign 'IConfigurableGameInstance' interface from active game instance to an 'AppOption' object");
			return false;
		}
	}
	
	void AppOption::AssignProfile(IGameProfile& profile)
	{
		m_Profile = &profile;
	}
	void AppOption::AssignActiveProfile()
	{
		if (auto instance = IApplication::GetInstance().GetActiveGameInstance())
		{
			m_Profile = instance->GetActiveProfile();
		}
	}

	AppOption::AppOption() = default;
	AppOption::AppOption(const AppOption& other, const kxf::XMLNode& node)
	{
		*this = other;
		m_ConfigNode = node;
	}
	AppOption::AppOption(const AppOption&) noexcept = default;
	AppOption::~AppOption() = default;

	bool AppOption::IsNull() const
	{
		return m_ConfigNode.IsNull() || m_Disposition == Disposition::None;
	}
	AppOption AppOption::QueryElement(const kxf::String& XPath) const
	{
		return AppOption(*this, m_ConfigNode.QueryElement(XPath));
	}
	AppOption AppOption::ConstructElement(const kxf::String& XPath)
	{
		return AppOption(*this, m_ConfigNode.ConstructElement(XPath));
	}

	void AppOption::NotifyChange()
	{
		IApplication& app = IApplication::GetInstance();
		switch (m_Disposition)
		{
			case Disposition::Global:
			{
				app.OnGlobalConfigChanged(*this);
				break;
			}
			case Disposition::Instance:
			{
				app.OnInstanceConfigChanged(*this, *m_Instance->QueryInterface<IGameInstance>());
				break;
			}
			case Disposition::Profile:
			{
				app.OnProfileConfigChanged(*this, *m_Profile);
				break;
			}
		};
	}

	AppOption& AppOption::operator=(const AppOption&) noexcept = default;
}
